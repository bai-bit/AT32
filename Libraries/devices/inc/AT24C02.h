#ifndef __AT24C02_H
#define __AT24C02_H

#include<iic.h>
#include<stdio.h>


void AT24CXX_Init(void);
uint8_t AT24CXX_check(void);
void AT24CXX_writebyte(uint32_t addr,uint8_t buf);
uint8_t AT24CXX_readbyte(uint32_t addr);
uint32_t AT24CXX_read(uint32_t addr,uint8_t *buf,uint32_t bufnum);
void AT24CXX_write(uint32_t addr,uint8_t *buf,uint32_t bufnum);

#endif
