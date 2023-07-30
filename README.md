# ST2205 Tool

This is a mirror of Jeroen Domburg et. al's ST2205 Tool, used to hack the
firmware of a bunch of ST2205-based LCD keychains with custom firmware,
allowing a user to send pixels to it directly. It has been largely abandoned,
so it mostly here for posterity, but I will probably push various small
improvements and bug fixes, which I've used to get it to work on a modern
machine.

The original blog post can be found
[here](https://spritesmods.com/?art=picframe), and most versions of the
firmware can be found [here](https://spritesmods.com/picframe/), though I've
tagged each version in this repo.

## How to use

- Use make to compile the binaries. You'll probably need the development
  headers and libraries of libgd for this, preferably v2.
- Install libraries: run 'make install' as root
- connect your device and observe (e.g. by running dmesg) the device node
  it's connected to. Look for e.g.:
  SCSI device sda: 4096 512-byte hdwr sectors (2 MB)
  Replace the 'sdX' in the next instruction with the devicename mentioned.
  (in this example: sda)
- If your device already is hacked with an earlier version of the firmware,
  use the backup of the original firmware as an argument to the fwhack.sh
  script: run './hackfw /dev/sdx /path/to/backup/of/original/fwimage.bak' If
  your device is unhacked as of yet, just run './hackfw /dev/sdX'.
- Run the hackfw.sh program to patch the firmware. It'll backup your firmware
  and do a check on it to see it's patchable. If that's the case and you agree,
  it'll patch it and upload the new firmware.
- If it works, you should be looking at 'It Works' displayed on your LCD.

A patched unit still has the same capabilities as an unpatched one. The only
difference is that, in upgrade mode, you can send pictures to the LCD via
the USB-port, which enables stuff like the displaying of real-time server
statistics.

This patch will work on devices with the ST220XU-chip (which is detected by the
hackfw.sh-program) and have a PCF8833-compatible display. If your device has
a 1.4" or 1.5" color CSTN-screen with a resolution of 128x128 pixels, it'll
probably work.

DISCLAIMER: Use at your own risk. I'm not responsible for any damage that occurs
by running my programs or following my instructions.


## FAQ:

Q: I have installed everything by doing a 'make install' as root, but programs
   still can't find libst2205.so.1! What am I doing wrong?  
A: Libst2205.so is installed into /usr/local/lib. Make sure your
   /etc/ld.so.conf contains the line '/usr/local/lib' and then run (as root)
   ldconfig.


## Changelog

Most changes can be found on the
[wiki](http://picframe.spritesserver.nl/wiki/Index.php), I just pasted them
together - michu@neophob.com

### v1.4.3
- libst2205.c: fix segfault introduced in v1.4.2, fails on an unhacked keychain
  (thanks Sean Burford)
- add a note to Coby DP 151 V8 hack-spec file, use another offset x/y position
  (thanks Sean Burford)
- removed debug information
- removed unneeded main.c file in libst2205 directory

### v1.4.2
- slice.c: fix compiler warning
- libst2205.c: Fix error handling if param block not found
- libst2205.c: Minor fixes
- add another innovage keychain, support now 17 modules
- main.c: fix display help

### v1.4.1
- applied Metan's patch, src:
  http://picframe.spritesserver.nl/wiki/index.php/Phackv1.4
  - segfault on ppc (empty return in function returning int)
  - compilation fails because of missing -fPIC on x86_64
  - coding style formated according to linux kernel style
  - + various small fixups
- supports now 16 st2205 modules, src:
  http://picframe.spritesserver.nl/wiki/index.php/Devices
- fixed hack.sh, backup fw
- fixed libst2205, crashes on Xscale/ARM CPU's, function enddata
- removed lcd4linux patch as the driver is upstream
