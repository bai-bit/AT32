#ifndef __AT32_TIMER_H
#define __AT32_TIMER_H

#include <stdint.h>

#define HW_TIMER0       (0x00U)
#define HW_TIMER1       (0x01U)
#define HW_TIMER2       (0x02U)
#define HW_TIMER3       (0x03U)
#define HW_TIMER4       (0x04U)
#define HW_TIMER5       (0x05U)
#define HW_TIMER6       (0x06U)



#define HW_TIMER_CH0    (0x00U)
#define HW_TIMER_CH1    (0x01U)
#define HW_TIMER_CH2    (0x02U)
#define HW_TIMER_CH3    (0x03U)
     
void timer_init(uint32_t instance, uint32_t chl, uint32_t val);
void timer_set_int_mode(uint32_t instance, uint32_t chl, uint8_t val);
void timer_clear_counter(uint32_t instance);
void timer_start(uint32_t instance);
void timer_stop(uint32_t instance);
void timer_set_time(uint32_t instance, uint32_t chl, uint32_t us);
void timer_set_value(uint32_t instance, uint32_t chl, uint32_t val);
void timer_set_one_shot(uint32_t instance, uint32_t chl, uint8_t val);
uint32_t timer_get_value(uint32_t instance, uint32_t chl);
void timer_set_to_value(uint32_t instance, uint32_t chl, uint32_t val);

#endif