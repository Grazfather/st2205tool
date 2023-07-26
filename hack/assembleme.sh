

#Needed: 
# - crasm. Is in debian repositories.
# - dd. Is everywhere :)
# - objcopy. Prolly comes with your c compiler.

rm hack.hex
crasm -o hack.hex hack.asm
if [ ! -e hack.hex ]; then exit 1; fi
objcopy -I ihex -O binary hack.hex hack.bin

rm hack_jmp.hex
crasm -o hack_jmp.hex hack_jmp.asm
if [ ! -e hack_jmp.hex ]; then exit 1; fi
objcopy -I ihex -O binary hack_jmp.hex hack_jmp.bin
