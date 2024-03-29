;-----------------------------------------------------------------------		
;
; ATR2CAR starter for SWITCHABLE XEGS CARTRIDGE with 256 sectors
; (c) 2022 GienekP
;
;-----------------------------------------------------------------------

RAMPROC = $0100

;-----------------------------------------------------------------------

TMP     = $A0

;-----------------------------------------------------------------------

CRITIC  = $42
RAMTOP  = $6A

DMACTLS = $022F
PDVMSK  = $0247
MEMTOP  = $02E5
DVSTAT  = $02EA
DDEVIC  = $0300
DUNIT   = $0301
DCMND   = $0302
DSTATS  = $0303
DBUFA   = $0304
DBYT    = $0308
DAUX1	= $030A
DAUX2	= $030B
BASICF  = $03F8
GINTLK  = $03FA

TRIG3   = $D013
IRQEN   = $D20E
IRQST   = $D20E
PORTB   = $D301
DMACTL  = $D400
VCOUNT  = $D40B
NMIEN   = $D40E

WAIT	= $C0DF
RESETWM = $C290
RESETCD = $C2C8
BOOT    = $C58B
JSIOINT = $E459
SIO     = $E971

;-----------------------------------------------------------------------		
; SWITCHABLE XEGS CARTRIDGE

		OPT h-f+
		
		ORG $BC00

;-----------------------------------------------------------------------		
; INITCART ROUTINE

INIT	rts
	
;-----------------------------------------------------------------------		
; CARTRUN ROUTINE
	
BEGIN	jsr IRQDIS
		jsr ROM2RAM
		jsr SETRAM
		jsr OVRDINT
		jsr IRQENB
		jsr RESERVE
		jsr FINAL
		jmp BYEBYE
	
;-----------------------------------------------------------------------		
; IRQ ENABLE

IRQENB	lda #$40
		sta NMIEN
		lda #$F7
		sta IRQST
		lda DMACTLS
		sta DMACTL
		cli
		rts

;-----------------------------------------------------------------------		
; IRQ DISABLE

IRQDIS	sei	
		lda #$00
		sta DMACTL
		sta NMIEN
		sta IRQEN
		sta IRQST
		rts
		
;-----------------------------------------------------------------------		
; COPY ROM TO RAM
	
ROM2RAM	lda #$C0
		sta TMP+1
		ldy #$00
		sty TMP
		ldx #$FF
				
CPOS	stx PORTB
		lda (TMP),Y
		dex
		stx PORTB
		sta (TMP),Y
		inx
		iny
		bne CPOS
		inc TMP+1
		lda TMP+1
		cmp #$D0
		bne OSOK
		lda #$D8
		sta TMP+1
OSOK	cmp #$00
		bne CPOS
		rts
		
;-----------------------------------------------------------------------		
; SET RAM & DISABLE BASIC

SETRAM	lda PORTB
		and #$FE
		ora #$02
		sta PORTB
		lda #$01
		sta BASICF
		rts
		
;-----------------------------------------------------------------------		
; COPY NEW SIOINT PROCEDURE

OVRDINT	lda #<SIOCPY
		sta TMP
		lda #>SIOCPY
		sta TMP+1
		lda JSIOINT+1
		sta TMP+2
		lda JSIOINT+2
		sta TMP+3
			
		ldy #ENDCPY-SIOCPY-1
LPCPY	lda (TMP),Y
		sta (TMP+2),Y
		dey
		bne LPCPY
		lda (TMP),Y
		sta (TMP+2),Y
		
		lda RESETWM+2
		sta RESETWM+5
		lda RESETWM+3
		sta RESETWM+6
		
		lda WAIT+69
		sta WAIT+72
		lda WAIT+70
		sta WAIT+73	
		
		rts
		
;-----------------------------------------------------------------------		
; COPY TO $RAMPROC FOR "KILLERS" PORTB

RESERVE	lda #<ZEROCP
		sta TMP
		lda #>ZEROCP
		sta TMP+1
		lda #<RAMPROC
		sta TMP+2
		lda #>RAMPROC
		sta TMP+3		
		ldy #ZEROEND-ZEROCP-1
RESCPY	lda (TMP),Y
		sta (TMP+2),Y
		dey
		bne RESCPY
		lda (TMP),Y
		sta (TMP+2),Y
		rts

;-----------------------------------------------------------------------		
; FINAL VALUES

FINAL 	lda #$1F
		sta MEMTOP
		lda #$BC
		sta MEMTOP+1
		lda #$C0
		sta RAMTOP
VCL1	lda VCOUNT
		cmp #$3
		bne VCL1
TSTMAX	lda VCOUNT
		cmp #$8A
		bne VCL2
		lda #$77
		sta RAMPROC+LICNT-ZEROCP+1
VCL2	cmp #$00
		bne TSTMAX
		lda #$80
		sta TMP+1
		lda #$00
		sta TMP
		tay
		rts
		
;-----------------------------------------------------------------------		
; LEAVE CART SPACE
		
BYEBYE	jmp RAMPROC+GOBOOT-ZEROCP

;-----------------------------------------------------------------------		
; SIO INTerface

SIOCPY
.local SIOINT,$C933

CRITIC  = $42
DSTATS  = $0303
DUNIT   = $0301
GETLOW  = $C9AF
PDIOR   = $D805
PDVMSK  = $0247
PDVREG  = $D1FF
PDVRS   = $0248
SIO     = $E971

		lda #$01
		jmp RAMPROC		;sta CRITIC
		lda DUNIT
		pha
		lda PDVMSK
		beq FOUND
		ldx #$08
NEXT 	jsr GETLOW
		beq FOUND
		txa
		pha
		jsr PDIOR
		pla
		tax
		bcc NEXT
		lda #$00
		sta PDVRS
		sta PDVREG
		beq END
FOUND 	jsr SIO
END 	pla
		sta DUNIT
		lda #$00
		sta CRITIC
		sty DSTATS
		ldy DSTATS
		rts
		
.end
ENDCPY	; --->>> $C96D

;-----------------------------------------------------------------------		
; RELOC CODE FOR RAMPROC

ZEROCP	lda VCOUNT
LICNT	cmp #$52		; $52->NTSC $77->PAL
		bne ZEROCP		
		ldy #$7F
		sty $D500
		jmp AROUND
		
		; --->>>CART<<<---

CPYSEC	lda #$FF
		sta $D500
		lda (TMP),Y
		stx $D500
		sta (TMP+2),Y
MODE	dey
		bpl CPYSEC
		
		ldy DSTATS	
				
BACK	lda #$FF
		sta $D500
		lda TRIG3
		sta GINTLK
		rts
		
GOSIO	jsr RAMPROC+BACK-ZEROCP
		jmp $C95B
		
GOBOOT	jsr RAMPROC+BACK-ZEROCP	
		tya
CLRM	sta (TMP),Y
		iny
		bne CLRM
		inc TMP+1
		ldx TMP+1
		cpx #$C0
		bne CLRM
		jsr BOOT
		jmp RESETWM
		
ZEROEND

;-----------------------------------------------------------------------		
; AROUND SIO INTerface

AROUND  lda DDEVIC
		cmp #$00
		bne D0
		lda #$31
		sta DDEVIC	
D0		and #$F0
		clc
		adc DUNIT
		cmp #$31
		beq D1
		lda #$01
		sta CRITIC
		lda DUNIT
		pha
		jmp RAMPROC+GOSIO-ZEROCP					
D1		lda #$00
		sta CRITIC
		lda DCMND
		cmp #$52
		beq SECREAD
		cmp #$57
		beq STATOK
		cmp #$50
		beq STATOK
		cmp #$53
		bne UNKWCMD		
		lda #<DVSTAT
		sta DBUFA
		lda #>DVSTAT
		sta DBUFA+1
		ldx #$03
CPSTAT	lda D1STAT,x
		sta DVSTAT,x
		dex
		bpl CPSTAT					
STATOK	ldy #$01
		sty DSTATS
UNKWCMD	jmp RAMPROC+BACK-ZEROCP
SECREAD	lda #$01
		sta DSTATS
		lda DBYT
		cmp #$80
		beq SEC80	
		ldy #$00
		lda #$C8	; INY
		sta RAMPROC+MODE-ZEROCP
		lda #$D0	; BNE
		sta RAMPROC+MODE-ZEROCP+1
		clc
		bcc CALCSEC		
SEC80	ldy #$7F
		lda #$88	; DEY
		sta RAMPROC+MODE-ZEROCP
		lda #$10	; BPL
		sta RAMPROC+MODE-ZEROCP+1	
CALCSEC lda #$00
		sta TMP
		lda DAUX1
		and #$1F
		clc
		adc #$80
		sta TMP+1			
		lda DAUX2
		asl
		asl
		asl
		and #$7F
		sta TMP+4
		lda DAUX1
		lsr
		lsr
		lsr
		lsr
		lsr
		ora TMP+4
		sta TMP+4			
		lda DBUFA
		sta TMP+2
		lda DBUFA+1
		sta TMP+3	
		ldx #$FF
		lda TMP+4
		sta RAMPROC+CPYSEC-ZEROCP+1	; replace $010F LDA #$FF
		jmp RAMPROC+CPYSEC-ZEROCP
		
D1STAT  dta $38,$ff,$01,$00

;-----------------------------------------------------------------------		

		ORG $BFFA
		dta <BEGIN, >BEGIN, $00, $04, <INIT, >INIT

;-----------------------------------------------------------------------		
