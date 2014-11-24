USPi
====

> If you read this file in an editor you should switch line wrapping on.

Overview
--------

USPi is a bare metal USB driver for the Raspberry Pi written in C. It was ported from the Circle USB library. Using C allows it to be used from bare metal C code for the Raspberry Pi. Like the Circle USB library it supports control (synchronous), bulk and interrupt (synchronous and asynchronous) transfers. Function drivers are available for USB keyboards, mice, gamepads, mass storage devices (e.g. USB flash devices) and the on-board Ethernet controller. USPi should run on all existing Raspberry Pi models.

USPi was "mechanically" ported from the Circle USB library which is written in C++. That's why the source code may look a little bit strange. But it was faster to do so.

Interface
---------

The USPi library provides functions to be used by the bare metal environment to access USB devices. There are six groups of functions which are declared in *include/uspi.h*:

* USPi initialization
* Keyboard
* Mouse
* GamePad
* USB Mass storage device
* Ethernet controller

The bare metal environment has to provide some functions to the USPi library which are declared in *include/uspios.h*. There are the six groups of functions:

* Memory allocation
* Timer
* Interrupt handling
* Property tags
* Logging
* Debug support

Configuration
-------------

Before you build the USPi library you have to configure it to meet your system configuration in the file *include/uspios.h* (top section).

Another option (NDEBUG) can be defined in Rules.mk to build the release version. In the test phase it is better to use the debug version which contains many additional checks.

Building
--------

Building is normally done on PC Linux. You need a [toolchain](http://elinux.org/Rpi_Software#ARM) for the ARM1176JZF core. You can also build and use USPi on the Raspberry Pi itself on Debian wheezy.

First edit the file *Rules.mk* and set the *PREFIX* of your toolchain commands.

Then go to the lib/ directory of USPi and do:

`make clean`  
`make`

The ready build *libuspi.a* file should be in the lib/ directory.

Using
-----

Add the USPi include/ directory to the include path in your Makefile and specify the *libuspi.a* library to *ld*. Include the file *uspi.h* where you want to access USPi functions.
