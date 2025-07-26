.global _start

_start:
 push 0
 push 0
 push 0
 push 1
loop0:
 mov rax, 100
 push rax
 mov rbx, 100
 push rbx
 pop rbx
 pop rax
 cmp rax, rbx
 je label0
 pop rax
mov QWORD [rsp + 8], rax
 push QWORD [rsp + 8]
 push QWORD [rsp + 8]
 pop rax
 push rax
 pop rax
 push QWORD [rsp + 16]
 pop rbx
 add rax, rbx
 push rax
 pop rax
mov QWORD [rsp + 18446744073709551608], rax
 push QWORD [rsp + 18446744073709551608]
 jmp loop0
label0:
 pop rsi
 pop rsi
 pop rsi
 pop rsi
