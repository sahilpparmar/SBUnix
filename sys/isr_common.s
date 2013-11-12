#
# isr_common.s 
# 

.macro PUSHA
    pushq %rax      
    pushq %rbx      
    pushq %rcx      
    pushq %rdx      
    pushq %rsp      
    pushq %rbp      
    pushq %rsi      
    pushq %rdi      
.endm

.macro POPA
    popq %rdi         
    popq %rsi         
    popq %rbp         
    popq %rsp         
    popq %rdx         
    popq %rcx         
    popq %rbx         
    popq %rax         
.endm

.text

.extern isr_handler

.global isr0
.global isr10
.global isr13
.global isr14
.global isr_common

isr0:
    cli
    pushq $0
    pushq $0
    jmp isr_common

isr10:
    cli
    pushq $0
    pushq $10
    jmp isr_common

isr13:
    cli
    pushq $0
    pushq $13
    jmp isr_common

isr14:
    cli
    pushq $0
    pushq $14
    jmp isr_common

isr_common:
    PUSHA
    callq isr_handler
    POPA
    add $0x10, %rsp
    sti
    iretq

