SmuCPU:SmuCPU.o
	gcc SmuCPU.o -o SmuCPU
	
SmuCPU.o:SmuCPU.S
	gcc -c SmuCPU.S -o SmuCPU.o	
	
SmuCPU.S:SmuCPU.i
	gcc -S SmuCPU.i -o SmuCPU.S
		
SmuCPU.i:SmuCPU.c
	gcc -E SmuCPU.c -o SmuCPU.i
	
.PHONY:
clear:
	rm -rf SmuCPU.i SmuCPU.S SmuCPU.o
	
	

	
