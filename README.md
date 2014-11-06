USPi
====

> If you read this file in an editor you should switch line wrapping on.

Overview
--------

USPi is a bare metal USB driver for the Raspberry Pi written in C. In fact it is (still) ported from the Circle USB library. Using C allows it to be used from bare metal C code for the Raspberry Pi. Like the Circle USB library it will support synchronous and asynchronous control, bulk and interrupt transfers. Function drivers will be available for USB keyboards, mass storage devices (e.g. USB flash devices) and the on-board Ethernet controller. USPi is only running on the Raspberry Pi model B and B+ at the moment.

USPi was "mechanically" ported from the Circle USB library which is written in C++. That's why the source code may look a little bit strange. But it was faster to do so.

Status
------

USPi is still under development. Currently it can be used to send and receive frames to/from the Ethernet.

Interface
---------

The functions provided by the USPi library are declared in *include/uspi.h*.

The functions to be provided by the bare metal environment to be used by the USPi library are declared in *include/uspios.h*.
