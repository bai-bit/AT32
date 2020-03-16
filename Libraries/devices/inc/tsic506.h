#ifndef __TSIC506_H
#define __TSIC506_H

#include <stdint.h>
#include <string.h>

typedef struct tsic_oper  {
    void (*tsic_delayus)(uint32_t num);
    void (*tsic_delayms)(uint32_t num);
    void (*tsic_power)(uint8_t status);
    uint8_t (*tsic_data_output)(void);
}tsic_oper_t;
void Tsic_Init(tsic_oper_t *remap_tsic_oper);
uint8_t read_tsic506_byte(double *value);

#endif
