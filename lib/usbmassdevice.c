//
// usbmassdevice.c
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
#include <uspi/usbmassdevice.h>
#include <uspi/usbhostcontroller.h>
#include <uspi/devicenameservice.h>
#include <uspi/util.h>
#include <uspi/macros.h>
#include <uspi/assert.h>
#include <uspios.h>

// USB Mass Storage Bulk-Only Transport

// Command Block Wrapper
typedef struct TCBW
{
	unsigned int	dCWBSignature,
#define CBWSIGNATURE		0x43425355
			dCWBTag,
			dCBWDataTransferLength;		// number of bytes
	unsigned char	bmCBWFlags,
#define CBWFLAGS_DATA_IN	0x80
			bCBWLUN		: 4,
#define CBWLUN			0
			Reserved1	: 4,
			bCBWCBLength	: 5,		// valid length of the CBWCB in bytes
			Reserved2	: 3;
	// CBWCB follows
}
PACKED TCBW;

// Command Status Wrapper
typedef struct TCSW
{
	unsigned int	dCSWSignature,
#define CSWSIGNATURE		0x53425355
			dCSWTag,
			dCSWDataResidue;		// difference in amount of data processed
	unsigned char	bCSWStatus;
#define CSWSTATUS_PASSED	0x00
#define CSWSTATUS_FAILED	0x01
#define CSWSTATUS_PHASE_ERROR	0x02
}
PACKED TCSW;

// SCSI Transparent Command Set

typedef struct TSCSIInquiry
{
	unsigned char	OperationCode,
#define SCSI_OP_INQUIRY		0x12
			LogicalUnitNumberEVPD,
			PageCode,
			Reserved,
			AllocationLength,
			Control;
#define SCSI_INQUIRY_CONTROL	0x00
}
PACKED TSCSIInquiry;

typedef struct TCBWInquiry
{
	TCBW		CBW;
	TSCSIInquiry	SCSIInquiry;
	unsigned char	Padding[10];
}
PACKED TCBWInquiry;

typedef struct TSCSIInquiryResponse
{
	unsigned char	PeripheralDeviceType	: 5,
#define SCSI_PDT_DIRECT_ACCESS_BLOCK	0x00			// SBC-2 command set (or above)
#define SCSI_PDT_DIRECT_ACCESS_RBC	0x0E			// RBC command set
			PeripheralQualifier	: 3,		// 0: device is connected to this LUN
			DeviceTypeModifier	: 7,
			RMB			: 1,		// 1: removable media
			ANSIApprovedVersion	: 3,
			ECMAVersion		: 3,
			ISOVersion		: 2,
			Reserved1,
			AdditionalLength,
			Reserved2[3],
			VendorIdentification[8],
			ProductIdentification[16],
			ProductRevisionLevel[4];
}
PACKED TSCSIInquiryResponse;

typedef struct TSCSIRead10
{
	unsigned char	OperationCode,
#define SCSI_OP_READ		0x28
			Reserved1;
	unsigned int	LogicalBlockAddress;			// big endian
	unsigned char	Reserved2;
	unsigned short	TransferLength;				// block count, big endian
	unsigned char	Control;
#define SCSI_READ_CONTROL	0x00
}
PACKED TSCSIRead10;

typedef struct TCBWRead
{
	TCBW		CBW;
	TSCSIRead10	SCSIRead;
	unsigned char	Padding[6];
}
PACKED TCBWRead;

typedef struct TSCSIWrite10
{
	unsigned char	OperationCode,
#define SCSI_OP_WRITE		0x2A
			Flags;
#define SCSI_WRITE_FUA		0x08
	unsigned int	LogicalBlockAddress;			// big endian
	unsigned char	Reserved;
	unsigned short	TransferLength;				// block count, big endian
	unsigned char	Control;
#define SCSI_WRITE_CONTROL	0x00
}
PACKED TSCSIWrite10;

typedef struct TCBWWrite
{
	TCBW		CBW;
	TSCSIWrite10	SCSIWrite;
	unsigned char	Padding[6];
}
PACKED TCBWWrite;

static unsigned s_nDeviceNumber = 1;

static const char FromUmsd[] = "umsd";

int USBBulkOnlyMassStorageDeviceTryRead (TUSBBulkOnlyMassStorageDevice *pThis, void *pBuffer, unsigned nCount);
int USBBulkOnlyMassStorageDeviceTryWrite (TUSBBulkOnlyMassStorageDevice *pThis, const void *pBuffer, unsigned nCount);
int USBBulkOnlyMassStorageDeviceReset (TUSBBulkOnlyMassStorageDevice *pThis);

void USBBulkOnlyMassStorageDevice (TUSBBulkOnlyMassStorageDevice *pThis, TUSBDevice *pDevice)
{
	assert (pThis != 0);

	USBDeviceCopy (&pThis->m_USBDevice, pDevice);
	pThis->m_USBDevice.Configure = USBBulkOnlyMassStorageDeviceConfigure;

	pThis->m_pEndpointIn = 0;
	pThis->m_pEndpointOut = 0;
	pThis->m_nCWBTag = 0;
	pThis->m_ullOffset = 0;
}

void _USBBulkOnlyMassStorageDevice (TUSBBulkOnlyMassStorageDevice *pThis)
{
	assert (pThis != 0);

	if (pThis->m_pEndpointOut != 0)
	{
		_USBEndpoint (pThis->m_pEndpointOut);
		free (pThis->m_pEndpointOut);
		pThis->m_pEndpointOut =  0;
	}
	
	if (pThis->m_pEndpointIn != 0)
	{
		_USBEndpoint (pThis->m_pEndpointIn);
		free (pThis->m_pEndpointIn);
		pThis->m_pEndpointIn =  0;
	}
	
	_USBDevice (&pThis->m_USBDevice);
}

boolean USBBulkOnlyMassStorageDeviceConfigure (TUSBDevice *pUSBDevice)
{
	TUSBBulkOnlyMassStorageDevice *pThis = (TUSBBulkOnlyMassStorageDevice *) pUSBDevice;
	assert (pThis != 0);

	TUSBConfigurationDescriptor *pConfDesc =
		(TUSBConfigurationDescriptor *) USBDeviceGetDescriptor (&pThis->m_USBDevice, DESCRIPTOR_CONFIGURATION);
	if (   pConfDesc == 0
	    || pConfDesc->bNumInterfaces <  1)
	{
		USBDeviceConfigurationError (&pThis->m_USBDevice, FromUmsd);

		return FALSE;
	}

	TUSBInterfaceDescriptor *pInterfaceDesc =
		(TUSBInterfaceDescriptor *) USBDeviceGetDescriptor (&pThis->m_USBDevice, DESCRIPTOR_INTERFACE);
	if (   pInterfaceDesc == 0
	    || pInterfaceDesc->bInterfaceNumber		!= 0x00
	    || pInterfaceDesc->bAlternateSetting	!= 0x00
	    || pInterfaceDesc->bNumEndpoints		<  2
	    || pInterfaceDesc->bInterfaceClass		!= 0x08		// Mass Storage Class
	    || pInterfaceDesc->bInterfaceSubClass	!= 0x06		// SCSI Transparent Command Set
	    || pInterfaceDesc->bInterfaceProtocol	!= 0x50)	// Bulk-Only Transport
	{
		USBDeviceConfigurationError (&pThis->m_USBDevice, FromUmsd);

		return FALSE;
	}

	const TUSBEndpointDescriptor *pEndpointDesc;
	while ((pEndpointDesc = (TUSBEndpointDescriptor *) USBDeviceGetDescriptor (&pThis->m_USBDevice, DESCRIPTOR_ENDPOINT)) != 0)
	{
		if ((pEndpointDesc->bmAttributes & 0x3F) == 0x02)		// Bulk
		{
			if ((pEndpointDesc->bEndpointAddress & 0x80) == 0x80)	// Input
			{
				if (pThis->m_pEndpointIn != 0)
				{
					USBDeviceConfigurationError (&pThis->m_USBDevice, FromUmsd);

					return FALSE;
				}

				pThis->m_pEndpointIn = (TUSBEndpoint *) malloc (sizeof (TUSBEndpoint));
				assert (pThis->m_pEndpointIn != 0);
				USBEndpoint2 (pThis->m_pEndpointIn, &pThis->m_USBDevice, pEndpointDesc);
			}
			else							// Output
			{
				if (pThis->m_pEndpointOut != 0)
				{
					USBDeviceConfigurationError (&pThis->m_USBDevice, FromUmsd);

					return FALSE;
				}

				pThis->m_pEndpointOut = (TUSBEndpoint *) malloc (sizeof (TUSBEndpoint));
				assert (pThis->m_pEndpointOut != 0);
				USBEndpoint2 (pThis->m_pEndpointOut, &pThis->m_USBDevice, pEndpointDesc);
			}
		}
	}

	if (   pThis->m_pEndpointIn  == 0
	    || pThis->m_pEndpointOut == 0)
	{
		USBDeviceConfigurationError (&pThis->m_USBDevice, FromUmsd);

		return FALSE;
	}

	if (!USBDeviceConfigure (&pThis->m_USBDevice))
	{
		LogWrite (FromUmsd, LOG_ERROR, "Cannot set configuration");

		return FALSE;
	}

	TCBWInquiry CBWInquiry;

	CBWInquiry.CBW.dCWBSignature		= CBWSIGNATURE;
	CBWInquiry.CBW.dCWBTag			= ++pThis->m_nCWBTag;
	CBWInquiry.CBW.dCBWDataTransferLength	= sizeof (TSCSIInquiryResponse);
	CBWInquiry.CBW.bmCBWFlags		= CBWFLAGS_DATA_IN;
	CBWInquiry.CBW.bCBWLUN			= CBWLUN;
	CBWInquiry.CBW.Reserved1		= 0;
	CBWInquiry.CBW.bCBWCBLength		= sizeof (TSCSIInquiry);
	CBWInquiry.CBW.Reserved2		= 0;

	CBWInquiry.SCSIInquiry.OperationCode		= SCSI_OP_INQUIRY;
	CBWInquiry.SCSIInquiry.LogicalUnitNumberEVPD	= 0;
	CBWInquiry.SCSIInquiry.PageCode			= 0;
	CBWInquiry.SCSIInquiry.Reserved			= 0;
	CBWInquiry.SCSIInquiry.AllocationLength		= sizeof (TSCSIInquiryResponse);
	CBWInquiry.SCSIInquiry.Control			= SCSI_INQUIRY_CONTROL;

	TSCSIInquiryResponse SCSIInquiryResponse;
	TCSW CSW;

	TUSBHostController *pHost = USBDeviceGetHost (&pThis->m_USBDevice);
	assert (pHost != 0);

	if (   DWHCIDeviceTransfer (pHost, pThis->m_pEndpointOut, &CBWInquiry, sizeof CBWInquiry) < 0
	    || DWHCIDeviceTransfer (pHost, pThis->m_pEndpointIn, &SCSIInquiryResponse, sizeof SCSIInquiryResponse) != (int) sizeof SCSIInquiryResponse
	    || DWHCIDeviceTransfer (pHost, pThis->m_pEndpointIn, &CSW, sizeof CSW) != (int) sizeof CSW)
	{
		LogWrite (FromUmsd, LOG_ERROR, "Device does not respond");

		return FALSE;
	}

	if (   CSW.dCSWSignature != CSWSIGNATURE
	    || CSW.dCSWTag       != pThis->m_nCWBTag)
	{
		LogWrite (FromUmsd, LOG_ERROR, "Invalid inquiry response");

		return FALSE;
	}

	if (CSW.bCSWStatus != CSWSTATUS_PASSED)
	{
		LogWrite (FromUmsd, LOG_ERROR, "Inquiry failed (status 0x%02X)", (unsigned) CSW.bCSWStatus);

		return FALSE;
	}

	if (CSW.dCSWDataResidue != 0)
	{
		LogWrite (FromUmsd, LOG_ERROR, "Invalid data residue on inquiry");

		return FALSE;
	}
	
	if (SCSIInquiryResponse.PeripheralDeviceType != SCSI_PDT_DIRECT_ACCESS_BLOCK)
	{
		LogWrite (FromUmsd, LOG_ERROR, "Unsupported device type: 0x%02X", (unsigned) SCSIInquiryResponse.PeripheralDeviceType);
		
		return FALSE;
	}

	TString DeviceName;
	String (&DeviceName);
	StringFormat (&DeviceName, "umsd%u", s_nDeviceNumber++);
	DeviceNameServiceAddDevice (DeviceNameServiceGet (), StringGet (&DeviceName), pThis, TRUE);

	_String (&DeviceName);
	
	return TRUE;
}

int USBBulkOnlyMassStorageDeviceRead (TUSBBulkOnlyMassStorageDevice *pThis, void *pBuffer, unsigned nCount)
{
	assert (pThis != 0);

	unsigned nTries = 4;

	int nResult;

	do
	{
		nResult = USBBulkOnlyMassStorageDeviceTryRead (pThis, pBuffer, nCount);

		if (nResult != (int) nCount)
		{
			int nStatus = USBBulkOnlyMassStorageDeviceReset (pThis);
			if (nStatus != 0)
			{
				return nStatus;
			}
		}
	}
	while (   nResult != (int) nCount
	       && --nTries > 0);

	return nResult;
}

int USBBulkOnlyMassStorageDeviceWrite (TUSBBulkOnlyMassStorageDevice *pThis, const void *pBuffer, unsigned nCount)
{
	assert (pThis != 0);

	unsigned nTries = 4;

	int nResult;

	do
	{
		nResult = USBBulkOnlyMassStorageDeviceTryWrite (pThis, pBuffer, nCount);

		if (nResult != (int) nCount)
		{
			int nStatus = USBBulkOnlyMassStorageDeviceReset (pThis);
			if (nStatus != 0)
			{
				return nStatus;
			}
		}
	}
	while (   nResult != (int) nCount
	       && --nTries > 0);

	return nResult;
}

unsigned long long USBBulkOnlyMassStorageDeviceSeek (TUSBBulkOnlyMassStorageDevice *pThis, unsigned long long ullOffset)
{
	assert (pThis != 0);

	pThis->m_ullOffset = ullOffset;

	return pThis->m_ullOffset;
}

int USBBulkOnlyMassStorageDeviceTryRead (TUSBBulkOnlyMassStorageDevice *pThis, void *pBuffer, unsigned nCount)
{
	assert (pThis != 0);

	assert (pBuffer != 0);

	if (   (pThis->m_ullOffset & UMSD_BLOCK_MASK) != 0
	    || pThis->m_ullOffset > UMSD_MAX_OFFSET)
	{
		return -1;
	}
	unsigned nBlockAddress = (unsigned) (pThis->m_ullOffset >> UMSD_BLOCK_SHIFT);

	if ((nCount & UMSD_BLOCK_MASK) != 0)
	{
		return -1;
	}
	unsigned short usTransferLength = (unsigned short) (nCount >> UMSD_BLOCK_SHIFT);

	//LogWrite (FromUmsd, LOG_DEBUG, "TryRead %u/0x%X/%u", nBlockAddress, (unsigned) pBuffer, (unsigned) usTransferLength);

	TCBWRead CBWRead;

	CBWRead.CBW.dCWBSignature		= CBWSIGNATURE;
	CBWRead.CBW.dCWBTag			= ++pThis->m_nCWBTag;
	CBWRead.CBW.dCBWDataTransferLength	= nCount;
	CBWRead.CBW.bmCBWFlags			= CBWFLAGS_DATA_IN;
	CBWRead.CBW.bCBWLUN			= CBWLUN;
	CBWRead.CBW.Reserved1			= 0;
	CBWRead.CBW.bCBWCBLength		= sizeof (TSCSIRead10);
	CBWRead.CBW.Reserved2			= 0;

	CBWRead.SCSIRead.OperationCode		= SCSI_OP_READ;
	CBWRead.SCSIRead.Reserved1		= 0;
	CBWRead.SCSIRead.LogicalBlockAddress	= uspi_le2be32 (nBlockAddress);
	CBWRead.SCSIRead.Reserved2		= 0;
	CBWRead.SCSIRead.TransferLength		= uspi_le2be16 (usTransferLength);
	CBWRead.SCSIRead.Control		= SCSI_READ_CONTROL;

	TUSBHostController *pHost = USBDeviceGetHost (&pThis->m_USBDevice);
	assert (pHost != 0);
	
	TCSW CSW;

	if (   DWHCIDeviceTransfer (pHost, pThis->m_pEndpointOut, &CBWRead, sizeof CBWRead) < 0
	    || DWHCIDeviceTransfer (pHost, pThis->m_pEndpointIn, pBuffer, nCount) != (int) nCount
	    || DWHCIDeviceTransfer (pHost, pThis->m_pEndpointIn, &CSW, sizeof CSW) != (int) sizeof CSW)
	{
		return -1;
	}

	if (   CSW.dCSWSignature   != CSWSIGNATURE
	    || CSW.dCSWTag         != pThis->m_nCWBTag
	    || CSW.bCSWStatus      != CSWSTATUS_PASSED
	    || CSW.dCSWDataResidue != 0)
	{
		return -1;
	}

	return nCount;
}

int USBBulkOnlyMassStorageDeviceTryWrite (TUSBBulkOnlyMassStorageDevice *pThis, const void *pBuffer, unsigned nCount)
{
	assert (pThis != 0);

	assert (pBuffer != 0);

	if (   (pThis->m_ullOffset & UMSD_BLOCK_MASK) != 0
	    || pThis->m_ullOffset > UMSD_MAX_OFFSET)
	{
		return -1;
	}
	unsigned nBlockAddress = (unsigned) (pThis->m_ullOffset >> UMSD_BLOCK_SHIFT);

	if ((nCount & UMSD_BLOCK_MASK) != 0)
	{
		return -1;
	}
	unsigned short usTransferLength = (unsigned short) (nCount >> UMSD_BLOCK_SHIFT);

	//LogWrite (FromUmsd, LOG_DEBUG, "TryWrite %u/0x%X/%u", nBlockAddress, (unsigned) pBuffer, (unsigned) usTransferLength);

	TCBWWrite CBWWrite;

	CBWWrite.CBW.dCWBSignature		= CBWSIGNATURE;
	CBWWrite.CBW.dCWBTag			= ++pThis->m_nCWBTag;
	CBWWrite.CBW.dCBWDataTransferLength	= nCount;
	CBWWrite.CBW.bmCBWFlags			= 0;
	CBWWrite.CBW.bCBWLUN			= CBWLUN;
	CBWWrite.CBW.Reserved1			= 0;
	CBWWrite.CBW.bCBWCBLength		= sizeof (TSCSIWrite10);
	CBWWrite.CBW.Reserved2			= 0;

	CBWWrite.SCSIWrite.OperationCode	= SCSI_OP_WRITE;
	CBWWrite.SCSIWrite.Flags		= SCSI_WRITE_FUA;
	CBWWrite.SCSIWrite.LogicalBlockAddress	= uspi_le2be32 (nBlockAddress);
	CBWWrite.SCSIWrite.Reserved		= 0;
	CBWWrite.SCSIWrite.TransferLength	= uspi_le2be16 (usTransferLength);
	CBWWrite.SCSIWrite.Control		= SCSI_WRITE_CONTROL;

	TUSBHostController *pHost = USBDeviceGetHost (&pThis->m_USBDevice);
	assert (pHost != 0);
	
	TCSW CSW;

	if (   DWHCIDeviceTransfer (pHost, pThis->m_pEndpointOut, &CBWWrite, sizeof CBWWrite) < 0
	    || DWHCIDeviceTransfer (pHost, pThis->m_pEndpointOut, (void *) pBuffer, nCount) < 0
	    || DWHCIDeviceTransfer (pHost, pThis->m_pEndpointIn, &CSW, sizeof CSW) != (int) sizeof CSW)
	{
		return -1;
	}

	if (   CSW.dCSWSignature   != CSWSIGNATURE
	    || CSW.dCSWTag         != pThis->m_nCWBTag
	    || CSW.bCSWStatus      != CSWSTATUS_PASSED
	    || CSW.dCSWDataResidue != 0)
	{
		return -1;
	}

	return nCount;
}

int USBBulkOnlyMassStorageDeviceReset (TUSBBulkOnlyMassStorageDevice *pThis)
{
	assert (pThis != 0);

	TUSBHostController *pHost = USBDeviceGetHost (&pThis->m_USBDevice);
	assert (pHost != 0);
	
	if (DWHCIDeviceControlMessage (pHost, USBDeviceGetEndpoint0 (&pThis->m_USBDevice), 0x21, 0xFF, 0, 0x00, 0, 0) < 0)
	{
		LogWrite (FromUmsd, LOG_DEBUG, "Cannot reset device");

		return -1;
	}

	if (DWHCIDeviceControlMessage (pHost, USBDeviceGetEndpoint0 (&pThis->m_USBDevice), 0x02, 1, 0, 1, 0, 0) < 0)
	{
		LogWrite (FromUmsd, LOG_DEBUG, "Cannot clear halt on endpoint 1");

		return -1;
	}

	if (DWHCIDeviceControlMessage (pHost, USBDeviceGetEndpoint0 (&pThis->m_USBDevice), 0x02, 1, 0, 2, 0, 0) < 0)
	{
		LogWrite (FromUmsd, LOG_DEBUG, "Cannot clear halt on endpoint 2");

		return -1;
	}

	USBEndpointResetPID (pThis->m_pEndpointIn);
	USBEndpointResetPID (pThis->m_pEndpointOut);

	return 0;
}
