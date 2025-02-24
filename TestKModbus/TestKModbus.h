#pragma once

#include "KModbus.h"

#ifdef __cplusplus
	extern "C" {
#endif

KMODBUS_STATUS	GetCom(unsigned char* c);
KMODBUS_STATUS	GetsCom(unsigned char* buf, int len);
KMODBUS_STATUS	PutCom(unsigned char c);
KMODBUS_STATUS	PutsCom(unsigned char* buf, int len);

void	CriLock(void);
void	CriUnlock(void);

#ifdef __cplusplus
	}
#endif
