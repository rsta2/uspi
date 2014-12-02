//
// usbstring.c
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
#include <uspi/usbstring.h>
#include <uspi/usbdevice.h>
#include <uspi/dwhcidevice.h>
#include <uspi/util.h>
#include <uspi/assert.h>
#include <uspios.h>

#define USBSTR_MIN_LENGTH	16

void USBString (TUSBString *pThis, struct TUSBDevice *pDevice)
{
	assert (pThis != 0);

	pThis->m_pDevice = pDevice;
	assert (pThis->m_pDevice != 0);

	pThis->m_pUSBString = 0;

	pThis->m_pString = malloc (sizeof (TString));
	assert (pThis->m_pString != 0);
	String (pThis->m_pString);
}

void USBStringCopy (TUSBString *pThis, TUSBString *pParent)
{
	assert (pThis != 0);
	assert (pParent != 0);
	pThis->m_pDevice = pParent->m_pDevice;

	pThis->m_pUSBString = 0;
	if (pParent->m_pUSBString != 0)
	{
		pThis->m_pUSBString = (TUSBStringDescriptor *) malloc (pParent->m_pUSBString->bLength);
		assert (pThis->m_pUSBString != 0);
		memcpy (pThis->m_pUSBString, pParent->m_pUSBString, pParent->m_pUSBString->bLength);
	}

	assert (pParent->m_pString != 0);
	pThis->m_pString = (TString *) malloc (sizeof (TString));
	assert (pThis->m_pString != 0);
	String2 (pThis->m_pString, StringGet (pParent->m_pString));
}

void _USBString (TUSBString *pThis)
{
	assert (pThis != 0);

	assert (pThis->m_pString != 0);
	_String (pThis->m_pString);
	free (pThis->m_pString);
	pThis->m_pString = 0;

	if (pThis->m_pUSBString != 0)
	{
		free (pThis->m_pUSBString);
		pThis->m_pUSBString = 0;
	}

	pThis->m_pDevice = 0;
}

boolean USBStringGetFromDescriptor (TUSBString *pThis, u8 ucID)
{
	assert (pThis != 0);
	assert (ucID > 0);

	if (pThis->m_pUSBString != 0)
	{
		free (pThis->m_pUSBString);
	}
	pThis->m_pUSBString = (TUSBStringDescriptor *) malloc (USBSTR_MIN_LENGTH);
	assert (pThis->m_pUSBString != 0);

	assert (pThis->m_pDevice != 0);
	if (DWHCIDeviceGetDescriptor (USBDeviceGetHost (pThis->m_pDevice), USBDeviceGetEndpoint0 (pThis->m_pDevice),
				      DESCRIPTOR_STRING, ucID,
				      pThis->m_pUSBString, USBSTR_MIN_LENGTH, REQUEST_IN) < 0)
	{
		return FALSE;
	}

	u8 ucLength = pThis->m_pUSBString->bLength;
	if (   ucLength < 2
	    || (ucLength & 1) != 0
	    || pThis->m_pUSBString->bDescriptorType != DESCRIPTOR_STRING)
	{
		return FALSE;
	}

	if (ucLength > USBSTR_MIN_LENGTH)
	{
		free (pThis->m_pUSBString);
		pThis->m_pUSBString = (TUSBStringDescriptor *) malloc (ucLength);
		assert (pThis->m_pUSBString != 0);

		if (DWHCIDeviceGetDescriptor (USBDeviceGetHost (pThis->m_pDevice), USBDeviceGetEndpoint0 (pThis->m_pDevice),
					      DESCRIPTOR_STRING, ucID,
					      pThis->m_pUSBString, ucLength, REQUEST_IN) != (int) ucLength)
		{
			return FALSE;
		}

		if (   pThis->m_pUSBString->bLength != ucLength
		    || (pThis->m_pUSBString->bLength & 1) != 0
		    || pThis->m_pUSBString->bDescriptorType != DESCRIPTOR_STRING)
		{
			return FALSE;
		}
	}

	// convert to ASCII string
	assert (pThis->m_pUSBString->bLength > 2);
	assert ((pThis->m_pUSBString->bLength & 1) == 0);
	size_t nLength = (pThis->m_pUSBString->bLength-2) / 2;

	assert (nLength <= (255-2) / 2);
	char Buffer[nLength+1];
	
	for (unsigned i = 0; i < nLength; i++)
	{
		u16 usChar = pThis->m_pUSBString->bString[i];
		if (   usChar < ' '
		    || usChar > '~')
		{
			usChar = '_';
		}
		
		Buffer[i] = (char) usChar;
	}
	Buffer[nLength] = '\0';

	assert (pThis->m_pString != 0);
	_String (pThis->m_pString);
	free (pThis->m_pString);

	pThis->m_pString = malloc (sizeof (TString));
	assert (pThis->m_pString != 0);
	String2 (pThis->m_pString, Buffer);

	return TRUE;
}

const char *USBStringGet (TUSBString *pThis)
{
	assert (pThis != 0);
	return StringGet (pThis->m_pString);
}
