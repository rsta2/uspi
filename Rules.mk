#
# Rules.mk
#
# USPi - An USB driver for Raspberry Pi written in C
# Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

ifeq ($(strip $(USPIHOME)),)
USPIHOME = ..
endif

-include $(USPIHOME)/Config.mk

RASPPI	?= 1
PREFIX	?= arm-none-eabi-

CC	= $(PREFIX)gcc
AS	= $(PREFIX)gcc
LD	= $(PREFIX)gcc
AR	= $(PREFIX)gcc-ar

ifeq ($(strip $(RASPPI)),1)
ARCH	?= -march=armv6j -mtune=arm1176jzf-s -mfloat-abi=hard
else ifeq ($(strip $(RASPPI)),2)
ARCH	?= -march=armv7-a -mtune=cortex-a7 -mfloat-abi=hard
else
ARCH	?= -march=armv8-a -mtune=cortex-a53 -mfloat-abi=hard
endif

OPTIMIZE ?= -Ofast -flto -ffunction-sections -fdata-sections

AFLAGS	+= $(ARCH) -DRASPPI=$(RASPPI)
CPPFLAGS+= #-DNDEBUG
CFLAGS	+= $(ARCH) -Wall -Wno-psabi -fsigned-char -fno-builtin -nostdinc -nostdlib \
	   -std=gnu99 -undef -DRASPPI=$(RASPPI) -I $(USPIHOME)/include $(OPTIMIZE)
LDFLAGS	+= -nodefaultlibs -nostartfiles -Wl,-gc-sections -flto -Wl,-Map,kernel.map \
	   -Wl,-T,$(USPIHOME)/env/uspienv.ld $(USPIHOME)/env/lib/startup.o
LIBS	+= -lgcc

%.o: %.S
	$(AS) $(CPPFLAGS) $(AFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.a *.elf *.lst *.img *.cir *.map *~ $(EXTRACLEAN)
