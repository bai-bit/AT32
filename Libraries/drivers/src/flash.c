#include "flash.h"
#include<stdlib.h>

void flash_init(void)
{
    //开启时钟
    RCC->AHBEN |= RCC_AHBEN_FLASHEN;
    flash_lock();
}

void flash_lock(void)
{
    //判断flash_CTRL中lock位
    //写入两个键值
    if (FLASH->CTRL & FLASH_CTRL_LCK)
    {
        FLASH->FCKEY = FLASH_KEY1;
        FLASH->FCKEY = FLASH_KEY2;
    }
    
    if (FLASH->CTRL & FLASH_CTRL_LCK)
    {
        //return status
    }
}

uint16_t Flash_ReadHalfWord(uint32_t addr)
{
    return *(uint16_t *)addr; 
}

void Flash_WritePageWord(uint32_t addr, uint16_t *buf, uint16_t len)   
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        Flash_WriteHalfWord(addr, buf[i]);
        addr += 2;
	}  
} 

uint16_t FLASH_BUF[SECTOR_SIZE/2];
void flash_write(uint32_t addr, uint16_t *buf, uint16_t len)	
{
    uint32_t secpos = 0, offaddr = 0;	   
    uint16_t secoff = 0, secremain = 0;	   
    uint16_t i = 0;  
    
	if (addr < FLASH_BASE || (addr >= (FLASH_BASE+BANK1*SECTOR_SIZE)))
		return;		
							
	offaddr = addr - FLASH_BASE;		
	secpos = offaddr / SECTOR_SIZE;			
	secoff = (offaddr % SECTOR_SIZE) / 2;		
	secremain = SECTOR_SIZE / 2 - secoff;		
    
	if (len <= secremain)
		secremain = len;
    
	while (1) 
	{
		flash_read(secpos*SECTOR_SIZE+FLASH_BASE, FLASH_BUF, SECTOR_SIZE/2);
		for (i = 0; i < secremain; i++)
		{
			if (FLASH_BUF[secoff+i] != 0XFFFF)
				break;
		}
		if (i < secremain)
		{
			Flash_ErasePage(secpos*SECTOR_SIZE+FLASH_BASE);
			for (i = 0; i < secremain; i++)
			{
				FLASH_BUF[i+secoff] = buf[i];	  
			}
			Flash_WritePageWord(secpos*SECTOR_SIZE+FLASH_BASE, FLASH_BUF, SECTOR_SIZE/2);  
		}
		else
		{
			Flash_WritePageWord(addr, buf, secremain); 		
		}
		if (len == secremain)
			break;
        else
        {
            secpos++;				
            secoff = 0;					 
            buf += secremain;  	
            addr += (secremain*2);		   
            len -= secremain;	
            if (len > (SECTOR_SIZE / 2))
                secremain = SECTOR_SIZE / 2;
            else 
                secremain = len;
        }
    }
}


void flash_read(uint32_t addr, uint16_t *buf, uint16_t len)   	
{
	uint16_t i;
	
	for (i = 0; i < len; i++)
	{
		buf[i] = Flash_ReadHalfWord(addr);
		addr += 2;	
	}
}

FLASH_Status Flash_WaitProcess(uint32_t timeout)
{
    FLASH_Status status = FLASH_COMPLETE;
    
    status = Flash_GetStatus();
    
    while ((status == FLASH_FLAG_BSY) && timeout)
    {
        status = Flash_GetStatus();
        timeout--;
    }
    
    if (!timeout)
    {
        status = FLASH_TIMEOUT;
    }
    
    return status;
}

FLASH_Status Flash_GetStatus(void)
{
    FLASH_Status status = FLASH_COMPLETE;
    
    if (FLASH->STS & FLASH_BSY)
    {
        status = FLASH_BSY;
    }
    else if (FLASH->STS & FLASH_FLAG_PRGMFLR)
    {
        status = FLASH_PRGMFLR;
    }
    else if (FLASH->STS & FLASH_FLAG_WRPRTFLR )
    {
        status = FLASH_WRPRTFLR;
    }
    else
    {
        status = FLASH_COMPLETE;
    }
    
    return status;
}

FLASH_Status Flash_ErasePage(uint32_t addr)
{
    FLASH_Status status = FLASH_COMPLETE;
    
    status = Flash_WaitProcess(TIMEOUT);

    if (status == FLASH_COMPLETE)
    {
        FLASH->CTRL |= CTRL_PGERS_SET;
        FLASH->ADDR = addr;
        FLASH->CTRL |= CTRL_RSTR_SET;

        status = Flash_WaitProcess(TIMEOUT);
    
        FLASH->CTRL &= ~CTRL_PGERS_SET;
    }
    return status;
}
  
FLASH_Status Flash_WriteHalfWord(uint32_t addr, uint16_t data)
{
    FLASH_Status status = FLASH_COMPLETE;
    
    status = Flash_WaitProcess(TIMEOUT);
    if (status == FLASH_COMPLETE)
    {
        FLASH->CTRL |= CTRL_PRGM_SET;
        
        *(uint16_t *)addr = data;
    
        status = Flash_WaitProcess(TIMEOUT);
    
        FLASH->CTRL &= ~CTRL_PRGM_SET;
    }
    
    return status;
}
