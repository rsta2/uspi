#
# Rules.mk
#
# The C build process was initially taken
#	from the "collection of low level examples"
# 	which is Copyright (c) 2012 David Welch dwelch@dwelch.com
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
USPIHOME = ../..
endif

CFLAGS	+= -I $(USPIHOME)/env/include

include $(USPIHOME)/Rules.mk

ifneq ($(strip $(AARCH64)),0)
$(error AARCH64 is not supported in sample/)
endif
