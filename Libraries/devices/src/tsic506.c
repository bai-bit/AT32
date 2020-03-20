#include "tsic506.h"
uint8_t wait_edge_trigger(uint8_t status);

tsic_oper_t tsic_oper;
void Tsic_Init(tsic_oper_t *remap_tsic_oper)
{
    memcpy(&tsic_oper, remap_tsic_oper, sizeof(tsic_oper));
}
void tsic_start(void)
{
    wait_edge_trigger(0);
    wait_edge_trigger(1);
}

uint16_t tsic_data(uint8_t *val)
{
    uint8_t i = 0;
    uint16_t temp = 0;
    uint8_t check = 0;
        
    for(i = 0; i < 9; i++)
    {
        wait_edge_trigger(1);
        tsic_oper.tsic_delayus(50);
        if(tsic_oper.tsic_data_output())
            temp |= 1 << (8 - i);
        else
            wait_edge_trigger(0);
    }
    
    for(i = 0; i < 9; i++)
    {
        if(temp & (1 << i))
            check++;
    }
    
    if(check % 2)
        return 1;

    *val = (temp >> 1);
    return 0;
}

void tsic_stop(void)
{
    wait_edge_trigger(1);
}
uint8_t read_tsic506_byte(double *value)
{
    //�ж�PA7�ϵĸߵ͵�ƽ�仯
    //����жϣ�����ռ�ձ����ж�
    //tsic_start()
    //tsic_data()
    //tsic_stop()
    //ÿ��λ���Ǵ��½��ؿ�ʼ�ߵ�ƽ����
    //һ��������125us

    uint8_t temp1 = 0,temp2 = 0;
    uint32_t data = 0;
    tsic_oper.tsic_power(1);
    tsic_oper.tsic_delayus(120);
    
    wait_edge_trigger(1);
    tsic_start();
    
    if(tsic_data(&temp1))
        return 1;
    
    tsic_stop();
    tsic_start();
    if(tsic_data(&temp2))
        return 1;
    
    data |= temp2;
    data |= temp1 << 8;

    *value = ((double) data) / 2047 * 70 - 10;
    
    tsic_oper.tsic_power(0);

    return 0;
}

uint8_t wait_edge_trigger(uint8_t status)
{
    int i = 100000;

    while(tsic_oper.tsic_data_output() == status && i)
    {
        tsic_oper.tsic_delayus(1);
        i--;
    }
    if(i == 0)
        return 1;
  
    return 0;
}