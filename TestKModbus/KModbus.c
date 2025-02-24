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
#include	"KModbus.h"
#include	<memory.h>

#define	GET_TICK(fd)			(*((fd)->GetTick))()
#define	GET_LAST_TICK(fd)		((fd)->LastTick)
#define	SET_LAST_TICK(fd,d)		((fd)->LastTick=(d))
#define	GET_NOCOMMTIME(fd)		((fd)->NoCommunicationTime)
#define	SET_NOCOMMTIME(fd,d)	((fd)->NoCommunicationTime=(d))

const int	QueryLength[18] = {
	0,
	6,		/* 01 */
	6,		/* 02 */
	6,		/* 03 */
	6,		/* 04 */
	6,		/* 05 */
	6,		/* 06 */
	0,
	6,		/* 08 */
	0,
	0,
	2,		/* 11 */
	2,		/* 12 */
	0,
	0,
	5,		/* 15 */
	5,		/* 16 */
	2		/* 17 */
};

static unsigned char	X0DM[ KMODBUS_X0_BUFSIZE ];
static unsigned char	X1DM[ KMODBUS_X1_BUFSIZE ];
static unsigned short	X3DM[ KMODBUS_X3_BUFSIZE ];
static unsigned short	X4DM[ KMODBUS_X4_BUFSIZE ];

static KMODBUS_STATUS	_SetXx(unsigned char *Base, int adrs, unsigned char *dt, int len)
{
	unsigned char	d, *pt;
	int				bit_d;
	int				idx_s;

	idx_s = 0;
	d = *dt++;
	bit_d = adrs % 8;

	pt = &Base[adrs / 8];

	while(len--) {
		if (d & (1 << idx_s)) {
			*pt |= (1 << bit_d);
		}
		else {
			*pt &= ~(1 << bit_d);
		}
		if (bit_d == 7) {
			++pt;
			bit_d = 0;
		}
		else {
			++bit_d;
		}
		if (idx_s == 7) {
			idx_s = 0;
			d = *dt++;
		}
		else {
			++idx_s;
		}
	}
	return KMODBUS_OK;
}

static KMODBUS_STATUS	_SetRegXx(unsigned short* Base, int adrs, unsigned char* dt, int len)
{
	unsigned short	*pt, u16;

	pt = &Base[adrs];

	while (len--) {
		u16 = ((unsigned short)(*dt++)) << 8;
		u16 |= ((unsigned short)(*dt++));
		*pt++ = u16;
	}
	return KMODBUS_OK;
}

KMODBUS_STATUS	SetX0(int adrs, unsigned char* dt, int len)
{
	KMODBUS_STATUS	ret;

	if (adrs + len > KMODBUS_X0_SIZE) {
		return KMODBUS_NON_EXISTENT_ADDRESS;
	}
	CRITICAL_SECTION_BEGIN
	ret = _SetXx(X0DM, adrs, dt, len);
	CRITICAL_SECTION_END
	return ret;
}

KMODBUS_STATUS	SetX1(int adrs, unsigned char* dt, int len)
{
	KMODBUS_STATUS	ret;

	if (adrs + len > KMODBUS_X1_SIZE) {
		return KMODBUS_NON_EXISTENT_ADDRESS;
	}
	CRITICAL_SECTION_BEGIN
	ret = _SetXx(X1DM, adrs, dt, len);
	CRITICAL_SECTION_END
	return ret;
}

KMODBUS_STATUS	SetX3(int adrs, unsigned char* dt, int len)
{
	KMODBUS_STATUS	ret;

	if (adrs + len > KMODBUS_X3_SIZE) {
		return KMODBUS_NON_EXISTENT_ADDRESS;
	}
	CRITICAL_SECTION_BEGIN
	ret = _SetRegXx(X3DM, adrs, dt, len);
	CRITICAL_SECTION_END
	return ret;
}

KMODBUS_STATUS	SetX4(int adrs, unsigned char* dt, int len)
{
	KMODBUS_STATUS	ret;

	if (adrs + len > KMODBUS_X4_SIZE) {
		return KMODBUS_NON_EXISTENT_ADDRESS;
	}
	CRITICAL_SECTION_BEGIN
	ret = _SetRegXx(X4DM, adrs, dt, len);
	CRITICAL_SECTION_END
	return ret;
}

static KMODBUS_STATUS	_GetXx(unsigned char* Base, int adrs, unsigned char* dt, int len)
{
	unsigned char	d, * pt;
	int				bit_s;
	int				idx_d;

	idx_d = 0;
	bit_s = adrs % 8;

	pt = &Base[adrs / 8];
	d = 0;

	while (len--) {
		if (*pt & (1 << bit_s)) {
			d |= (1 << idx_d);
		}
		if (bit_s == 7) {
			bit_s = 0;
			++pt;
		}
		else {
			++bit_s;
		}
		if (idx_d == 7) {
			idx_d = 0;
			*dt++ = d;
			d = 0;
		}
		else {
			++idx_d;
		}
	}
	if (idx_d != 0) {
		*dt = d;
	}
	return KMODBUS_OK;
}

static KMODBUS_STATUS	_GetRegXx(unsigned short* Base, int adrs, unsigned char* dt, int len)
{
	unsigned short* pt;
	unsigned short	u16;

	pt = &Base[adrs];

	while (len--) {
		u16 = *pt++;
		*dt++ = (unsigned char)(u16 >> 8);
		*dt++ = (unsigned char)(u16 & 0x00FF);
	}
	return KMODBUS_OK;
}

KMODBUS_STATUS	GetX0(int adrs, unsigned char* dt, int len)
{
	KMODBUS_STATUS	ret;

	if (adrs + len > KMODBUS_X0_SIZE) {
		return KMODBUS_NON_EXISTENT_ADDRESS;
	}
	CRITICAL_SECTION_BEGIN
	ret = _GetXx(X0DM, adrs, dt, len);
	CRITICAL_SECTION_END
	return ret;
}

KMODBUS_STATUS	GetX1(int adrs, unsigned char* dt, int len)
{
	KMODBUS_STATUS	ret;

	if (adrs + len > KMODBUS_X1_SIZE) {
		return KMODBUS_NON_EXISTENT_ADDRESS;
	}
	CRITICAL_SECTION_BEGIN
	ret = _GetXx(X1DM, adrs, dt, len);
	CRITICAL_SECTION_END
	return ret;
}

KMODBUS_STATUS	GetX3(int adrs, unsigned char* dt, int len)
{
	KMODBUS_STATUS	ret;

	if (adrs + len > KMODBUS_X3_SIZE) {
		return KMODBUS_NON_EXISTENT_ADDRESS;
	}
	CRITICAL_SECTION_BEGIN
	ret = _GetRegXx(X3DM, adrs, dt, len);
	CRITICAL_SECTION_END
	return ret;
}

KMODBUS_STATUS	GetX4(int adrs, unsigned char* dt, int len)
{
	KMODBUS_STATUS	ret;

	if (adrs + len > KMODBUS_X4_SIZE) {
		return KMODBUS_NON_EXISTENT_ADDRESS;
	}
	CRITICAL_SECTION_BEGIN
	ret = _GetRegXx(X4DM, adrs, dt, len);
	CRITICAL_SECTION_END
	return ret;
}


unsigned short	KModbus_Get(int adrs)
{
	KMODBUS_STATUS		ret;
	unsigned short		u16;
	unsigned char		u8[2];

	if (adrs < 1) {
		return 0x0000;
	}
	if (adrs < 10001) {
		ret = GetX0(adrs - 1, u8, 1);
		if (ret != KMODBUS_OK ) {
			return 0x0000;
		}
		return (u8[0] & 0x01) ? 0xFF00 : 0x0000;
	}
	if (adrs < 20001) {
		ret = GetX1(adrs - 10001, u8, 1);
		if (ret != KMODBUS_OK) {
			return 0x0000;
		}
		return (u8[0] & 0x01) ? 0xFF00 : 0x0000;
	}
	if (adrs < 30001) {
		return 0x0000;
	}
	if (adrs < 40001) {
		ret = GetX3(adrs - 30001, u8, 1);
		if (ret != KMODBUS_OK) {
			return 0x0000;
		}
		u16 = KModbud_B2N(u8);
		return u16;
	}
	if (adrs < 50001) {
		ret = GetX4(adrs - 40001, u8, 1);
		if (ret != KMODBUS_OK) {
			return 0x0000;
		}
		u16 = KModbud_B2N(u8);
		return u16;
	}
	return 0x0000;
}
void			KModbus_Set(int adrs, unsigned short reg)
{
	KMODBUS_STATUS		ret;
	unsigned char		u8[2];

	if (adrs < 1) {
		return;
	}
	if (adrs < 10001) {
		u8[0] = (reg == 0) ? 0x00 : 0x01;
		ret = SetX0(adrs - 1, u8, 1);
		return;
	}
	if (adrs < 20001) {
		u8[0] = (reg == 0) ? 0x00 : 0x01;
		ret = SetX1(adrs - 10001, u8, 1);
		return;
	}
	if (adrs < 30001) {
		return;
	}
	if (adrs < 40001) {
		u8[0] = (unsigned char)(reg >> 8);
		u8[1] = (unsigned char)(reg & 0x00FF);
		ret = SetX3(adrs - 30001, u8, 1);
		return;
	}
	if (adrs < 50001) {
		u8[0] = (unsigned char)(reg >> 8);
		u8[1] = (unsigned char)(reg & 0x00FF);
		ret = SetX4(adrs - 40001, u8, 1);
		return;
	}
}

static void ExceptionResponse(PKModbus_t hd, KMODBUS_STATUS errcode)
{
	unsigned short		crc16;

	hd->TxBuf[0] = hd->RxBuf[0];
	hd->TxBuf[1] = hd->RxBuf[1] | 0x80;
	switch (errcode) {
		case KMODBUS_NON_EXISTENT_ADDRESS:
			hd->TxBuf[2] = 0x02;
			break;
		case KMODBUS_UNSUPPORT_FUNCTION:
			hd->TxBuf[2] = 0x01;
			break;
		case KMODBUS_INVALID_PARAM:
		default:
			hd->TxBuf[2] = 0x03;
			break;
	}
	crc16 = KModbus_CalcCRC16(hd->TxBuf, 3);
	hd->TxBuf[3] = (unsigned char)(crc16 & 0x00FF);
	hd->TxBuf[4] = (unsigned char)(crc16 >> 8);

	hd->MessageCounter++;
	hd->Interface.Puts(hd->TxBuf, 5);
}

typedef	KMODBUS_STATUS(*ReadBitsFunc)(int adrs, unsigned char* dt, int len);
typedef	KMODBUS_STATUS(*ReadRegsFunc)(int adrs, unsigned char* dt, int len);

static KMODBUS_STATUS	entry_ReadBits(PKModbus_t hd, ReadBitsFunc func)
{
	KMODBUS_STATUS	ret;
	int				adrs, len, txlen;
	unsigned char*	txptr;
	unsigned char	bytecount;
	unsigned short	crc16;

	adrs = KModbud_B2N(&hd->RxBuf[2]);
	len = KModbud_B2N(&hd->RxBuf[4]);
	bytecount = (unsigned char)((len + 7) / 8);
	txlen = (int)bytecount + 3;

	txptr = hd->TxBuf;
	*txptr++ = hd->RxBuf[0];
	*txptr++ = hd->RxBuf[1];
	*txptr++ = bytecount;
	ret = (*func)(adrs, txptr, len);
	if (ret != KMODBUS_OK) {
		ExceptionResponse(hd, ret);
		return ret;
	}
	txptr += bytecount;
	crc16 = KModbus_CalcCRC16(hd->TxBuf, txlen);
	*txptr++ = (unsigned char)(crc16 & 0x00FF);
	*txptr = (unsigned char)(crc16 >> 8);
	txlen += 2;

	hd->MessageCounter++;
	return hd->Interface.Puts(hd->TxBuf, txlen);
}

static KMODBUS_STATUS	entry_ReadRegs(PKModbus_t hd, ReadRegsFunc func)
{
	KMODBUS_STATUS	ret;
	int				adrs, len, txlen;
	unsigned char*	txptr;
	unsigned char	bytecount;
	unsigned short	crc16;

	adrs = KModbud_B2N(&hd->RxBuf[2]);
	len = KModbud_B2N(&hd->RxBuf[4]);
	bytecount = (unsigned char)len * sizeof(unsigned short);
	txlen = (int)bytecount + 3;

	txptr = hd->TxBuf;
	*txptr++ = hd->RxBuf[0];
	*txptr++ = hd->RxBuf[1];
	*txptr++ = bytecount;
	ret = (*func)(adrs, txptr, len);
	if (ret != KMODBUS_OK) {
		ExceptionResponse(hd, ret);
		return ret;
	}
	txptr += bytecount;
	crc16 = KModbus_CalcCRC16(hd->TxBuf, txlen);
	*txptr++ = (unsigned char)(crc16 & 0x00FF);
	*txptr = (unsigned char)(crc16 >> 8);
	txlen += 2;

	hd->MessageCounter++;
	return hd->Interface.Puts(hd->TxBuf, txlen);
}

KMODBUS_STATUS	entry_ReadCoilStatus01(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;

	ret = entry_ReadBits(hd, GetX0);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}
KMODBUS_STATUS	entry_ReadInputStatus02(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;

	ret = entry_ReadBits(hd, GetX1);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

KMODBUS_STATUS	entry_ReadHoldingRegister03(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;

	ret = entry_ReadRegs( hd, GetX4);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

KMODBUS_STATUS	entry_ReadInputRegister04(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;

	ret = entry_ReadRegs(hd, GetX3);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

KMODBUS_STATUS	entry_ForceSingleCoil05(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;
	int				adrs, data;

	adrs = KModbud_B2N(&hd->RxBuf[2]);
	data = KModbud_B2N(&hd->RxBuf[4]);
	if (data != 0x0000 && data != 0xFF00) {
		return KMODBUS_INVALID_PARAM;
	}
	ret = SetX0(adrs, &hd->RxBuf[4], 1);
	if (ret != KMODBUS_OK) {
		ExceptionResponse(hd, ret);
		return ret;
	}
	hd->MessageCounter++;
	ret = hd->Interface.Puts(hd->RxBuf, 8 );
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

KMODBUS_STATUS	entry_PresetSingleRegister06(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;
	int				adrs;

	adrs = KModbud_B2N(&hd->RxBuf[2]);
	ret = SetX4(adrs, &hd->RxBuf[4], 1);
	if (ret != KMODBUS_OK) {
		ExceptionResponse(hd, ret);
		return ret;
	}

	hd->MessageCounter++;
	ret = hd->Interface.Puts(hd->RxBuf, 8);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

static KMODBUS_STATUS sub0_Diagnostics(PKModbus_t hd, unsigned short data)
{
	KMODBUS_STATUS	ret;

	ret = hd->Interface.Puts(hd->RxBuf, 8);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}
static KMODBUS_STATUS sub2_Diagnostics(PKModbus_t hd, unsigned short data)
{
	unsigned short	crc16;

	memcpy(hd->TxBuf, hd->RxBuf, 4);
	hd->TxBuf[4] = (unsigned char)(hd->DiagnosticRegister >> 8);
	hd->TxBuf[5] = (unsigned char)(hd->DiagnosticRegister & 0x00FF);
	crc16 = KModbus_CalcCRC16(hd->TxBuf, 6);
	hd->TxBuf[6] = (unsigned char)(crc16 & 0x00FF);
	hd->TxBuf[7] = (unsigned char)(crc16 >> 8);

	hd->MessageCounter++;
	return hd->Interface.Puts(hd->TxBuf, 8);
}
static KMODBUS_STATUS sub_resp_Diagnostics(PKModbus_t hd, unsigned short resp)
{
	unsigned short	crc16;

	memcpy(hd->TxBuf, hd->RxBuf, 4);
	hd->TxBuf[4] = (unsigned char)(resp >> 8);
	hd->TxBuf[5] = (unsigned char)(resp & 0x00FF);
	crc16 = KModbus_CalcCRC16(hd->TxBuf, 6);
	hd->TxBuf[6] = (unsigned char)(crc16 & 0x00FF);
	hd->TxBuf[7] = (unsigned char)(crc16 >> 8);

	hd->MessageCounter++;
	return hd->Interface.Puts(hd->TxBuf, 8);
}
KMODBUS_STATUS	entry_Diagnostics08(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;
	unsigned short	DiagnosticSubcode;
	unsigned short	data;

	DiagnosticSubcode = (unsigned short)KModbud_B2N(&hd->RxBuf[2]);
	data = (unsigned short)KModbud_B2N(&hd->RxBuf[4]);

	hd->MessageCounter++;

	if (hd->ListenOnlyMode != 0) {
		hd->NoResponseCount++;
		if (DiagnosticSubcode != 1) {
			return KMODBUS_OK;
		}
	}
	switch (DiagnosticSubcode) {
	case 0:
		ret = sub0_Diagnostics(hd, data);
		return ret;

	case 1:
		hd->MessageCounter = 0;
		if (data == 0xFF00) {
			hd->EventCounter = 0;
		}
		if (hd->ListenOnlyMode == 0) {
			ret = sub0_Diagnostics(hd, data);
		}
		hd->ListenOnlyMode = 0;
		return ret;

	case 2:
		ret = sub_resp_Diagnostics(hd, hd->DiagnosticRegister);
		return ret;

	case 4:
		hd->ListenOnlyMode = 1;
		return KMODBUS_OK;

	case 10:
		hd->MessageCounter = 0;
		hd->EventCounter = 0;
		hd->CRCErrorCounter = 0;
		hd->DiagnosticRegister = 0;
		hd->ExceptionErrorCount = 0;
		hd->NoResponseCount = 0;
		ret = sub0_Diagnostics(hd, data);
		return ret;

	case 11:
	case 14:
		ret = sub_resp_Diagnostics(hd, hd->MessageCounter);
		return ret;

	case 12:
		ret = sub_resp_Diagnostics(hd, hd->CRCErrorCounter);
		return ret;

	case 13:
	case 17:
		ret = sub_resp_Diagnostics(hd, hd->ExceptionErrorCount);
		return ret;

	case 15:
		ret = sub_resp_Diagnostics(hd, hd->NoResponseCount);
		return ret;

	case 18:
		ret = sub_resp_Diagnostics(hd, 0 );
		return ret;

	}
	ExceptionResponse(hd, 0x01);
	return KMODBUS_UNSUPPORT_FUNCTION;
}

KMODBUS_STATUS	entry_FetchCommunicationEventCounter11(PKModbus_t hd)
{
	unsigned short	crc16;

	hd->TxBuf[0] = hd->RxBuf[0];
	hd->TxBuf[1] = hd->RxBuf[1];
	hd->TxBuf[2] = 0x00;
	hd->TxBuf[3] = 0x00;
	hd->TxBuf[4] = (unsigned char)(hd->EventCounter >> 8);
	hd->TxBuf[5] = (unsigned char)(hd->EventCounter & 0x00FF);
	crc16 = KModbus_CalcCRC16(hd->TxBuf, 6);
	hd->TxBuf[6] = (unsigned char)(crc16 & 0x00FF);
	hd->TxBuf[7] = (unsigned char)(crc16 >> 8);

	hd->MessageCounter++;
	return hd->Interface.Puts(hd->TxBuf, 8);
}

KMODBUS_STATUS	entry_FetchCommunicationEventLog12(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;
	unsigned short	crc16;

	hd->TxBuf[0] = hd->RxBuf[0];
	hd->TxBuf[1] = hd->RxBuf[1];
	hd->TxBuf[2] = 8;
	hd->TxBuf[3] = 0x00;
	hd->TxBuf[4] = 0x00;
	hd->TxBuf[5] = (unsigned char)(hd->EventCounter >> 8);
	hd->TxBuf[6] = (unsigned char)(hd->EventCounter & 0x00FF);
	hd->TxBuf[7] = (unsigned char)(hd->MessageCounter >> 8);
	hd->TxBuf[8] = (unsigned char)(hd->MessageCounter & 0x00FF);
	hd->TxBuf[9] = 0;
	hd->TxBuf[10] = 0;
	crc16 = KModbus_CalcCRC16(hd->TxBuf, 11);
	hd->TxBuf[11] = (unsigned char)(crc16 & 0x00FF);
	hd->TxBuf[12] = (unsigned char)(crc16 >> 8);

	hd->MessageCounter++;
	ret = hd->Interface.Puts(hd->TxBuf, 13);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

KMODBUS_STATUS	entry_ForceMultipleCoils15(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;
	int				adrs, len, i;
	unsigned char	bytecount;
	unsigned short	crc16;
	unsigned char	*p_src, *p_des;

	adrs = KModbud_B2N(&hd->RxBuf[2]);
	len = KModbud_B2N(&hd->RxBuf[4]);
	bytecount = (unsigned char)((len + 7) / 8);
	if (hd->RxBuf[6] != bytecount) {
		return KMODBUS_INVALID_PARAM;
	}
	ret = SetX0(adrs, &hd->RxBuf[7], len);
	if (ret != KMODBUS_OK) {
		ExceptionResponse(hd, ret);
		return ret;
	}
	p_src = hd->RxBuf;
	p_des = hd->TxBuf;
	i = 6;
	while (i--) {
		*p_des++ = *p_src++;
	}
	crc16 = KModbus_CalcCRC16(hd->TxBuf, 6);
	*p_des++ = (unsigned char)(crc16 & 0x00FF);
	*p_des++ = (unsigned char)(crc16 >> 8);

	hd->MessageCounter++;
	ret = hd->Interface.Puts(hd->TxBuf, 8);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

KMODBUS_STATUS	entry_PresetMultipleRegisters16(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;
	int				adrs, len, i;
	unsigned char	bytecount;
	unsigned short	crc16;
	unsigned char* p_src, * p_des;

	adrs = KModbud_B2N(&hd->RxBuf[2]);
	len = KModbud_B2N(&hd->RxBuf[4]);
	bytecount = (unsigned char)((len * sizeof(unsigned short)));
	if (hd->RxBuf[6] != bytecount) {
		return KMODBUS_INVALID_PARAM;
	}
	ret = SetX4(adrs, &hd->RxBuf[7], len);
	if (ret != KMODBUS_OK) {
		ExceptionResponse(hd, ret);
		return ret;
	}
	p_src = hd->RxBuf;
	p_des = hd->TxBuf;
	i = 6;
	while (i--) {
		*p_des++ = *p_src++;
	}
	crc16 = KModbus_CalcCRC16(hd->TxBuf, 6);
	*p_des++ = (unsigned char)(crc16 & 0x00FF);
	*p_des++ = (unsigned char)(crc16 >> 8);

	hd->MessageCounter++;
	ret = hd->Interface.Puts(hd->TxBuf, 8);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

KMODBUS_STATUS	entry_ReportSlaveID17(PKModbus_t hd)
{
	KMODBUS_STATUS	ret;
	unsigned short	crc16;

	hd->TxBuf[0] = hd->RxBuf[0];
	hd->TxBuf[1] = hd->RxBuf[1];
	hd->TxBuf[2] = 8;
	hd->TxBuf[3] = hd ->ID;
	hd->TxBuf[4] = 0xFF;
	hd->TxBuf[5] = 0x00;
	hd->TxBuf[6] = 0x00;
	hd->TxBuf[7] = 0x00;
	hd->TxBuf[8] = 0x00;
	hd->TxBuf[9] = 0x00;
	hd->TxBuf[10] = 0x00;
	crc16 = KModbus_CalcCRC16(hd->TxBuf, 11);
	hd->TxBuf[11] = (unsigned char)(crc16 & 0x00FF);
	hd->TxBuf[12] = (unsigned char)(crc16 >> 8);

	hd->MessageCounter++;
	ret = hd->Interface.Puts(hd->TxBuf, 13);
	if (ret == KMODBUS_OK) {
		hd->EventCounter++;
	}
	return ret;
}

typedef KMODBUS_STATUS (*KModbusHandler)(PKModbus_t);

const KModbusHandler	KModbusHandler_Tbl[18] = {
	0,
	entry_ReadCoilStatus01,						/* 01 */
	entry_ReadInputStatus02,					/* 02 */
	entry_ReadHoldingRegister03,				/* 03 */
	entry_ReadInputRegister04,					/* 04 */
	entry_ForceSingleCoil05,					/* 05 */
	entry_PresetSingleRegister06,				/* 06 */
	0,
	entry_Diagnostics08,						/* 08 */
	0,
	0,
	entry_FetchCommunicationEventCounter11,		/* 11 */
	entry_FetchCommunicationEventLog12,			/* 12 */
	0,
	0,
	entry_ForceMultipleCoils15,					/* 15 */
	entry_PresetMultipleRegisters16,			/* 16 */
	entry_ReportSlaveID17						/* 17 */
};

KMODBUS_STATUS	KMODBUS_Get(unsigned char *c)
{
	
	return KMODBUS_NODATA;
}
KMODBUS_STATUS	KMODBUS_Gets(unsigned char *buf, int len)
{
	
	return KMODBUS_NODATA;
}
KMODBUS_STATUS	KMODBUS_Put(unsigned char c)
{
	
	return KMODBUS_OK;
}
KMODBUS_STATUS	KMODBUS_Puts(unsigned char *buf, int len)
{
	
	return KMODBUS_OK;
}

static KMODBUS_STATUS	KModbusGet( PKModbus_t hd, KMODBUS_TICK timeout, unsigned char *buf )
{
	KMODBUS_STATUS	ret;
	KMODBUS_TICK	st;
	
	st = GET_TICK(hd);
	
	do{
		ret = hd->Interface.Get(buf);
		if( ret == KMODBUS_OK ){
			return ret;
		}
		KMODBUS_LOOP_SWITCH; /* Avoidance of monopolization */
		
	}while( (timeout == KMODBUS_FOREVER) || GET_TICK(hd) - st < timeout );
	return KMODBUS_TIMEOUT;
}

/* Calculate the CRC16 of the buffer data */
unsigned short	KModbus_CalcCRC16(unsigned char* buf, int len)
{
	int				i, lsb;
	unsigned short	crc16 = 0xffff;
	
	while (len) {
		crc16 ^= (unsigned short)*buf++;
		for (i = 0; i < 8; i++) {
			lsb = crc16 & 0x0001;
			crc16 >>= 1;
			if (lsb) {
				crc16 ^= 0xA001;
			}
		}
		--len;
	}
	return crc16;
}

/* Convert 16bit little endian to native order */
unsigned short	KModbud_L2N(unsigned char* little16)
{
	unsigned short	naitive16;

	naitive16 = (*little16);
	++little16;
	naitive16 |= ((unsigned short)(*little16)) << 8;
	return naitive16;
}

/* Convert 16-bit big endian to native order */
unsigned short	KModbud_B2N(unsigned char* big16)
{
	unsigned short	naitive16;

	naitive16 = ((unsigned short)(*big16)) << 8;
	++big16;
	naitive16 |= (*big16);
	return naitive16;
}

void	KModbus_Init(PKModbus_t hd)
{
	hd->ID = KMODBUS_ID;
	hd->GetTick = KMODBUS_GETTICKCOUNT;
	hd->Interface.Get = KMODBUS_GETCOM;
	hd->Interface.Put = KMODBUS_PUTCOM;
	hd->Interface.Puts = KMODBUS_PUTSCOM;

	hd->ListenOnlyMode = 0;
	hd->EventCounter = 0;
	hd->MessageCounter = 0;
	hd->DiagnosticRegister = 0;
	hd->CRCErrorCounter = 0;
	hd->ExceptionErrorCount = 0;
	hd->NoResponseCount = 0;
	hd->NoCommunicationTime = 10;

	memset(X0DM, 0x00, sizeof(X0DM));
	memset(X1DM, 0x00, sizeof(X1DM));
	memset(X3DM, 0x00, sizeof(X3DM));
	memset(X4DM, 0x00, sizeof(X4DM));
}

KMODBUS_STATUS	KModbusServer(PKModbus_t hd, int* ResQuit)
{
	KMODBUS_STATUS	ret;
#ifdef _USE_NO_COMMNICATION_TIME_
	KMODBUS_TICK	now;
#endif
	unsigned char	cd, * pt;
	unsigned short	crc16;
	int				len, cnt;

	SET_LAST_TICK(hd, GET_TICK(hd));

	for(;;){

	sym_top:

		KMODBUS_LOOP_SWITCH; /* Avoidance of monopolization */

		/* Monitor quit requests */
		if (ResQuit) {
			if (*ResQuit) {
				return KMODBUS_OK;
			}
		}
		/* Waiting for ID code reception */
		ret = hd->Interface.Get(&cd);
		if (ret != KMODBUS_OK || hd->ID != cd) {
			goto sym_top;
		}
		SET_LAST_TICK(hd, GET_TICK(hd));
		hd->RxBuf[0] = cd;

		/* Function code reception */
		ret = KModbusGet(hd, GET_NOCOMMTIME(hd), &cd);
		if (ret != KMODBUS_OK) {
			goto sym_top;
		}
		if (cd > 0x17) {
			goto sym_top;
		}
		hd->RxBuf[1] = cd;

		/* Fixed-length partial read */
		len = QueryLength[cd];
		pt = (unsigned char*)&hd->RxBuf[2];
		cnt = len;
		while (cnt--) {
			ret = KModbusGet(hd, GET_NOCOMMTIME(hd), pt);
			if (ret != KMODBUS_OK) {
				goto sym_top;
			}
			++pt;
		}
		SET_LAST_TICK(hd, GET_TICK(hd));

		/* Variable length partial read */
		if (hd->RxBuf[1] == 15 || hd->RxBuf[1] == 16) {
			cnt = hd->RxBuf[6] + 2;
			len += cnt;
			while (cnt--) {
				ret = KModbusGet(hd, GET_NOCOMMTIME(hd), pt);
				if (ret != KMODBUS_OK) {
					goto sym_top;
				}
				++pt;
			}
			SET_LAST_TICK(hd, GET_TICK(hd));
		}

		/* Calculate the CRC16 of the buffer data */
		crc16 = KModbus_CalcCRC16(&hd->RxBuf[0], len);
		if (hd->RxBuf[len] != (crc16 & 0x00FF) || hd->RxBuf[len + 1] != (crc16 >> 8)) {
			hd->CRCErrorCounter++;
			goto sym_top;
		}

#ifdef _USE_NO_COMMNICATION_TIME_
		/* Check no communication time */
		for (;;) {
			ret = hd->Interface.Get(&cd);
			now = GET_TICK(hd);
			if (ret != KMODBUS_OK) {
				if (now - GET_LAST_TICK(hd) > GET_NOCOMMTIME(hd)) {
					break;
				}
			}
			else {
				SET_LAST_TICK(hd, now);
			}
			KMODBUS_LOOP_SWITCH; /* Avoidance of monopolization */
		}
#endif
		if (hd->RxBuf[1] > 0 && hd->RxBuf[1] < 18) {
			if (hd->ListenOnlyMode == 0 || hd->RxBuf[1] == 8) {
				if( KModbusHandler_Tbl[hd->RxBuf[1]] ){
					ret = (*KModbusHandler_Tbl[hd->RxBuf[1]])(hd);
				}
			}
		}

		if (ResQuit) {
			if (*ResQuit) {
				break;
			}
		}
	}
	return KMODBUS_OK;
}
