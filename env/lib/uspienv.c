//
// uspienv.c
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
#include <uspienv.h>
#include <uspienv/bcmpropertytags.h>
#include <uspienv/alloc.h>
#include <uspienv/assert.h>

static TUSPiEnv *s_pEnv = 0;

int USPiEnvInitialize (void)
{
	TBcmPropertyTags Tags;
	BcmPropertyTags (&Tags);
	TPropertyTagMemory TagMemory;
	if (!BcmPropertyTagsGetTag (&Tags, PROPTAG_GET_ARM_MEMORY, &TagMemory, sizeof TagMemory, 0))
	{
		TagMemory.nBaseAddress = 0;
		TagMemory.nSize = ARM_MEM_SIZE;
	}

	mem_init (TagMemory.nBaseAddress, TagMemory.nSize);

	s_pEnv = (TUSPiEnv *) malloc (sizeof (TUSPiEnv));
	if (s_pEnv == 0)
	{
		_BcmPropertyTags (&Tags);

		return 0;
	}

	ScreenDevice (&s_pEnv->m_Screen, 0, 0);
	if (!ScreenDeviceInitialize (&s_pEnv->m_Screen))
	{
		_ScreenDevice (&s_pEnv->m_Screen);
		_BcmPropertyTags (&Tags);

		free (s_pEnv);
		s_pEnv = 0;

		return 0;
	}

	InterruptSystem (&s_pEnv->m_Interrupt);
	Timer (&s_pEnv->m_Timer, &s_pEnv->m_Interrupt);
	Logger (&s_pEnv->m_Logger, LogDebug, &s_pEnv->m_Timer);

	if (   !LoggerInitialize (&s_pEnv->m_Logger, &s_pEnv->m_Screen)
	    || !InterruptSystemInitialize (&s_pEnv->m_Interrupt)
	    || !TimerInitialize (&s_pEnv->m_Timer))
	{
		_Logger (&s_pEnv->m_Logger);
		_Timer (&s_pEnv->m_Timer);
		_InterruptSystem (&s_pEnv->m_Interrupt);
		_ScreenDevice (&s_pEnv->m_Screen);
		_BcmPropertyTags (&Tags);

		free (s_pEnv);
		s_pEnv = 0;

		return 0;
	}

	return 1;
}

void USPiEnvClose (void)
{
	_Logger (&s_pEnv->m_Logger);
	_Timer (&s_pEnv->m_Timer);
	_InterruptSystem (&s_pEnv->m_Interrupt);
	_ScreenDevice (&s_pEnv->m_Screen);

	free (s_pEnv);
	s_pEnv = 0;
}

TScreenDevice *USPiEnvGetScreen (void)
{
	assert (s_pEnv != 0);
	return &s_pEnv->m_Screen;
}
