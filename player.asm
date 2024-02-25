LINL32: equ $f3af
INIT32: equ $006f
CHPUT: equ $00a2
CURRENTPAGE: equ $c000
ASCII8BANK1: equ $6800

SFG_YM2151_STATUS: equ $3FF1
SFG_YM2151_ADDRESS: equ $3FF0
SFG_YM2151_DATA: equ $3FF1
SFG_CLOCK: equ 3579545

org 4000h
db "AB"
dw Main
db 00,00,00,00,00,00

Main:
    ld a,32
    ld (LINL32),a
    call INIT32
    ld hl,SampleText
    call Print
Restart:
    ld a,1
    ld (CURRENTPAGE),a
    ld (ASCII8BANK1),a
    ld hl,Data

    jp Loop

Test2:
    ld a,$80
    cp $80
    jp c,Test2 ; b >= 8
    ld hl,NonZeroText
    call Print
Die2:
    nop
    jp Die2

Test:
    ld b,8
    ld a,7
    cp b
    jp c,Test ; b >= 8
    ld hl,TestText
    call Print
Die:
    nop
    jp Die


Loop:
    ld b,(hl)
    inc hl
    ld c,(hl)
    inc hl
    ld a,7
    cp b
    jp c,Write_Register

    ld a,b
    or c
    jp z,Restart

;bc is length of wait
Player_Wait:
    ld de,$1 ; arbitrary wait unit length
Player_Wait_Inner:
    nop
    nop
    nop
    dec de
    ld a,d
    or e
    jp nz,Player_Wait_Inner
    dec bc
    ld a,b
    or c
    jp nz,Player_Wait
    jp EndLoop
    
Write_Register:
    ; aa 8  8 dd 0  0
    di
    ld a,%00010111
    out ($a8),a
    ld a,b
    ld (SFG_YM2151_ADDRESS),a
    cp (hl)  ; R800 wait: ~4 bus cycles
    ld a,c
    ld (SFG_YM2151_DATA),a
    cp (hl)  ; R800 wait: ~4 bus cycles
    ld a,%00010100
    out ($a8),a
    ei

EndLoop:
    ld a,h
    cp $80
    jp c,Loop
    ld hl,$6000
    ld a,(CURRENTPAGE)
    add a,1
    ld (CURRENTPAGE),a
    ld (ASCII8BANK1),a
    jp Loop

Print:
    ld a,(hl)
    and a
    ret z
    call CHPUT
    inc hl
    jr Print

SampleText:
    db "Hello world",0

TestText:
    db "success",0

NonZeroText:
    db "non zero",0

LessThanText:
    db "$79 less than $80",0

Data:
include 'data.asm'

