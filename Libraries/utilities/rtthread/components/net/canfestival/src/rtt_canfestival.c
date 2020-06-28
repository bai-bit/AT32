#include <stdbool.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "canfestival.h"
#include "TaskSlave.h"
#include "timer.h"
#include "gpio.h"
#include "at32_can.h"
#include "chkv.h"

#include "app.h"
#include "common.h"
#include "at32_can.h"

static rt_mq_t      rx_mq;
static rt_mq_t      tx_mq;
static rt_mutex_t   cf_mutex;
static rt_sem_t     cf_tx_sem;

#define RTT_CF_DEBUG		0
#if ( RTT_CF_DEBUG == 1 )
#include <stdio.h>
#define RTT_CF_TRACE	printf
#else
#define RTT_CF_TRACE(...)
#endif

void _cf_enter_mutex(void)
{
	rt_mutex_take(cf_mutex, RT_WAITING_FOREVER);
}

void _cf_exit_mutex(void)
{
	rt_mutex_release(cf_mutex);
}

    
static void _reflash_data(void)
{
    CAN_ACC[0] = sys.imu.cal_acc[0]*1000;
    CAN_ACC[1] = sys.imu.cal_acc[1]*1000;
    CAN_ACC[2] = sys.imu.cal_acc[2]*1000;
    
    GYR[0] = sys.imu.cal_gyr2[0]*10;
    GYR[1] = sys.imu.cal_gyr2[1]*10;
    GYR[2] = sys.imu.cal_gyr2[2]*10;
    
    EUL[0] = sys.att.eul[0]*100;
    EUL[1] = sys.att.eul[1]*100;
    EUL[2] = sys.att.eul[2]*100;
    
    QUAT[0] = sys.att.q[0]*10000;
    QUAT[1] = sys.att.q[1]*10000;
    QUAT[2] = sys.att.q[2]*10000;
    QUAT[3] = sys.att.q[3]*10000;
}

static void _dump_msg(char *str, Message *m)
{
    int i;
    RTT_CF_TRACE("%s id:0x%X, len:%d  data:", str, m->cob_id, m->len);
    for(i=0; i<m->len; i++)
    {
        RTT_CF_TRACE("0x%02X ", m->data[i]);
    }
    RTT_CF_TRACE("\r\n");
}

/* CF can operation interface */

unsigned char canSend(CAN_PORT notused, Message *m)
{
    rt_mq_send(tx_mq, m, sizeof(Message));
    return 0;
}

void canopen_recv_thread_entry(void* parameter)
{
	Message msg;
 
    while (1)
    {
        rt_mq_recv(rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        _dump_msg("rev", &msg);
        
        _cf_enter_mutex();
        canDispatch(&TaskSlave_Data, &msg);
        _cf_exit_mutex();
        
        /* saveing the key param */
        if(NodeID != sys.can_intf.node_id || CAN_BAUD != sys.can_intf.can_baud)
        {
            sys.can_intf.node_id = NodeID;
            sys.can_intf.can_baud = CAN_BAUD;
            
            chkv_write_kv("INF_CAN", (uint8_t*)&sys.can_intf, sizeof(hi_can_intf_t));  
            chkv_save();
        }
    }
}

void canopen_tx_thread_entry(void* parameter)
{
    Message msg;
    
    while(1)
    {
        rt_mq_recv(tx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        
        _reflash_data();
        //_dump_msg("send", m);
        
        rt_sem_take(cf_tx_sem, rt_tick_from_millisecond(10));
        _cf_enter_mutex();
        CAN_Send(HW_CAN1, msg.cob_id, msg.data, msg.len);
        _cf_exit_mutex();
    }
}

/* CF timer interface:  timer need increase every 10 us */

static rt_uint32_t timer_val;
static rt_uint32_t cur_time;

void setTimer(TIMEVAL value)
{
    value /= 100;
    
    cur_time = 0;
    timer_val = value;
    
   // RTT_CF_TRACE("stimer  val:%d\r\n", value);
}

TIMEVAL getElapsedTime(void)
{
    return cur_time*100;
}


static void cf_timer_cb(void* parameter)
{
    cur_time++;
    
    if(cur_time >= timer_val)
    {
        TimeDispatch();
        cur_time = 0;
    }
}

void canopen_system_init(void)
{
    rt_thread_t tid;
    
    NodeID = sys.can_intf.node_id;
    CAN_BAUD = sys.can_intf.can_baud;
    
    /* CAN */
    SetPinMux(HW_GPIOA, 12, 2, 3);   /* CAN_TX */
    SetPinMux(HW_GPIOA, 11, 2, 0);   /* CAN_RX */
    
    CAN_Init(HW_CAN1, CAN_BAUD);
    CAN_SetIntMode(HW_CAN1, kCAN_IntRx_FIFO1, 1);
    CAN_SetIntMode(HW_CAN1, kCAN_IntTxEmpty, 1);
    
    /* let this node id pass */
    CAN_SetFilterMaskMode(HW_CAN1, HW_CAN_RX_FT0, NodeID, 0x7F, HW_CAN_RX_FIFO1);
    
    /* let boardcast not id pass */
    CAN_SetFilterMaskMode(HW_CAN1, HW_CAN_RX_FT1, 0x00, 0x7F, HW_CAN_RX_FIFO1);
    
    rx_mq = rt_mq_create("rx_mq", sizeof(Message), 4, RT_IPC_FLAG_FIFO);
    tx_mq = rt_mq_create("tx_mq", sizeof(Message), 8, RT_IPC_FLAG_FIFO);
    cf_tx_sem = rt_sem_create("tx_sem", 1, RT_IPC_FLAG_FIFO);
    cf_mutex = rt_mutex_create("cf_mtx", RT_IPC_FLAG_FIFO);

    rt_timer_start(rt_timer_create("cf_timer", cf_timer_cb, RT_NULL, rt_tick_from_millisecond(1), RT_TIMER_FLAG_PERIODIC));
    
    tid = rt_thread_create("tcan_rf", canopen_recv_thread_entry, RT_NULL, 1024, 28, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);
    
    tid = rt_thread_create("tcan_tf", canopen_tx_thread_entry, RT_NULL, 512, 29, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);
    
    setNodeId(&TaskSlave_Data, NodeID);
    setState(&TaskSlave_Data, Initialisation);
    setState(&TaskSlave_Data, Pre_operational);
}


void USB_HP_CAN1_TX_IRQHandler(void)
{
    if(CAN1->TSTS & CAN_TSTS_RQC0)
    {
        CAN1->TSTS = CAN_TSTS_RQC0;
    }
    
    if(CAN1->TSTS & CAN_TSTS_RQC1)
    {
        CAN1->TSTS = CAN_TSTS_RQC1;
    }
    
    if(CAN1->TSTS & CAN_TSTS_RQC2)
    {
        CAN1->TSTS = CAN_TSTS_RQC2;
    }
    
    rt_sem_release(cf_tx_sem);
}


void CAN1_RX1_IRQHandler(void)
{
    rt_interrupt_enter();
    
    static Message m;
    uint8_t isRemote;
    uint32_t id;
    
    /* rx fifo overflow */
    if(CAN1->RF1 & CAN_RF1_RFOV1)
    {
        CAN1->RF1 = CAN_RF1_RFOV1;
    }
    
    /* rx fifo fill */
    if(CAN1->RF1 & CAN_RF1_RFFU1)
    {
        CAN1->RF1 = CAN_RF1_RFFU1;
    }
    
    if(CAN1->RF1 & CAN_RF1_RFP1)
    {
        CAN_Receive(HW_CAN1, HW_CAN_RX_FIFO1, &id, m.data, &m.len, &isRemote);
        m.cob_id = id;
        rt_mq_send(rx_mq, &m, sizeof(Message));
    }

    rt_interrupt_leave();
}

