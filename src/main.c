#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instruccion.h"
#include "mv.h"


bool ValidarArgumentos(int argc, char** argv, char** file_path, bool* d);
bool ValidarHeader(FILE* file);
bool InicializarMV(FILE* file, MV* mv);

void Debug(MV mv, Instruccion instruccion);

int main(int argc, char** argv) {
  MV mv;
  char* file_path = NULL;
  bool d = false;

  if(!ValidarArgumentos(argc, argv, &file_path, &d))
    return -1;
  
  FILE* file = fopen(file_path, "rb");
  
  if(!file) {
    printf("No se pudo abrir el archivo \"%s\".\n", file_path);
    return -1;
  }

  if(!ValidarHeader(file))
    return -1;


  if(!InicializarMV(file, &mv))
    return -1;
  
  while(mv.ejecutando == 0) {
    Instruccion instr = LeerProximaInstruccion(&mv);

    EjecutarInstruccion(&mv, instr);
    if(d && mv.ejecutando == 0)
    Debug(mv, instr);
  }

  switch(mv.ejecutando){
    case 1:
      printf("No se admite la division por 0");
      break;
    case 2:
      printf("Fallo de Segmento");
      break;
    case 3:
      printf("Instrucci�n invalida");
      break;
    case -1:
      printf("STOP");   //EJECUCION TERMINADA CORRECTAMENTE
      break;

  }
  printf("final");
  
  return 0;
}

bool ValidarArgumentos(int argc, char** argv, char** file, bool* d) {
  if(argc < 2) {
    printf("No se especifico el archivo a ejecutar.\n");
    return false;
  }
  *file = argv[1];
  if(argc == 3) {
    if(strcmp(argv[2], "-d") != 0) {
      printf("Argumento invalido: %s.\n", argv[2]);
      return false;
    }else {
      *d = true;
    }
  } 
  return true;
}

const char* kHeaderID      = "VMX24";
const int   kHeaderVersion = 1;

bool ValidarHeader(FILE* file) {

  struct {
    uint8_t id[5];
    uint8_t version;
  } header;

  fread(&header, sizeof(header), 1, file);

  bool id_ok = true;
  for(int i = 0; i < 5; i++) {
    if(kHeaderID[i] != header.id[i]) {
      id_ok = false;
      break;
    }
  }

  if(!id_ok) {
    printf("Header invalido. Identificador incorrecto. \n");
    return false;
  }

  if(kHeaderVersion != header.version){ 
    printf("Header invalido. Version incorrecta. \n"); 
    return false;
  }

  return true;
};

bool InicializarMV(FILE* file, MV* mv) {
  uint16_t tam_codigo = 0;
  
  fread(&tam_codigo, sizeof(uint16_t), 1, file);
  uint8_t aux = tam_codigo&0xFF;
  tam_codigo >>= 8;
  tam_codigo |= (aux << 8);

  mv->segmentos[CODE].base = 0;
  mv->segmentos[CODE].size = tam_codigo;
  mv->segmentos[DATA].base = tam_codigo;
  mv->segmentos[DATA].size = MEM_SIZE - tam_codigo;

  for(int i = 0; i < 16; i++) {
    mv->regs[i] = 0;
  }
  mv->regs[CS] = mv->segmentos[CODE].base;
  mv->regs[DS] = mv->segmentos[DATA].base;
  mv->regs[IP] = mv->regs[CS];
  
  mv->mem = (uint8_t*) malloc(sizeof(uint8_t) * MEM_SIZE);

  fread(&mv->mem[mv->regs[CS]], sizeof(uint8_t), mv->segmentos[CODE].size, file);

  mv->ejecutando = 0;

  return true;
};

void ByteHex(char c) {
  printf("%02X", c&0xFF);
}


void IntHex(int c) {
  printf("%04X", c);
}

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
void Debug(MV mv, Instruccion instruccion) {
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