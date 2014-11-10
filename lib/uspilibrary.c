//
// uspilibrary.c
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
#include <uspi/uspilibrary.h>
#include <uspi.h>
#include <uspios.h>
#include <uspi/util.h>
#include <uspi/assert.h>

static const char FromUSPi[] = "uspi";

static TUSPiLibrary *s_pLibrary = 0;

int USPiInitialize (void)
{
	assert (s_pLibrary == 0);
	s_pLibrary = (TUSPiLibrary *) malloc (sizeof (TUSPiLibrary));
	assert (s_pLibrary != 0);

	DeviceNameService (&s_pLibrary->NameService);
	DWHCIDevice (&s_pLibrary->DWHCI);
	USBStandardHub (&s_pLibrary->USBHub1, &s_pLibrary->DWHCI, USBSpeedHigh);
	s_pLibrary->pEth0 = 0;

	if (!DWHCIDeviceInitialize (&s_pLibrary->DWHCI))
	{
		LogWrite (FromUSPi, LOG_ERROR, "Cannot initialize USB host controller interface");

		_USBStandardHub (&s_pLibrary->USBHub1);
		_DWHCIDevice (&s_pLibrary->DWHCI);
		_DeviceNameService (&s_pLibrary->NameService);
		free (s_pLibrary);
		s_pLibrary = 0;

		return 0;
	}

	if (!USBStandardHubInitialize (&s_pLibrary->USBHub1))
	{
		LogWrite (FromUSPi, LOG_ERROR, "Cannot initialize USB root hub");

		_USBStandardHub (&s_pLibrary->USBHub1);
		_DWHCIDevice (&s_pLibrary->DWHCI);
		_DeviceNameService (&s_pLibrary->NameService);
		free (s_pLibrary);
		s_pLibrary = 0;

		return 0;
	}

	if (!USBStandardHubConfigure (&s_pLibrary->USBHub1))
	{
		LogWrite (FromUSPi, LOG_ERROR, "USB device enumeration failed");

		_USBStandardHub (&s_pLibrary->USBHub1);
		_DWHCIDevice (&s_pLibrary->DWHCI);
		_DeviceNameService (&s_pLibrary->NameService);
		free (s_pLibrary);
		s_pLibrary = 0;

		return 0;
	}

	s_pLibrary->pUKBD1 = (TUSBKeyboardDevice *) DeviceNameServiceGetDevice (DeviceNameServiceGet (), "ukbd1", FALSE);
	
	s_pLibrary->pUMSD1 = (TUSBBulkOnlyMassStorageDevice *) DeviceNameServiceGetDevice (DeviceNameServiceGet (), "umsd1", TRUE);

	s_pLibrary->pEth0 = (TSMSC951xDevice *) DeviceNameServiceGetDevice (DeviceNameServiceGet (), "eth0", FALSE);

	LogWrite (FromUSPi, LOG_DEBUG, "USPi library successfully initialized");

	return 1;
}

int USPiKeyboardAvailable (void)
{
	assert (s_pLibrary != 0);
	return s_pLibrary->pUKBD1 != 0;
}

void USPiKeyboardRegisterKeyPressedHandler (TUSPiKeyPressedHandler *pKeyPressedHandler)
{
	assert (s_pLibrary != 0);
	assert (s_pLibrary->pUKBD1 != 0);
	USBKeyboardDeviceRegisterKeyPressedHandler (s_pLibrary->pUKBD1, pKeyPressedHandler);
}

void USPiKeyboardRegisterShutdownHandler (TUSPiShutdownHandler *pShutdownHandler)
{
	assert (s_pLibrary != 0);
	assert (s_pLibrary->pUKBD1 != 0);
	USBKeyboardDeviceRegisterShutdownHandler (s_pLibrary->pUKBD1, pShutdownHandler);
}

void USPiKeyboardRegisterKeyStatusHandlerRaw (TKeyStatusHandlerRaw *pKeyStatusHandlerRaw)
{
	assert (s_pLibrary != 0);
	assert (s_pLibrary->pUKBD1 != 0);
	USBKeyboardDeviceRegisterKeyStatusHandlerRaw (s_pLibrary->pUKBD1, pKeyStatusHandlerRaw);
}

int USPiMassStorageDeviceAvailable (void)
{
	assert (s_pLibrary != 0);
	return s_pLibrary->pUMSD1 != 0;
}

int USPiMassStorageDeviceRead (unsigned long long ullOffset, void *pBuffer, unsigned nCount)
{
	assert (s_pLibrary != 0);
	assert (s_pLibrary->pUMSD1 != 0);

	if (USBBulkOnlyMassStorageDeviceSeek (s_pLibrary->pUMSD1, ullOffset) != ullOffset)
	{
		return -1;
	}

	return USBBulkOnlyMassStorageDeviceRead (s_pLibrary->pUMSD1, pBuffer, nCount);
}

int USPiMassStorageDeviceWrite (unsigned long long ullOffset, const void *pBuffer, unsigned nCount)
{
	assert (s_pLibrary != 0);
	assert (s_pLibrary->pUMSD1 != 0);

	if (USBBulkOnlyMassStorageDeviceSeek (s_pLibrary->pUMSD1, ullOffset) != ullOffset)
	{
		return -1;
	}

	return USBBulkOnlyMassStorageDeviceWrite (s_pLibrary->pUMSD1, pBuffer, nCount);
}

int USPiEthernetAvailable (void)
{
	assert (s_pLibrary != 0);
	return s_pLibrary->pEth0 != 0;
}

void USPiGetMACAddress (unsigned char Buffer[6])
{
	assert (s_pLibrary != 0);
	assert (s_pLibrary->pEth0 != 0);
	TMACAddress *pMACAddress = SMSC951xDeviceGetMACAddress (s_pLibrary->pEth0);

	assert (Buffer != 0);
	MACAddressCopyTo (pMACAddress, Buffer);
}

int USPiSendFrame (const void *pBuffer, unsigned nLength)
{
	assert (s_pLibrary != 0);
	assert (s_pLibrary->pEth0 != 0);
	return SMSC951xDeviceSendFrame (s_pLibrary->pEth0, pBuffer, nLength) ? 1 : 0;
}

int USPiReceiveFrame (void *pBuffer, unsigned *pResultLength)
{
	assert (s_pLibrary != 0);
	assert (s_pLibrary->pEth0 != 0);
	return SMSC951xDeviceReceiveFrame (s_pLibrary->pEth0, pBuffer, pResultLength) ? 1 : 0;
}
