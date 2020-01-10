#ifndef __AT24C02_H
#define __AT24C02_H

#include "gpio_iic.h"
#include <stdint.h>
#include <stdio.h>
#define AT24C02 1

#if AT24C01
#define EEPROM_ENDADDR 127
#elif AT24C02
#define EEPROM_ENDADDR 255
#endif

uint8_t AT24CXX_check(void);
void AT24CXX_writebyte(uint32_t addr,uint8_t buf);
uint8_t AT24CXX_readbyte(uint32_t addr);
uint32_t AT24CXX_read(uint32_t addr,uint8_t *buf,uint32_t bufnum);
void AT24CXX_write(uint32_t addr,uint8_t *buf,uint32_t bufnum);

#endif
