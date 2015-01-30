# cc3100-linux
Patch to run wlan module TI cc3100boost under linux.

To get started:
- download the cc3100sdk from ti: http://www.ti.com/tool/cc3100sdk

  - install (extract) the sdk:
    `wine CC3100SDK-1.0.0-windows-installer.exe`

  - go to the folder where you installed the sdk cc3100-sdk/ and clone the git repository in this folder!
	    `git clone ....`

  - to modify the example applications:
      `patch ...`

Three sample applications from TI were tested and can be compiled:

  - `make CROSS_COMPILE=<pathToYourCrossCompiler> debug`
    - http://processors.wiki.ti.com/index.php/CC31xx_SPI_Debug_Tool

  - `make CROSS_COMPILE=<pathToYourCrossCompiler> tcp`
    - http://processors.wiki.ti.com/index.php/CC31xx_TCP_Socket_Application

  - `make CROSS_COMPILE=<pathToYourCrossCompiler> wlanap` 
    - http://processors.wiki.ti.com/index.php/CC31xx_Getting_Started_with_WLAN_AP


- The driver was tested on the following platforms:
  - Raspberry Pi type B, Raspian (build 2014-09-09) Kernel 3.16.5+
    - Raspian image: http://downloads.raspberrypi.org/raspbian_latest
    - kernel source: https://github.com/raspberrypi/linux
  - Beaglebone (white) Rev. 3, Debian 7.5 (build 2014-05-14) Kernel 3.14.22
    - Debian image: http://elinux.org/BeagleBoardDebian#BeagleBone.2FBeagleBone_Black
    - kernel source: https://github.com/beagleboard/linux

Note: To execute these applications the kernel module cc3100-spi has to be loaded successfully. For further information read the file ./cc3100-spi-kernelmodul/README!
