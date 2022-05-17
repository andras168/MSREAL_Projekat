all:matmul
matmul : matmul.c
	gcc -o matmul matmul.c
clean :
	rm matmul
