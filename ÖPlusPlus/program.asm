section .data

section .text
global CMAIN
CMAIN:
	push ebp ; save old top of stack
	mov ebp, esp ; current top of stack is bottom of new stack frame
	sub esp, 12
	mov byte [ebp - 1], 0x00
	mov byte [ebp - 2], "x"
	mov byte [ebp - 3], "%"
	mov byte [ebp - 4], " "
	mov byte [ebp - 5], "r"
	mov byte [ebp - 6], "u"
	mov byte [ebp - 7], "a"
	mov byte [ebp - 8], "s"
	mov byte [ebp - 9], "o"
	mov byte [ebp - 10], "n"
	mov byte [ebp - 11], "i"
	mov byte [ebp - 12], "d"
	lea eax, [ebp - 12] ; copy pointer of start of string
	mov dword [ebp - 16], eax
	sub esp, 4
	mov eax, 4
	push eax
	mov dword [ebp - 20], eax ; store argument
	sub esp, 4
	mov eax, [ebp - 16]
	push eax
	mov dword [ebp - 24], eax ; store argument
	sub esp, 4
	mov ecx, dword [ebp - 20] ; evaluated argument
	mov eax, dword [ebp - 24] ; evaluated argument
	push ebp ; save old top of stack
	mov ebp, esp ; current top of stack is bottom of new stack frame
	push ecx
	push eax
	call printf
	add esp, 8 ; Assume all arguments are 4 bytes each
	mov esp, ebp ; restore esp, now points to old ebp(start of frame)
	pop ebp ; restore old ebp
	mov esp, ebp ; restore esp, now points to old ebp(start of frame)
	pop ebp ; restore old ebp
	ret
