#include "disassembler.h"

#include <stdlib.h>

static const char* nombre_instrucciones [] = {
  "MOV", "ADD", "SUB", "SWAP", "MUL", "DIV", "COMP", "SHL", "SHR", "AND", "OR", "XOR", "RND", NULL, NULL, NULL,
  "SYS", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN", "LDL", "LDH", "NOT", NULL, NULL, NULL, NULL,
  "STOP" 
};

static const char* nombre_regs [] = {
  "CS", "DS", "__RES1", "__RES2", "__RES3", "IP", "__RES4", "__RES5", "CC", "AC", "A", "B", "C", "D", "E", "F"
};

static const char* tam_regs [] = {
  "EAX", " AL", " AH", " AX"
};

void MostrarOp(MV mv, TipoOperando tipo, int operando);
void ByteHex(char c);
void IntHex(int c);

void MostrarInstruccion(MV mv, Instruccion instruccion) {
  int tam_instr = 1 + ((~instruccion.tipo_a)&3) + ((~instruccion.tipo_b)&3);
  int dir = mv.regs[IP] - tam_instr;
  printf("[");
  IntHex(dir);
  printf("] ");
  for( int i = 0; i < tam_instr; i++) {
    ByteHex(mv.mem[dir + i]);
    printf(" ");
  }
  for(int i = 0; i < 7-tam_instr; i++)
    printf("   ");

  printf("| %4s ", nombre_instrucciones[instruccion.operacion]);
  if(instruccion.tipo_a != NINGUNO)
    MostrarOp(mv, instruccion.tipo_a, instruccion.operando_a);
  if(instruccion.tipo_b != NINGUNO) {
    printf(", ");
    MostrarOp(mv, instruccion.tipo_b, instruccion.operando_b);
  }
  printf(" \n");
}

void ByteHex(char c) {
  printf("%02X", c&0xFF);
}


void IntHex(int c) {
  printf("%04X", c);
}


void MostrarOp(MV mv, TipoOperando tipo, int operando){
  switch (tipo)
  {
  case INMEDIATO: {
   printf("%d", operando); 
    break;
  }
  case REGISTRO: {
    int reg = operando & 0xF;
    int tam = (operando & 0x30) >> 4; 
    if(reg > 9) {
      char* aux = malloc(4);
      strcpy(aux, tam_regs[tam]);
      char letra_reg = nombre_regs[reg][0];
      aux[1] = letra_reg;
      printf("%s", aux);
    }else {
      printf("%s", nombre_regs[reg]); 
    }
    break;
  }
  case MEMORIA: {

    int cod_reg = (operando&0xF0000) >> 16;
    int offset = operando&0xFFFF;
    char signo = offset >= 0 ? '+' : '-';
    printf("[%s %c %d]", nombre_regs[cod_reg], signo, offset); 
    break;
  }
  }
}