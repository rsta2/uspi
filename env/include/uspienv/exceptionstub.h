//
// exceptionstub.h
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
#ifndef _uspienv_exceptionstub_h
#define _uspienv_exceptionstub_h

#include <uspienv/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARM_OPCODE_BRANCH(distance)	(0xEA000000 | (distance))
#define ARM_DISTANCE(from, to)		((u32 *) &(to) - (u32 *) &(from) - 2)

typedef struct TExceptionTable
{
	u32 Reset;
	u32 UndefinedInstruction;
	u32 SupervisorCall;
	u32 PrefetchAbort;
	u32 DataAbort;
	u32 Unused;
	u32 IRQ;
	u32 FIQ;
}
TExceptionTable;

#define ARM_EXCEPTION_TABLE_BASE	0x00000000

void IRQStub (void);

void InterruptHandler (void);

#ifdef __cplusplus
}
#endif

#endif
