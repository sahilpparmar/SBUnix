#
# irq_common.s -- IRQ common handler
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

.extern irq_handler

.global irq0
.global irq1
.global irq_common

irq0:
    cli
    pushq $0x0
    pushq $0x20
    jmp irq_common
 
irq1:
    cli
    pushq $0x0
    pushq $0x21
    jmp irq_common

irq_common:
    PUSHA
    callq irq_handler
    POPA
    add $0x10, %rsp
    sti
    iretq

