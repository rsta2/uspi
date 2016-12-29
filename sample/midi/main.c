//
// main.c
//
#include <uspienv.h>
#include <uspi.h>
#include <uspios.h>
#include <uspienv/util.h>

static const char FromSample[] = "sample";

#define MIDI_NOTE_OFF	0b1000
#define MIDI_NOTE_ON	0b1001

static void PacketHandler (unsigned nCable, unsigned nLength, u8 *pPacket)
{
	// The packet contents are just normal MIDI data - see
	// https://www.midi.org/specifications/item/table-1-summary-of-midi-message

	u8 ucStatus = pPacket[0];
	u8 ucChannel = ucStatus & 0xf;
	u8 ucType = ucStatus >> 4;

	switch (ucType)
	{
	case MIDI_NOTE_OFF:
	case MIDI_NOTE_ON:
	{
		u8 ucKey = pPacket[1], ucVelocity = pPacket[2];
		LogWrite(FromSample,
				 LOG_NOTICE,
				 "Note %u %s! (velocity %u, cable %u, channel %u)",
				 ucKey,
				 ucType == MIDI_NOTE_OFF ? "off" : "on",
				 ucVelocity,
				 nCable,
				 ucChannel);
		break;
	}
	default:
		LogWrite(FromSample, LOG_NOTICE, "Other MIDI message type! (cable %u, channel %u)", nCable, ucChannel);
		break;
	}
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

	if (!USPiMIDIAvailable ())
	{
		LogWrite (FromSample, LOG_ERROR, "MIDI device not found");

		USPiEnvClose ();

		return EXIT_HALT;
	}

	USPiMIDIRegisterPacketHandler (PacketHandler);

	LogWrite (FromSample, LOG_NOTICE, "Play!");

	// just wait and turn the rotor
	for (unsigned nCount = 0; 1; nCount++)
	{
		ScreenDeviceRotor (USPiEnvGetScreen (), 0, nCount);
	}

	return EXIT_HALT;
}
