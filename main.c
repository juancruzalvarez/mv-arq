#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instruccion.h"
#include "mv.h"
#include "disassembler.h"


bool ValidarArgumentos(int argc, char** argv, char** file_path, bool* d);
bool ValidarHeader(FILE* file);
bool InicializarMV(FILE* file, MV* mv);


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

  while(mv.estado == EJECUTANDO) {
    Instruccion instr = LeerProximaInstruccion(&mv);
    if(d) 
      MostrarInstruccion(mv, instr);
    EjecutarInstruccion(&mv, instr);
  }

  switch(mv.estado){
    case ERR_DIV_CERO:
      printf("Error: Division por cero.\n");
      break;
    case ERR_SEGMENTACION:
      printf("Error: Fallo de Segmento.\n");
      break;
    case ERR_INSTRUCCION_INVALIDA:
      printf("Error: Instruccion invalida.\n");
      break;
    case FINALIZADO:
      printf("Ejecucion finalizada correctamente.\n"); 
      break;
  }
  
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

  srand(time(0));

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
  mv->regs[CS] = 0x0;
  mv->regs[DS] = 0x00010000;
  mv->regs[IP] = 0x0;
  
  mv->mem = (uint8_t*) malloc(sizeof(uint8_t) * MEM_SIZE);

  fread(&mv->mem[0], sizeof(uint8_t), mv->segmentos[CODE].size, file);

  mv->estado = EJECUTANDO;

  return true;
};
