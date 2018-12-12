//
// pagetable.c
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
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
#include <uspienv/pagetable.h>
#include <uspienv/armv6mmu.h>
#include <uspienv/sysconfig.h>
#include <uspienv/synchronize.h>
#include <uspienv/assert.h>

#if RASPPI == 1
#define TTBR_MODE	(  ARM_TTBR_INNER_CACHEABLE		\
			 | ARM_TTBR_OUTER_NON_CACHEABLE)
#else
#define TTBR_MODE	(  ARM_TTBR_INNER_WRITE_BACK		\
			 | ARM_TTBR_OUTER_WRITE_BACK)
#endif

void PageTable (TPageTable *pThis, u32 nMemSize)
{
	assert (pThis != 0);

	pThis->m_pTable = (u32 *) MEM_PAGE_TABLE1;

	assert (((u32) pThis->m_pTable & 0x3FFF) == 0);

	for (unsigned nEntry = 0; nEntry < 4096; nEntry++)
	{
		u32 nBaseAddress = MEGABYTE * nEntry;

#if RASPPI == 3
		if (nBaseAddress == MEM_COHERENT_REGION)
		{
			pThis->m_pTable[nEntry] = ARMV6MMUL1SECTION_COHERENT | nBaseAddress;
		}
		else
#endif
		if (nBaseAddress < nMemSize)
		{
			extern u8 _etext;
			if (nBaseAddress < (u32) &_etext)
			{
				pThis->m_pTable[nEntry] = ARMV6MMUL1SECTION_NORMAL | nBaseAddress;
			}
			else
			{
				pThis->m_pTable[nEntry] = ARMV6MMUL1SECTION_NORMAL_XN | nBaseAddress;
			}
		}
		else
		{
			pThis->m_pTable[nEntry] = ARMV6MMUL1SECTION_DEVICE | nBaseAddress;
		}
	}

	CleanDataCache ();
	DataSyncBarrier ();
}

void _PageTable (TPageTable *pThis)
{
}

u32 PageTableGetBaseAddress (TPageTable *pThis)
{
	assert (pThis != 0);

	return (u32) pThis->m_pTable | TTBR_MODE;
}
