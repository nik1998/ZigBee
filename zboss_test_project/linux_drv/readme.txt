Here is driver for uz2400 interrupt for Linux/ARM and files to tune it.
The idea is to use spidev driver to access SPI port from the userspace using ioctl
and get info about UZ2400 interrupt by sleeping in select() using zigbee_intr driver.
All that stuff is to be placed into arch/arm/mach-mx3 directory of Linux code tree.

|to compile enable SPIDEV and ZIGBEE_INTR (UZ24000) drivers.
