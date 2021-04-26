#ifndef CM_CONFIG_H
#define CM_CONFIG_H

#include <rtthread.h>
#include <ulog.h>


#define CM_DEBUG		1

#if ( CM_DEBUG == 1 )
#define CM_TRACE	LOG_RAW
#else
#define CM_TRACE(...)
#endif

#define cm_malloc       rt_malloc
#define cm_free         rt_free

#endif
