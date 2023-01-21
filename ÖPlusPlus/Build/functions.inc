
rand_range_int:
	; Subroutine Prologue
	; Create call frame
	push ebp
	mov ebp, esp
	push ebx
	; Get arguments
	mov eax, [ebp + 8]
	mov dword [ebp - 4], eax
	sub esp, 4
	mov eax, [ebp + 12]
	mov dword [ebp - 8], eax
	sub esp, 4
	
	; Body
	; push rhs and lhs
	mov eax, [ebp - 4]
	push eax
	mov eax, [ebp - 8]
	pop ebx
	; math operation
	sub eax, ebx
	push eax
	
	; push rhs
	mov eax, 1
	pop ebx
	; math operation
	add eax, ebx
	push eax
	
	; push lhs
	mov eax, 32767
	pop ebx
	; math operation
	mov edx, 0
	idiv ebx
	push eax
	
	; push rhs
	mov eax, 1
	pop ebx
	; math operation
	add eax, ebx
	push eax
	
	; push lhs
	push edx
	; Create call frame
	push ebp
	mov ebp, esp
	call rand
	; Restore call frame
	mov esp, ebp
	pop ebp
	pop edx
	pop ebx
	; math operation
	mov edx, 0
	idiv ebx
	push eax
	
	; push lhs
	mov eax, [ebp - 4]
	pop ebx
	; math operation
	add eax, ebx
	
	; Subroutine Epilogue
	; Restore calle-saved registers
	pop ebx
	mov esp, ebp ; deallocate local varibales
	pop ebp ; restore old base pointer
	ret

to_float:
    ; Subroutine Prologue

    ; Create call frame
    push ebp
    mov ebp, esp

    push ebx
    ; Get arguments
	; x
    mov eax, [ebp + 8]
	mov dword [ebp - 4], eax
	sub esp, 4

	; Body
	fild dword [ebp - 4];

    ; Subroutine Epilogue
    ; Restore calle-saved registers
    pop ebx
    mov esp, ebp ; deallocate local variables
    pop ebp ; restore old base pointer
    ret

cos:
    ; Subroutine Prologue

    ; Create call frame
    push ebp
    mov ebp, esp

    push ebx
    ; Get arguments
    ; angle
    mov eax, [ebp + 8]
    mov dword [ebp - 8], eax
    mov eax, [ebp + 12]
    mov dword [ebp - 4], eax
    sub esp, 8

    ; Body
    ; push rhs and lhs
    fld qword [ebp - 8] ; angle
    ; math operation
    fcos

    ; Subroutine Epilogue
    ; Restore calle-saved registers
    pop ebx
    mov esp, ebp ; deallocate local variables
    pop ebp ; restore old base pointer
    ret

sin:
    ; Subroutine Prologue

    ; Create call frame
    push ebp
    mov ebp, esp

    push ebx
    ; Get arguments
    ; angle
    mov eax, [ebp + 8]
    mov dword [ebp - 8], eax
    mov eax, [ebp + 12]
    mov dword [ebp - 4], eax
    sub esp, 8

    ; Body
    ; push rhs and lhs
    fld qword [ebp - 8] ; angle
    ; math operation
    fsin

    ; Subroutine Epilogue
    ; Restore calle-saved registers
    pop ebx
    mov esp, ebp ; deallocate local variables
    pop ebp ; restore old base pointer
    ret

tan:
    ; Subroutine Prologue

    ; Create call frame
    push ebp
    mov ebp, esp

    push ebx
    ; Get arguments
    ; angle
    mov eax, [ebp + 8]
    mov dword [ebp - 8], eax
    mov eax, [ebp + 12]
    mov dword [ebp - 4], eax
    sub esp, 8

    ; Body
    ; push rhs and lhs
    fld qword [ebp - 8] ; angle
    ; math operation
    fptan
    fstp st0 ; discard the 1.0 pushed onto the stack

    ; Subroutine Epilogue
    ; Restore calle-saved registers
    pop ebx
    mov esp, ebp ; deallocate local variables
    pop ebp ; restore old base pointer
    ret

sqrt:
    ; Subroutine Prologue

    ; Create call frame
    push ebp
    mov ebp, esp

    push ebx
    ; Get arguments
    ; angle
    mov eax, [ebp + 8]
    mov dword [ebp - 8], eax
    mov eax, [ebp + 12]
    mov dword [ebp - 4], eax
    sub esp, 8

    ; Body
    ; push rhs and lhs
    fld qword [ebp - 8] ; angle
    ; math operation
    fsqrt

    ; Subroutine Epilogue
    ; Restore calle-saved registers
    pop ebx
    mov esp, ebp ; deallocate local variables
    pop ebp ; restore old base pointer
    ret