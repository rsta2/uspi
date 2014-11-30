//
// main.c
//
#include <uspienv.h>
#include <uspi.h>
#include <uspios.h>
#include <uspienv/assert.h>

static const char FromSample[] = "sample";

static void GamePadStatusHandler (const USPiGamePadState *pState)
{
	LogWrite (FromSample, LogNotice, "buttons 0x%X x %d y %d z %d rx %d ry %d rz %d",
		  pState->buttons,
		  pState->x, pState->y, pState->z,
		  pState->rx, pState->ry, pState->rz);
}

int main (void)
{
	if (!USPiEnvInitialize ())
	{
		return EXIT_HALT;
	}
	
	if (!USPiInitialize ())
	{
		LogWrite (FromSample, LOG_ERROR, "Cannot initialize USPi");

		USPiEnvClose ();

		return EXIT_HALT;
	}
	
	if (!USPiGamePadAvailable ())
	{
		LogWrite (FromSample, LOG_ERROR, "Gamepad not found");

		USPiEnvClose ();

		return EXIT_HALT;
	}

	const USPiGamePadState *pState = USPiGamePadGetStatus ();
	assert (pState != 0);

	LogWrite (FromSample, LogNotice, "Gamepad: Vendor 0x%X Product 0x%X Version 0x%X",
		  (unsigned) pState->idVendor, (unsigned) pState->idProduct, (unsigned) pState->idVersion);

	LogWrite (FromSample, LogNotice, "Buttons: Count %d", pState->nbuttons);

	LogWrite (FromSample, LogNotice, "Axis: Flags 0x%X Minimum %d Maximum %d",
		  pState->flags, pState->minimum, pState->maximum);

	USPiGamePadRegisterStatusHandler (GamePadStatusHandler);

	// just wait and turn the rotor
	for (unsigned nCount = 0; 1; nCount++)
	{
		ScreenDeviceRotor (USPiEnvGetScreen (), 0, nCount);
	}

	return EXIT_HALT;
}
