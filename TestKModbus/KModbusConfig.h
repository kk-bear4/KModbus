#ifndef	__KMODBUSCONFIG_H__
#define	__KMODBUSCONFIG_H__

#include "KModbus.h"

#include "TestKModbus.h"
#include <windows.h>

#ifdef __cplusplus
	extern "C" {
#endif

#define	KMODBUS_X0_SIZE				(10000)
#define	KMODBUS_X1_SIZE				(10000)
#define	KMODBUS_X3_SIZE				(10000)
#define	KMODBUS_X4_SIZE				(10000)

#define	KMODBUS_X0_BUFSIZE			(((KMODBUS_X0_SIZE)+7)/8)
#define	KMODBUS_X1_BUFSIZE			(((KMODBUS_X1_SIZE)+7)/8)
#define	KMODBUS_X3_BUFSIZE			((KMODBUS_X3_SIZE))
#define	KMODBUS_X4_BUFSIZE			((KMODBUS_X4_SIZE))

#define	KMODBUS_LOOP_SWITCH			Sleep(1)
#define	_USE_NO_COMMNICATION_TIME_

#define	CRITICAL_SECTION_BEGIN		CriLock();
#define	CRITICAL_SECTION_END		CriUnlock();

#define	KMODBUS_ID				(1)
#define	KMODBUS_GETTICKCOUNT	GetTickCount
#define	KMODBUS_GETCOM			GetCom
#define	KMODBUS_PUTCOM			PutCom
#define	KMODBUS_PUTSCOM			PutsCom


#ifdef __cplusplus
	}
#endif

#endif	/* __KMODBUSCONFIG_H__ */

