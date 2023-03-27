%include "D:/Spel/Programming/C++/ÖPlusPlus/ÖPlusPlus\ASM Build files\io.inc"
%include "D:/Spel/Programming/C++/ÖPlusPlus/ÖPlusPlus\ASM Build files\stdlib.inc"
%include "D:/Spel/Programming/C++/ÖPlusPlus/ÖPlusPlus\ASM Build files\functions.inc"

section .data

section .text
global CMAIN
CMAIN:
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	; push rhs and lhs
	mov eax, 30
	push eax
	mov eax, 2
	pop ebx
	; math operation
	sub eax, ebx
	push eax
	
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	ret
