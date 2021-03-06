; Name: Andreas Yiannakou, Student Number: 1347357  Username: ay13695
; Declaration of authorship: This file was written entirely by me. 
; Filename: calendarcalc.asm
; Purpose: To calculate the day of the week you were born using Zeller's congruence and store a number between 0-6 in memory[FFF].
; 0 = Saturday, 1 = Sunday, 2 = Monday, 3 = Tuesday, 4 = Wednesday, 5 = Thursday and 6 = Friday.
; For more info on Zeller's algorithm, http://en.wikipedia.org/wiki/Zeller%27s_congruence 
; To check the asm file is working correctly, http://www.mathsisfun.com/games/dayofweek.html

Day:			SET 30
Month:		SET 3
Year:			SET 1990	
		
					ldc Month
					ldc 3
					sub 
					brlz adjust
					ldc Month
					stl -1
					ldc Year
					stl -2
					ldc Month
					adc 1
					stl -6
					br findK
sum:			ldc Day
					ldl -8
					add
					ldl -11
					add
					ldl -9
					add		
					ldl -5
					sub
					stl 0		
hloop:		ldl 0
					adc -7
					brlz 2 
					stl 0
					br hloop 
					HALT	

adjust:		ldc Month
					adc 12
					stl -1
					ldc Year
					adc -1
					stl -2
					ldc Month
					adc 13
					stl -6
					
findK:		ldl -2
					stl -3		
					ldl -3
Kloop:		adc -100
					brlz FindJ
					stl -3
					br Kloop 
					
FindJ:    ldl -2
					ldl -3
					sub
Jloop:		adc -100
					brz Find2J
					ldl -4
					adc 1
					stl -4
					br Jloop

Find2J:		ldl -4
					ldl	-4
					add
					stl -5
					br divide1

divide1:	ldl -4					
dloop1:		adc D1
					brlz multi1
					brz multi1
					ldl -9
					adc 1
					stl -9
					br dloop1	


M1:				SET 26
M2:				SET 5
D1:				SET -4
D2:				SET -10	

multi1:		ldl -6
					adc -1
					stl -100
					ldc M1
					stl -7

mloop1:		ldl -100
					brz divide2
					ldl	-7
					ldc M1
					add
					stl -7
					ldl -100
					adc -1
					stl -100
					br mloop1

divide2:	ldl -7					
dloop2:		adc D2
					brlz multi2
					brz  multi2
					ldl -8
					adc 1
					stl -8
					br dloop2	

multi2:		ldl -3
					adc -1
					stl -100
					ldc M1
					stl -10

mloop2:		ldl -100
					brz divide3
					ldl	-10
					ldc M2
					add
					stl -10
					ldl -100
					adc -1
					stl -100
					br mloop2

divide3:	ldl -10					
dloop3:		adc D1
					brlz sum
					brz  sum
					ldl -11
					adc 1
					stl -11
					br dloop3
