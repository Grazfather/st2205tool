    CPU 65c02
    OUTPUT HEX
    * = $6bbb

;This gets patched into the existing usb routines.
    jmp $07A00
    nop
