.data
extern g_memory:qword
extern g_stack_instruction:qword	; pointer to instruction stack's content

.code
; rcx - size of array
; r10 - pointer to function
; r11 - pointer to array's content
native_call proc
	;mov rcx, qword ptr [g_stack_array + 8]	; Array count
	;mov r11, g_stack_array
	mov r11, [r11+rcx*8]			; Top array's content
	mov rcx, qword ptr [r11 + 8]	; Top array's max_index

	mov r10, qword ptr [g_stack_instruction + 8]	; Max index of instruction stack
	dec r10											; Get before top instruction index, where function pointer resides
	mov r9, g_stack_instruction
	mov r10, [r9 + r10*8]							
	mov r10, [r10 + 4]								; Get shift
	mov r9, g_memory
	mov r10, qword ptr [r9 + r10]					; Get function pointer

	cmp rcx, 4
	jg param_4_plus
	mov rdx, switch
	add rdx, rcx
	jmp rdx

	switch:
		jmp param_0
		jmp param_1
		jmp param_2
		jmp param_3
		jmp param_4
		
	param_4_plus:
		mov r9, [r11+rcx*8]
	param_4:
		mov rdx, r11
	param_3:
		
	param_2:
		
	param_1:
		mov rcx, [r11]
	param_0:
		call r10
		ret
native_call  endp
end