//
// main.c
//
#include <uspienv.h>
#include <uspi.h>
#include <uspios.h>
#include <uspienv/util.h>

static const char FromSample[] = "sample";

static void MouseStatusHandler (unsigned nButtons, int nDisplacementX, int nDisplacementY)
{
	LogWrite (FromSample, LOG_NOTICE, "Buttons %c%c%c, X %d, Y %d",
		  nButtons & MOUSE_BUTTON1 ? 'L' : '-',
		  nButtons & MOUSE_BUTTON3 ? 'M' : '-',
		  nButtons & MOUSE_BUTTON2 ? 'R' : '-',
		  nDisplacementX, nDisplacementY);
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
	
	if (!USPiMouseAvailable ())
	{
		LogWrite (FromSample, LOG_ERROR, "Mouse not found");

		USPiEnvClose ();

		return EXIT_HALT;
	}

	USPiMouseRegisterStatusHandler (MouseStatusHandler);

	LogWrite (FromSample, LOG_NOTICE, "Move your mouse!");

	// just wait and turn the rotor
	for (unsigned nCount = 0; 1; nCount++)
	{
		ScreenDeviceRotor (USPiEnvGetScreen (), 0, nCount);
	}

	return EXIT_HALT;
}
