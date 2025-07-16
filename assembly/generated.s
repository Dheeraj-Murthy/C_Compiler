.global _start

_start:
	mov X16, #1
  push #1,
  pop #1,
	mov X0, #4
	mov X1, #5
	mul X0, X0, X1
	mov X1, #2
	sdiv X0, X0, X1
	add X0, X0, #4
	svc 0x80
