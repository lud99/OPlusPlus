%include "D:/Spel/Programming/C++/ÖPlusPlus/ÖPlusPlus\ASM Build files\io.inc"
%include "D:/Spel/Programming/C++/ÖPlusPlus/ÖPlusPlus\ASM Build files\stdlib.inc"
%include "D:/Spel/Programming/C++/ÖPlusPlus/ÖPlusPlus\ASM Build files\functions.inc"

section .data
	float_1 DQ 32767.000000
	float_2 DQ -1.000000
	float_3 DQ 1.000000
	float_4 DQ 4.000000

section .text
rand_range_float:
	; Subroutine Prologue
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	push ebx
	; Get arguments
	; min
	mov eax, [ebp + 8]
	mov dword [ebp - 8], eax
	mov eax, [ebp + 12]
	mov dword [ebp - 4], eax
	sub esp, 8
	; max
	mov eax, [ebp + 16]
	mov dword [ebp - 16], eax
	mov eax, [ebp + 20]
	mov dword [ebp - 12], eax
	sub esp, 8
	
	; Body
	; push rhs and lhs
	fld qword [float_1]
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	call rand
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	mov dword [ebp - 20], eax ; store argument Integer
	sub esp, 4
	mov ebx, dword [ebp - 20] ; evaluated argument
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	push ebx
	call to_float
	add esp, 4 ; size of arguments
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	; math operation
	fdivrp
	
	; push lhs
	; push rhs and lhs
	fld qword [ebp - 8] ; min75
	fld qword [ebp - 16] ; max39
	; math operation
	fsubrp
	
	; math operation
	fmulp
	
	; push rhs
	fld qword [ebp - 8] ; min75
	; math operation
	faddp
	
	; Subroutine Epilogue
	; Restore calle-saved registers
	pop ebx
	mov esp, ebp ; deallocate local variables
	pop ebp ; restore old base pointer
	ret
pi:
	; Subroutine Prologue
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	push ebx
	; Get arguments
	
	; Body
	mov eax, 0
	mov dword [ebp - 4], eax
	sub esp, 4
	mov eax, 100000
	mov dword [ebp - 8], eax
	sub esp, 4
	mov eax, 0
	mov dword [ebp - 12], eax
	sub esp, 4
for_start1:
	mov eax, [ebp - 12] ; i49
	push eax
	mov eax, [ebp - 8] ; its66
	pop ebx
	cmp ebx, eax
	jge for_end1
	fld qword [float_2]
	fld qword [float_3]
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	sub esp, 8
	fstp qword [esp]
	sub esp, 8
	fstp qword [esp]
	call rand_range_float
	add esp, 16 ; size of arguments
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	fstp qword [ebp - 20]
	sub esp, 8
	fld qword [float_2]
	fld qword [float_3]
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	sub esp, 8
	fstp qword [esp]
	sub esp, 8
	fstp qword [esp]
	call rand_range_float
	add esp, 16 ; size of arguments
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	fstp qword [ebp - 28]
	sub esp, 8
	; push rhs and lhs
	fld qword [ebp - 28] ; y83
	fld qword [ebp - 28] ; y83
	; math operation
	fmulp
	
	; push lhs
	; push rhs and lhs
	fld qword [ebp - 20] ; x56
	fld qword [ebp - 20] ; x56
	; math operation
	fmulp
	
	; math operation
	faddp
	
	fld qword [float_3]
	fxch st1
	fcomip
	fstp st0
	ja if_end2
	inc dword [ebp - 4]
if_end2:
	inc dword [ebp - 12]
	jmp for_start1
for_end1:
	; push rhs and lhs
	mov eax, [ebp - 8] ; its66
	mov dword [ebp - 16], eax ; store argument Integer
	sub esp, 4
	mov ebx, dword [ebp - 16] ; evaluated argument
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	push ebx
	call to_float
	add esp, 4 ; size of arguments
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	mov eax, [ebp - 4] ; inside59
	mov dword [ebp - 20], eax ; store argument Integer
	sub esp, 4
	mov ebx, dword [ebp - 20] ; evaluated argument
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	push ebx
	call to_float
	add esp, 4 ; size of arguments
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	; math operation
	fdivrp
	
	; push lhs
	fld qword [float_4]
	; math operation
	fmulp
	
	; Subroutine Epilogue
	; Restore calle-saved registers
	pop ebx
	mov esp, ebp ; deallocate local variables
	pop ebp ; restore old base pointer
	ret
global CMAIN
CMAIN:
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	mov eax, 100
	mov dword [ebp - 4], eax ; store argument Integer
	sub esp, 4
	mov ebx, dword [ebp - 4] ; evaluated argument
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	push ebx
	call srand
	add esp, 4 ; size of arguments
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	call pi
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	fstp qword [ebp - 12]
	sub esp, 8
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	call pi
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	sub esp, 9
	mov byte [ebp - 13], 0x00
	mov byte [ebp - 14], `\n`
	mov byte [ebp - 15], `f`
	mov byte [ebp - 16], `%`
	mov byte [ebp - 17], ` `
	mov byte [ebp - 18], `=`
	mov byte [ebp - 19], ` `
	mov byte [ebp - 20], `i`
	mov byte [ebp - 21], `p`
	lea eax, [ebp - 21] ; copy pointer of start of string
	mov dword [ebp - 25], eax ; store argument String
	sub esp, 4
	mov edx, dword [ebp - 25] ; evaluated argument
	push edx
	
	; Create call frame
	push ebp
	mov ebp, esp
	
	sub esp, 8
	fstp qword [esp]
	push edx
	call printf
	add esp, 12 ; size of arguments
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	pop edx
	
	; Restore call frame
	mov esp, ebp
	pop ebp
	
	ret
