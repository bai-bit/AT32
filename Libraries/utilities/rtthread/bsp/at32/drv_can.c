#include <rtdevice.h>
#include <stdio.h>
#include "common.h"
#include "at32_can.h"


struct rt_can_device rt_can[2];



static rt_err_t configure(struct rt_can_device *can, struct can_configure *cfg)
{
    rt_kprintf("%s\r\n", __FUNCTION__);
    CAN_Init(HW_CAN1, rt_can[1].config.baud_rate);
    return RT_EOK;
}


static rt_err_t control(struct rt_can_device *can, int cmd, void *arg)
{
    rt_uint32_t errtype;
    rt_uint32_t argval;
    
    switch(cmd)
    {
        case RT_DEVICE_CTRL_SET_INT:
            argval = (rt_uint32_t) arg;
            if (argval == RT_DEVICE_FLAG_INT_RX)
            {
                rt_kprintf("%s RT_DEVICE_FLAG_INT_RX argval:0x%X\r\n", __FUNCTION__, argval);
                CAN_SetIntMode(HW_CAN1, kCAN_IntRx_FIFO0, 1);
            }
            else if (argval == RT_DEVICE_FLAG_INT_TX)
            {
                rt_kprintf("%s RT_DEVICE_FLAG_INT_TX argval:0x%X\r\n", __FUNCTION__, argval);
                CAN_SetIntMode(HW_CAN1, kCAN_IntTxEmpty, 1);
            }
        
            break;
        case RT_CAN_CMD_GET_STATUS:
            errtype = CAN_GetErrStatus(HW_CAN1);
            can->status.rcverrcnt = errtype >> 24;
            can->status.snderrcnt = (errtype >> 16 & 0xFF);
            can->status.errcode = errtype & 0x07;
            if (arg != &can->status)
            {
                rt_memcpy(arg, &can->status, sizeof(can->status));
            }
            break;
        case RT_DEVICE_CTRL_CLR_INT:
            rt_kprintf("%s RT_DEVICE_CTRL_CLR_INT\r\n", __FUNCTION__);
            break;
        case RT_CAN_CMD_SET_FILTER:
            rt_kprintf("%s RT_CAN_CMD_SET_FILTER\r\n", __FUNCTION__);
            break;
        case RT_CAN_CMD_SET_BAUD:
            rt_kprintf("%s RT_CAN_CMD_SET_BAUD\r\n", __FUNCTION__);
            break;
        default:
            rt_kprintf("%s unknown cmd:%d\r\n", __FUNCTION__, cmd);
            break;
    }
    return RT_EOK;
}

static int sendmsg(struct rt_can_device *can, const void *buf, rt_uint32_t boxno)
{
    struct rt_can_msg *pmsg = (struct rt_can_msg *) buf;
    
    rt_kprintf("%s boxno:%d\r\n", __FUNCTION__, boxno);
    
    CAN_Send(HW_CAN1, pmsg->id, pmsg->data, pmsg->len);

    return RT_EOK;
}


static int recvmsg(struct rt_can_device *can, void *buf, rt_uint32_t boxno)
{
    struct rt_can_msg *pmsg = (struct rt_can_msg *) buf;
    
    int i;
    uint32_t id;
    uint8_t len;
    uint8_t is_remote;
    
    CAN_Receive(HW_CAN1, HW_CAN_RX_FIFO0, &id, pmsg->data, &len, &is_remote);
    pmsg->id = id;
    pmsg->len = len;
    pmsg->hdr = 0;
//    
//    rt_kprintf("id:0x%X len:%d\r\n", id, len);
//    for(i=0; i<len; i++)
//    {
//        printf(" %X", buf[i]);
//    }
//    printf("\r\n");
    
    rt_kprintf("%s\r\n", __FUNCTION__);
    return RT_EOK;
}

static const struct rt_can_ops canops =
{
    configure,
    control,
    sendmsg,
    recvmsg,
};

int rt_hw_can_init(const char *name)
{
    int result;

    uint32_t instance;
    sscanf(name, "can%d", &instance);
    
    rt_can[instance].config.baud_rate = CAN1MBaud;
    rt_can[instance].config.msgboxsz = 1;
    rt_can[instance].config.sndboxnumber = 1;
    rt_can[instance].config.mode = RT_CAN_MODE_NORMAL;
    rt_can[instance].config.privmode = 0;
    rt_can[instance].config.ticks = 50;
    rt_can[instance].config.maxhdr = 1;
    rt_hw_can_register(&rt_can[instance], name, &canops, RT_NULL);

    return result;
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

    rt_hw_can_isr(&rt_can[1], RT_CAN_EVENT_TX_DONE | 0 << 8);
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    
    /* full */
    if(CAN1->RF0 & CAN_RF0_RFFU0)
    {
        CAN1->RF0 = CAN_RF0_RFFU0;
    }
    
    /* overflow */
    if(CAN1->RF0 & CAN_RF0_RFOV0)
    {
        CAN1->RF0 = CAN_RF0_RFOV0;
    }
    
    rt_hw_can_isr(&rt_can[1], RT_CAN_EVENT_RXOF_IND | 0 << 8);
}

