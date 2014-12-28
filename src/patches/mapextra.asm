arch nes.cpu
header
banksize $2000

//; ---------------------------------------------------------------------------
//; Move extra level data (wind speed, miniboss screen lock/door close) to
//; unused space in the regular room data header and read it from there
//; instead.
//;
//; This frees up the space which was originally used for this data's table.
//; That space will be used for other code, like the switch fix (see below).
//; ---------------------------------------------------------------------------
define MapExtraData  {MapHeader}+$18
define MapWindType   {MapExtraData}+0

//; this value is 1 higher than what the game uses
//; (so that 0 = no extra data stored here)
define MapBossCount  {MapExtraData}+1

define MapDoorX      {MapExtraData}+2
define MapDoorY      {MapExtraData}+3
define MapDoorTop    {MapExtraData}+4
define MapDoorBtm    {MapExtraData}+5

//; if MapDoorX == $FF, use these for screen lock instead
define MapLockPosL   {MapExtraData}+3
define MapLockPosH   {MapExtraData}+4

define MidbossCount  $78c8
define ScrollControl $78c9
define LockScroll    $02
define LockPosL      $78ca
define LockPosH      $78cb

define DoorTopTile   $78ca
define DoorBtmTile   $78cb
define DoorX         $78cc
define DoorXScreen   $78cd
define DoorY         $78ce
define DoorYScreen   $78cf

define WindType      $78d0

define DoorXTemp     $4b
define DoorYTemp     $4c

//; This code is at the same location in all versions
bank $38
org $ad6f
	lda   {MapWindType}
	//; game uses 0xFF for no wind, KALE uses zero (for convenience)
	sec
	sbc   #$01
	sta   {WindType}
	
	//; midboss stuff?
	lda   {MapBossCount}
	bne   +
	rts
	
+
	sec
	sbc   #$01
	sta   {MidbossCount}
	
	lda   #0
	sta   {DoorXTemp}
	sta   {DoorYTemp}
	
	lda   {MapDoorX}
	cmp   #$ff
	beq   LockScreen
	
	//; set up locked door for midboss
	
	//; get X position of door
	asl
	rol   {DoorXTemp}
	asl
	rol   {DoorXTemp}
	asl
	rol   {DoorXTemp}
	asl
	rol   {DoorXTemp}
	sta   {DoorX}
	lda   {DoorXTemp}
	sta   {DoorXScreen}
	
	//; get Y position of door
	lda   {MapDoorY}
	asl
	rol   {DoorYTemp}
	asl
	rol   {DoorYTemp}
	asl
	rol   {DoorYTemp}
	asl
	rol   {DoorYTemp}
	sta   {DoorY}
	lda   {DoorYTemp}
	sta   {DoorYScreen}
	
	//; get replacement door tiles
	lda   {MapDoorBtm}
	pha
	lda   {MapDoorTop}
	pha
	ldx   {DoorX}
	jsr   $ef90
	ldy   $10
	lda   ($16),y
	sta   {DoorTopTile}
	pla
	sta   ($16),y
	ldx   {DoorX}
	lda   {DoorY}
	clc
	adc   #$10
	tay
	jsr   $ef90
	ldy   $10
	lda   ($16),y
	sta   {DoorBtmTile}
	pla
	sta   ($16),y
	lda   #$01
	sta   {ScrollControl}
	sta   $78d1 //; ???
	rts
	
LockScreen:
	//; lock the screen for a midboss instead of covering up a door
	lda   #{LockScroll}
	sta   {ScrollControl}
	lda   {MapLockPosL}
	sta   {LockPosL}
	lda   {MapLockPosH}
	sta   {LockPosH}
	rts
	
//; TODO: a warnpc here

//; ---------------------------------------------------------------------------
//; Replace the obsolete extra data table with some new code
//; ---------------------------------------------------------------------------

//; Insert jumps to new code here
bank $28
org {SwitchOld}
	lda   {Bank8Num}
	pha
	lda   #$12
	jsr   {SwapBank8}
	jsr   SwitchFix
	pla
	jmp   {SwapBank8}

//; as with the code that references it, this data table is at the same location
//; in every version of the game
bank $12
org $9e92

//; ---------------------------------------------------------------------------
//; Modify the star switch position check to allow it to be placed on any 
//; screen
//; ---------------------------------------------------------------------------

//; signify that these hacks have been added
db "KALE"

define SwitchTemp $00
SwitchFix:
	//; initial destination = horizontal screen #
	ldx   {PlayerXHi}
	
	//; get the upper 8 bits of 12-bit Y position
	lda   {PlayerYHi} //; upper 4 bits
	sta   {SwitchTemp}
	lda   {PlayerYLo} //; lower 8 bits
	lsr   {SwitchTemp}
	ror
	lsr   {SwitchTemp}
	ror
	lsr   {SwitchTemp}
	ror
	lsr   {SwitchTemp}
	ror
	
	//; if Y >= 12 (i.e. more than one screen deep) then add the level width to
	//; the destination screen
-
	cmp   #12
	bcc   +
	sec
	sbc   #12
	pha
	txa
	clc
	adc   {MapWidth}
	tax
	pla
	bne   -

+
	stx   {ExitToScreen}
	sta   {SwitchTemp}
	
	//; get 4+4 bit X/Y coord based on the middle 4 bits of the player's X pos
	lda   {PlayerXLo}
	and   #$f0
	ora   {SwitchTemp}
	sta   {ExitToPos}
	
	rts

//; will this actually work? it's right on a bank boundary so xkas might get confused
warnpc $a000