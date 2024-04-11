MOV ah, 5
MOV bx, 15
MOV [cs+3], [53]
CMP cs, ebx
NOT eax
STOP