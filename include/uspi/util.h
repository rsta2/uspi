//
// util.h
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
#ifndef _uspi_util_h
#define _uspi_util_h

#include <uspi/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void *uspi_memset (void *pBuffer, int nValue, size_t nLength);

void *uspi_memcpy (void *pDest, const void *pSrc, size_t nLength);

int uspi_memcmp (const void *pBuffer1, const void *pBuffer2, size_t nLength);

size_t uspi_strlen (const char *pString);

int uspi_strcmp (const char *pString1, const char *pString2);

char *uspi_strcpy (char *pDest, const char *pSrc);

char *uspi_strncpy (char *pDest, const char *pSrc, size_t nMaxLen);

char *uspi_strcat (char *pDest, const char *pSrc);

u16 uspi_le2be16 (u16 usValue);

u32 uspi_le2be32 (u32 ulValue);

#ifdef __cplusplus
}
#endif

#endif
