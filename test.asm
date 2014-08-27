		CPU 6502
;==============================================================================
;	 Advanced Coding Examples for the Students of AER201S
;==============================================================================
;
;
		ORG $E000
ABC = ($80+(1*5))
		SEI				 ; INITIALIZING THE STACK POINTER
		LDX #$FF
		TXS
    
    lda #$01
    asl a
;
		LDX #$00
    lda ($80+(1*5)),Y
    lda ($80+(1*2),X)
		LDY #$00
    
    bla bla
Delay		DEX
		BNE Delay
		DEY
		BNE Delay
;
;=============================================================================
;	 Prime Number Finder
;=============================================================================
;  This Prime Number Finder uses the sieve method to find the primes up to 255
;  and then uses those primes to find the primes up to 65535.  Note that this
;  is of course not THE most efficient way to find primes but it makes a good
;  demonstration.
;  It would be neat to stack this code up against a casually written/optimized
;  compiled C prime number finder on a raging 386.  I have a feeling there will
;  be less than a factor of ten difference on execution speed.  You may be
;  surprised just how fast the 6502 is on simple problems.
;
Test_num = $00				 ;  Test Number to Eliminate non-primes
Array	 = $AF				 ;  Base Address for the array of primes
Test_0 = %10100101
Test_1 = &66
Test_2 = 128
Test_3 = 1024
Test_4 = Test_3 + (110 - 1) * 100
Test_5 = (Delay >> 8) & $ff
Test_6 = Test_0 + -10
Test_7 = Test_1 + (Test_0 * -1)
Test_8 = %11
Test_9 = !Test_2
Test_10 = !Test_num
Test_11 = ~Test_0 & $ff
Test_12   EQU 1
Test_13 = PC

    blabla
;
;
		lda #$01
		sta $a003
		lda #$01
		sta $a001		 ;  Turns on an LED on bit zero of port A of VIA 1
				;  to let you know it has started looking for primes
;

		ldx #$01		  ;  Initialize the array of numbers
Init_Loop  txa
		sta Array,x
		inx
		bne Init_loop
;
		lda #$02		  ; Initialize the Test_num = 2
		sta Test_num
		lda #$04		  ; Put the square of 2 in the accumulator
				;	as the first non-prime
;
;  Start Setting the Multiples of the Test_num to zero
Start_num
Got_Mult	tax
    pha
    lda #0
		sta Array,x	  ; Set multiples of Test_num to zero since they
    pla
		clc				 ; are not prime.
		adc Test_num	 ; Calculate the next multiple
		bcs Next_num	 ; Until the Multiples are outside the array
		jmp Got_Mult
;
Next_num	inc Test_num	 ; Go on to the next Test_num
		ldx Test_num
		cpx #$10		  ; Until Test_num => sqrt(largest number)
		beq More_Primes
		lda Array,x
		beq Next_num	 ; Don't use Test_num if Test_num is not prime
		txa
;	 Got a valid new Test_num, now find its square because all non-primes
;		 multiples less than its square are eliminated already
		dex
		clc
Square	  adc Test_num
		dex
		bne Square
;	 OK Got the square of Test_num in the accumulator
;		 lets start checking
		jmp Start_num
;
;
More_Primes
;
;	Ok now we have all the primes up to 255 in the memory locations $01-$FF
;	  Lets repack them more neatly into an array with no spaces to make our
;	  life easier
;
		ldx #$00			 ; .X is a pointer into the loose array
		ldy #$01			 ; .Y is a pointer into the packed array
Repack	  inx
		beq Done_packing
		lda Array,x
		beq Repack
		sta Array,y
		iny
		jmp Repack
;
 
Prime_Ptr = $F0				  ; This is a points into the list of primes greater
				  ;  than $FF and less that $10000
;
Poss_Prime = $F2				 ; Possible prime
Temp		 = $F4				 ; A Temporary Number used to find modulus
Shift		= $F6				 ; Number of Places that .A is shifted
TempArg	 = $F7				 ; A temporary number; argument of modulus
 
;
Done_packing
		lda #$00			 ; Store a $00 at the end of the array of short
		sta Array,y		 ; primes so we know when we have reached the end
		lda #$00
		sta Prime_ptr	  ; Set the Prime Pointer (for primes >$FF)
		lda #$02			 ; pointing into $0200. The found primes will be
		sta Prime_ptr+1	; recorded sequentially from there on.
;
		lda #$01			 ; Start with $0101 as the first possible prime
		sta Poss_Prime
		sta Poss_Prime+1
;
Next_PP	 ldy #$02
Next_AP	 lda Array,y
		beq Prime
		jsr Mod
		beq Next_Poss_prime	  ; it was a multiple of Array,y
					; and therefore not prime
		iny
		jmp Next_AP
;
Prime		ldx #$00
		lda Poss_prime		 ; Store prime away in the array of primes
		sta (Prime_ptr,x)
		inx
		lda Poss_prime+1
		sta (Prime_ptr,x)
		clc
		lda Prime_ptr		  ; Increment the pointer in the array of primes
		adc #$02
		sta Prime_ptr
		lda Prime_ptr+1
		adc #$00
		sta Prime_ptr+1
;
Next_Poss_prime
		clc						; Increment Poss_Prime to look at the next
		lda Poss_Prime		 ; number
		adc #$01
		sta Poss_Prime
		lda Poss_Prime+1
		adc #$00
		sta Poss_Prime+1
		bcc Next_PP			 ; Carry will be set when we reach $10000
;
;	Ends when it has found all the primes up to 65535
;
;
 
		lda #$00
		sta $a001		 ; Turns off the LED after the code finishes
;
DONE		 JMP DONE		  ; Endless loop at end to halt execution
;
;
;
; --------------------------------------------------------------------------
; Find the Modulus Remainder of Poss_Prime and number in A
; --------------------------------------------------------------------------
; Input Regs: .A Number being divided into the Possible Prime
;				 Poss_Prime contains the number being tested for primeness
; Output  Regs:  .A  Modulo remainder
;
Mod			  ldx Poss_Prime		; Transfer Poss_Prime to Temp
			stx Temp
			ldx Poss_Prime+1
			stx Temp+1
			ldx #$00				; Set the bit shifting counter to #$00
			stx Shift
;
;  Compare A to the upper byte of Temp
;
Compare		 sec					  ; Compare to see if the .A is greater than
			cmp Temp+1			 ; (equal to) the high byte of Temp
			bcs A_Bigger
;
;  If the accumulator is smaller than the upper byte of Temp then shift it
;  until it is bigger or it overflows the highest bit
;
			clc
			rol a
			bcc Not_off_end
;
;  It has overflowed the highest bit, unroll it by one position
;
			ror a
			sta TempArg
			jmp Start_Mod
;
;  Not overflowed yet, go and compare it to Temp+1 again
;
Not_off_end	inc Shift
			jmp Compare
;
;  If the accumulator is bigger and it has been shifted then unshift by one
;  bit
;
A_Bigger		ldx Shift
			cpx #$00
			sta TempArg
			beq Start_Mod
			clc
			ror a
			dec Shift
			sta TempArg
;
;  If the accumulator was smaller than the highest byte of Temp it now
;	  has been shifted to strip off the high bit at least
;  If the accumulator was larger than the highest byte then proceed with the
;	  regular modulus shift and subtracts
;
Start_Mod	  lda Temp+1
			sec
			cmp TempArg
			bcc Dont_Subt
;
;  Subtract as a stage of division
;
			sbc TempArg
			sta Temp+1
;
Dont_Subt
;
;  We would now like to shift the TempArg relative the Temp
;	 1) Shift is greater than zero - accumulator was shifted - unshift it
;	 2) Shift Temp - if shift reaches -8 then we are out of Temp and
;		 what we have left is the modulus --RTS
;
			lda Shift
			bmi Sh_Temp	  ; Case 2
			beq Sh_Temp
;	Case 1
			clc
			ror TempArg
			dec Shift
			jmp Start_Mod
;
Sh_Temp		 cmp #$f8
			bne Continue
			lda Temp+1		 ;  This is the Modulus
			rts
 
Continue		dec Shift
			clc
			rol Temp
			rol Temp+1
			jmp Start_Mod
;
			ORG $FFFC
			WORD $E000
			END
