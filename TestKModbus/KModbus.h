/*
MIT License

Copyright © 2024 Koji Kobayashi All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

*/
#ifndef	__KMODBUS_H__
#define	__KMODBUS_H__

#ifdef __cplusplus
	extern "C" {
#endif

#define	KMODBUS_MAX_RXBUF			(268)
#define	KMODBUS_MAX_TXBUF			(268)

#define	KMODBUS_NO_RECEIVED_DATA		(1)
#define	KMODBUS_OK						(0)
#define	KMODBUS_NODATA					(-1)
#define	KMODBUS_NOT_RESPONSE			(-2)
#define	KMODBUS_INVALID_PARAM			(-3)
#define	KMODBUS_TIMEOUT					(-4)
#define	KMODBUS_NON_EXISTENT_ADDRESS	(-5)
#define	KMODBUS_UNSUPPORT_FUNCTION		(-6)

typedef	int					KMODBUS_STATUS;
typedef	unsigned short		KMODBUS_ADDRESS;
typedef	unsigned short		KMODBUS_COIL_STATUS;
typedef	unsigned short		KMODBUS_INPUT_STATUS;
typedef	unsigned short		KMODBUS_HOLDING_REGISTER;
typedef	unsigned short		KMODBUS_INPUT_REGISTER;
typedef	unsigned short		KMODBUS_DIAGNOSTICS;
typedef	unsigned short		KMODBUS_EVENTCOUNTER;

typedef	unsigned long		KMODBUS_TICK;
#define	KMODBUS_FOREVER		((KMODBUS_TICK)(-1))

typedef	struct {
	KMODBUS_EVENTCOUNTER	counter;
	unsigned short			event_counter;
	unsigned char			event[2];
} KMODBUS_EVENTLOG;

typedef	struct {
	unsigned char			ByteCount;
	unsigned char			SlaveID;
	unsigned char			RunIndicators;
	unsigned char			Additionalinformation[1];
} REPORTSLAVEID;

typedef struct KModbusIF_t {
	KMODBUS_STATUS (*Get)(unsigned char* c);
	KMODBUS_STATUS (*Gets)(unsigned char* buf, int len);
	KMODBUS_STATUS (*Put)(unsigned char c);
	KMODBUS_STATUS (*Puts)(unsigned char* buf, int len);

} KModbusIF_t, *PKModbusIF_t;

typedef struct KModbusFunc_t {
	KMODBUS_STATUS (*ReadCoilStatus)(void* hd, KMODBUS_ADDRESS ad, int len, KMODBUS_COIL_STATUS* buf);
	KMODBUS_STATUS (*ReadInputStatus)(void* hd, KMODBUS_ADDRESS ad, int len, KMODBUS_COIL_STATUS* buf);
	KMODBUS_STATUS (*ReadHoldingRegister)(void* hd, KMODBUS_ADDRESS ad, int len, KMODBUS_HOLDING_REGISTER* buf);
	KMODBUS_STATUS (*ReadInputRegister)(void* hd, KMODBUS_ADDRESS ad, int len, KMODBUS_INPUT_REGISTER* buf);

	KMODBUS_STATUS (*ForceSingleCoil)(void* hd, KMODBUS_ADDRESS ad, KMODBUS_COIL_STATUS dt);
	KMODBUS_STATUS (*PresetSingleRegister)(void* hd, KMODBUS_ADDRESS ad, KMODBUS_HOLDING_REGISTER dt);
	KMODBUS_STATUS (*Diagnostics)(void* hd, KMODBUS_DIAGNOSTICS no, KMODBUS_HOLDING_REGISTER dt);

	KMODBUS_STATUS (*FetchCommunicationEventCounter)(void* hd, KMODBUS_EVENTCOUNTER* buf);
	KMODBUS_STATUS (*FetchCommunicationEventLog)(void* hd, KMODBUS_EVENTLOG* buf);

	KMODBUS_STATUS (*ForceMultipleCoils)(void* hd, KMODBUS_ADDRESS ad, int len, KMODBUS_COIL_STATUS* buf);
	KMODBUS_STATUS (*PresetMultipleRegisters)(void* hd, KMODBUS_ADDRESS ad, int len, KMODBUS_HOLDING_REGISTER* buf);

} KModbusFunc_t;

typedef struct KModbus_t {
	KModbusIF_t		Interface;
	KModbusFunc_t	FuncTable;
	KMODBUS_TICK	(*GetTick)(void);

	KMODBUS_TICK	LastTick;
	KMODBUS_TICK	NoCommunicationTime;

	unsigned char	RxBuf[KMODBUS_MAX_RXBUF];
	unsigned char	TxBuf[KMODBUS_MAX_TXBUF];

	unsigned short	ListenOnlyMode;
	unsigned short	EventCounter;
	unsigned short	MessageCounter;
	unsigned short	DiagnosticRegister;
	unsigned short	CRCErrorCounter;
	unsigned short	ExceptionErrorCount;
	unsigned short	NoResponseCount;

	unsigned char	ID;

} KModbus_t, * PKModbus_t;

#include "KModbusConfig.h"

unsigned short	KModbud_L2N(unsigned char* little16);
unsigned short	KModbud_B2N(unsigned char* big16);
unsigned short	KModbus_CalcCRC16(unsigned char* buf, int len);

void			KModbus_Init(PKModbus_t hd);
KMODBUS_STATUS	KModbusServer(PKModbus_t hd, int* ResQuit);

unsigned short	KModbus_Get(int adrs);
void			KModbus_Set(int adrs, unsigned short reg);

#ifdef __cplusplus
	}
#endif

#endif	/* __KMODBUS_H__ */
