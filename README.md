# Software CPU emulator
This is a simple "CPU emulator", which consists of the following components:
1. **Assembler [asm+.exe]** - translates assembly (.asy) programs into bytecode (.bsy) "executable" files.
2. **Disassembler [asm-.exe]** - translates back bytecode into assembly. 
3. **CPU emulator [scpu.exe]** - runs bytecode programs.

### Assembly syntax
These are all the commands that are supported:

*Input*
1. in
2. out

*Arithmetic*
3. add
4. sub
5. mul
6. div

*Math*
7. pow
8. sqrt
9. sin
10. cos
11. abs
12. flr

*Stack*
13. push
14. pop

*Control flow*
15. call
16. ret
17. jmp
18. jae
19. ja
20. jb
21. jbe
22. je
23. jne
24. hlt

*Graphics*
25. upd
26. clr

### Example 1 - factorial
```Lisp
; number of which to take the factorial
in

; calling fact function, after its execution the answer will be on top of the stack
call :fact
out

hlt

fact:
	pop rax
	push 1
	call :fact_rec

	ret

fact_rec:
	push rax
	mul

	push rax
	push 1
	sub

	pop rax

	push 1
	push rax

	jae :end
		call :fact_rec

	end:
		ret
```

### Example 2 - quadratic equation
```Lisp
; input a, b, c coefficients of a quadratic equation (ax^2 + bx + c = 0)
in
in
in

call :sqeq
hlt

; solves square equation
; takes three coefficients from stack
sqeq:
	pop rcx
	pop rbx
	pop rax

	; if rax == 0 solve linear
	push rax
	push 0
	je :lineq

	; if rax != 0 solve square
	jmp :square

; solves linear equation
lineq:
	push rcx
	
	push rbx
	push -1
	mul

	div

	out

	ret

; solves square equation in case rax != 0
square:
	push rbx
	push rbx
	mul

	push 4
	push rax
	push rcx
	mul
	mul

	sub

	pop rdx

	push rdx
	push 0
	ja :twosols

    push rdx
    push 0
	je :onesol

    jmp :nosol

	twosols:
        push 2
        out

        push rdx
		sqrt
		pop rdx

		push rbx
		push -1
		mul
		push rdx
		add
		push 2
		div
		out

		push rbx
		push -1
		mul
		push rdx
		sub
		push 2
		div
		out

		ret

	onesol:
        push 1
        out

		push rbx
		push -1
		mul

		push 2
		push rax
		mul

		div

		out

		ret

    nosol:
        push 0
        out

```

### Example 3 - basic wireframe renderer of a 3d cube

*this and the previous examples can be found in the* examples/ *folder*

# Libraries used
1. [SDL2](https://www.libsdl.org/)
2. (my) file_manager
3. (my) stack
4. (my) log_generator