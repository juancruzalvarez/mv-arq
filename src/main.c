#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "instruccion.h"
#include "mv.h"

bool ValidarHeader(FILE* file);
bool InicializarMV(FILE* file, MV* mv);

int main(int argc, char** argv) {

  MV mv;
  FILE* file = fopen(argv[1], "rb");
  
  if(!file) {
    printf("No se pudo abrir el archivo \"%s\".\n", argv[0]);
    return -1;
  }

  if(!ValidarHeader(file))
    return -1;

  if(!InicializarMV(file, &mv))
    return -1;

  while(mv.ejecutando) {
    Instruccion instr = LeerProximaInstruccion(&mv);
    // en modo debug escribir por pantalla la instruccion.
    EjecutarInstruccion(&mv, instr);
  }
  
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
  int tam_codigo = 0;
  
  fread(&tam_codigo, sizeof(uint16_t), 1, file);
  fread(mv->segmentos, sizeof(Segmento), 8, file);

  // Nescesario??
  for(int i = 0; i < 16; i++) {
    mv->regs[i] = 0;
  }

  mv->regs[CS] = mv->segmentos[CODE].base;
  mv->regs[DS] = mv->segmentos[DATA].base;
  mv->regs[IP] = mv->regs[CS];
  
  mv->mem = (uint8_t*) malloc(sizeof(uint8_t) * MEM_SIZE);

  fread(&mv->mem[mv->regs[CS]], sizeof(uint8_t), mv->segmentos[CODE].size, file);

  mv->ejecutando = true;

  return true;
};
