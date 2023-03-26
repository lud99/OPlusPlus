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
	
	mov eax, 0
	mov dword [ebp - 4], eax
	sub esp, 4
	sub esp, 4
	mov byte [ebp - 5], 0x00
	mov byte [ebp - 6], `c`
	mov byte [ebp - 7], `b`
	mov byte [ebp - 8], `a`
	lea eax, [ebp - 8] ; copy pointer of start of string
	mov dword [ebp - 12], eax
	sub esp, 4
while_start1:
	mov eax, 1
	push eax
	mov eax, 1
	pop ebx
	cmp ebx, eax
	jne while_end1
	mov eax, [ebp - 12] ; s29
	mov dword [ebp - 16], eax ; store argument String
	sub esp, 4
	mov ebx, dword [ebp - 16] ; evaluated argument
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	push ebx
	call printf
	add esp, 4 ; size of arguments
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	inc dword [ebp - 4]
	jmp while_start1
while_end1:
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	ret
