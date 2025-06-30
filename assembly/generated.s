.section __TEXT,__text
.global _start
_start:
	mov X0, #1
	adrp X1, helloworld@PAGE
	add X1, X1, helloworld@PAGEOFF
	mov X2, #13
	mov X16, #4
	svc #0x80

	mov X0, #0
	mov X16, #1
	svc #0x80
.data
helloworld: .ascii "helloworld"