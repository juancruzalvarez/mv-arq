MOV EAX, 5
MOV EBX, 5
ADD EAX, EBX
MOV [3], EAX
MOV EDX, DS
ADD EDX, 3
MOV CL, 1
MOV CH, 1
MOV AL, 1
SYS 2
STOP
