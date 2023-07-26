#!/bin/bash
echo "Interactive script to hack the firmware of your keychain photo"
echo "player."
if [ ! -e "$1" ]; then
    echo "Usage: $0 /dev/sdX"
    echo "where /dev/sdX is the path to the device your photo-unit is connected to."
    echo "Make sure the device is in upload-mode!"
    exit 0;
fi

if [ ! -e "phack" -o ! -e "splice" ]; then
    echo "Please run 'make' first to compile the tools."
    exit 0;
fi

echo
echo "Ok, first off all, we're going to backup the firmware and memory of your"
echo "device to fwimage.bak and memimage.bak."
./phack -m "baks r ok" $1 > /dev/null
./phack -df fwimage.bak $1  > /dev/null || exit 1 
./phack -d memimage.bak $1  > /dev/null || exit 1

echo "Making a working copy..."
cp fwimage.bak fwimage.bin
echo "Checking requirements..."
./phack -m "is fw ok?" $1 > /dev/null
dd if=fwimage.bin bs=256 skip=58 count=2 of=fwbit 2>/dev/null
#check for all FFs Md5sum may not be _the_ tool for that, but it works OK.
if ! md5sum fwbit | grep -q de03fe65a6765caa8c91343acc62cffc; then
    echo "No room at the location we want to place the hack!"
    echo "This specific hack won't work for this particular firmware, I'm sorry."
    exit 1;        
fi


off=`fgrep -f hack/lookforme.bin fwimage.bak -a -b -o | cut -d ':' -f 1`
if [ "$off" -lt 1000 ]; then
    echo "Hmmm, a certain routine that's required for this hack seems missing :/"
    echo "This specific hack won't work for this particular firmware, I'm sorry."
    exit 1;
fi
echo 
if [ "$off" = 11741 ]; then
    echo "Good, you seem to have the same firmware as the author."
else
    echo "You seem to have a firmware that has the same properties as the authors."
    echo "Continuing is possible, but may pose a risk. Don't continue if you absolutely"
    echo "can't miss your device!"
fi

echo "Requirements OK, we can try to hack the device. Proceed? (yes/no)"
./phack -m "Yay! \\o/" $1 > /dev/null
read yn
if ! [ "$yn" = "yes" ]; then
    echo "No 'yes' received. OK, bailing."
    ./phack -m "Kbyetnx." $1 > /dev/null
    exit 0;
fi
echo "Patching fw..."
echo $off
./splice fwimage.bin hack/hack_jmp.bin "$off" >/dev/null || exit 1
./splice fwimage.bin hack/hack.bin 0x3A00 >/dev/null || exit 1
echo "Uploading fw"
./phack -m "Eeeek!" $1 > /dev/null
./phack -uf fwimage.bin $1
echo
echo
echo "All done. To test, disconnect the device and when it has rebooted, connect"
echo "it again, go into 'update mode' and press enter. To quit, use ctrl-c."
read
echo "Ok, just a sec..."
sleep 5
./phack -l test.png
