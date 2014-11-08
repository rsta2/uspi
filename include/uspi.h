//
// uspi.h
//
// Services provided by the USPi library
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

#ifdef __cplusplus
extern "C" {
#endif

//
// USPi initialization
//

// returns 0 on failure
int USPiInitialize (void);

//
// Mass storage device
//

// returns != 0 if available
int USPiMassStorageDeviceAvailable (void);

#define USPI_BLOCK_SIZE		512			// other block sizes are not supported

// ullOffset and nCount must be multiple of USPI_BLOCK_SIZE
// returns number of read bytes or < 0 on failure
int USPiMassStorageDeviceRead (unsigned long long ullOffset, void *pBuffer, unsigned nCount);

// ullOffset and nCount must be multiple of USPI_BLOCK_SIZE
// returns number of written bytes or < 0 on failure
int USPiMassStorageDeviceWrite (unsigned long long ullOffset, const void *pBuffer, unsigned nCount);

//
// Ethernet services
//
// (You should delay 2 seconds after USPiInitialize before accessing the Ethernet.)
//

// checks the controller only, not if Ethernet link is up
// returns != 0 if available
int USPiEthernetAvailable (void);

void USPiGetMACAddress (unsigned char Buffer[6]);

// returns 0 on failure
int USPiSendFrame (const void *pBuffer, unsigned nLength);

// pBuffer must have size USPI_FRAME_BUFFER_SIZE
// returns 0 on failure
#define USPI_FRAME_BUFFER_SIZE	1600
int USPiReceiveFrame (void *pBuffer, unsigned *pResultLength);

#ifdef __cplusplus
}
#endif

#endif
