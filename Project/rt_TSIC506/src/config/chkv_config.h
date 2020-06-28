/**
  ******************************************************************************
  * @file    chkv_config.h
  * @author  YANDLD
  * @version V2.5
  * @date    2015.3.26
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com 
  * @note    virtual EEPORM implemntation
  ******************************************************************************
  */
#ifndef _CHKV_CFG_H_
#define _CHKV_CFG_H_


#include <string.h>
#include <stdlib.h>
#include "rtthread.h"

#define CHKV_DEBUG		1
#if ( CHKV_DEBUG == 1 )
#include <stdio.h>
#define CHKV_TRACE	printf
#else
#define CHKV_TRACE(...)
#endif

#define MAX_KV_NAME_SIZE     (16)
#define MAX_KV_DATA_SIZE     (64)

#define chkv_malloc       rt_malloc
#define chkv_free         rt_free



#endif


