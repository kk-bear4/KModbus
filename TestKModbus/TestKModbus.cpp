#include <windows.h>
#include <tchar.h>
#include "TestKModbus.h"

#include <thread>
#include <mutex>

std::mutex		g_mtx;

KModbus_t		hKModbus;

#define	DEVCOMPORT	_T("\\\\.\\COM11")

HANDLE	g_hCom;

bool OpenCom()
{
	g_hCom = CreateFile(
		DEVCOMPORT,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (g_hCom == INVALID_HANDLE_VALUE) {
		return false;
	}

	DCB	dcb;

	if (GetCommState(g_hCom, &dcb)) {
		dcb.BaudRate = 9600;
		dcb.fBinary = TRUE;
		dcb.ByteSize = 8;
		dcb.fParity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_DISABLE;
		dcb.fRtsControl = RTS_CONTROL_DISABLE;
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		dcb.fNull = FALSE;
		dcb.fAbortOnError = FALSE;

		SetCommState(g_hCom, &dcb);
	}

	COMMTIMEOUTS	CommTimeouts;

	GetCommTimeouts(g_hCom, &CommTimeouts);
	CommTimeouts.ReadIntervalTimeout = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	SetCommTimeouts(g_hCom, &CommTimeouts);

	return true;
}

void	CloseCom()
{
	CloseHandle(g_hCom);
}

unsigned short		x0[8];
unsigned short		x4[8];

void	dumpX0()
{
	printf("X0:");
	for (int i = 0; i < 8; i++) {
		printf(" %0d", (x0[i])? 1: 0);
	}
	printf("\n");
}
void	dumpX4()
{
	printf("X4:");
	for (int i = 0; i < 8; i++) {
		printf(" %04X", x4[i]);
	}
	printf("\n");
}
int main()
{
	KMODBUS_STATUS		ret;
	int					i, ReqQuit = 0;

	OpenCom();

	KModbus_Init(&hKModbus);

	for (i = 0; i < 8; i++) {
		x0[i] = KModbus_Get( 1 + i );
		KModbus_Set(10001 + i, 0xff00 );
		KModbus_Set(30001 + i, 0x3000 + i);
		x4[i] = KModbus_Get( 40001 + i);
	}
	dumpX0();
	dumpX4();

	std::thread t([&] {
		unsigned short	flag = 0;
		bool			bUpdate0, bUpdate4;
		int				ofs;

		while (x0[0] == 0) {
			::Sleep(25);

			bUpdate0 = false;
			bUpdate4 = false;

			for (ofs = 0; ofs <8; ofs++) {
				flag = KModbus_Get( 1 + ofs );
				if (x0[ofs] != flag) {
					x0[ofs] = flag;
					bUpdate0 = true;
				}
				flag = KModbus_Get(40001 + ofs);
				if (x4[ofs] != flag) {
					x4[ofs] = flag;
					bUpdate4 = true;
				}
			}
			if (bUpdate0) {
				dumpX0();
			}
			if (bUpdate4) {
				dumpX4();
			}

			if (x0[0] != 0) {
				ReqQuit = 1;
			}
		}
	});

	ret = KModbusServer( &hKModbus, &ReqQuit );

	CloseCom();

	t.join();
	return ret;
}

KMODBUS_STATUS	GetCom(unsigned char* c)
{
	DWORD	ComError;
	COMSTAT ComStat;
	DWORD	rlen;

	if (ClearCommError(g_hCom, &ComError, &ComStat)) {
		if (ComStat.cbInQue > 0) {
			if (ReadFile(g_hCom, c, 1, &rlen, NULL) && rlen == 1) {
				return KMODBUS_OK;
			}
		}
		return KMODBUS_NODATA;
	}
	return KMODBUS_INVALID_PARAM;
}
KMODBUS_STATUS	GetsCom(unsigned char* buf, int len)
{
	KMODBUS_STATUS		ret;

	while (len--) {
		ret = GetCom(buf++);
		if (ret != KMODBUS_OK) {
			return ret;
		}
	}
	return KMODBUS_OK;
}
KMODBUS_STATUS	PutCom(unsigned char c)
{
	DWORD	rlen;

	if (WriteFile(g_hCom, &c, 1, &rlen, NULL) && rlen == 1) {
		return KMODBUS_OK;
	}
	return KMODBUS_INVALID_PARAM;
}
KMODBUS_STATUS	PutsCom(unsigned char* buf, int len)
{
	DWORD	rlen;

	if (WriteFile(g_hCom, buf, len, &rlen, NULL) && rlen == len) {
		return KMODBUS_OK;
	}
	return KMODBUS_INVALID_PARAM;
}

void	CriLock(void)
{
	g_mtx.lock();
}
void	CriUnlock(void)
{
	g_mtx.unlock();
}
