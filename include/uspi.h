//
// uspi.h
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _uspi_h
#define _uspi_h

#ifndef __uspi__
	#include <uspi/dwhcidevice.h>
	#include <uspi/usbdevice.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Memory allocation
void *malloc (unsigned nSize);		// result must be 4-byte aligned
void free (void *pBlock);

// Synchronization
void EnterCritical (void);		// disable interrupts (nested calls possible)
void LeaveCritical (void);		// enable interrupts (nested calls possible)

// Timer
void MsDelay (unsigned nMilliSeconds);	
void usDelay (unsigned nMicroSeconds);

typedef void TKernelTimerHandler (unsigned hTimer, void *pParam, void *pContext);

unsigned StartKernelTimer (unsigned nDelay,		// in HZ units (see uspi/sysconfig.h)
			   TKernelTimerHandler *pHandler,
			   void *pParam, void *pContext);
void CancelKernelTimer (unsigned hTimer);

// Interrupt handling
typedef void TInterruptHandler (void *pParam);

void ConnectInterrupt (unsigned nIRQ, TInterruptHandler *pHandler, void *pParam);

// Property tags
int IsModelA (void);

int SetPowerStateOn (unsigned nDeviceId);

// Logger
#define LOG_PANIC	0
#define LOG_ERROR	1
#define LOG_WARNING	2
#define LOG_NOTICE	3
#define LOG_DEBUG	4

void LoggerWrite (const char *pSource, unsigned Severity, const char *pMessage, ...);

//void AddDevice (const char *pName, TDevice *pDevice, int bBlockDevice);

#ifndef NDEBUG

// Debugging support
void uspi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine);

void DebugHexdump (const void *pBuffer, unsigned nBufLen, const char *pSource /* = 0 */);

#endif

#ifdef __cplusplus
}
#endif

#endif
