MOV [3],'a'
MOV [2],'l'
MOV [1],'o'
MOV [0],'H'
MOV EDX, DS
ADD EDX, 3
MOV CH, 1
MOV CL, 4
MOV AX, %02
SYS %2
STOP