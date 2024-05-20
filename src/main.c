#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instruccion.h"
#include "mv.h"
#include "disassembler.h"


bool ValidarArgumentos(int argc, char** argv, char** mvx_path, char** mvi_path, int* m, bool* d);
bool ValidarHeader(FILE* file);
bool InicializarMVX(FILE* file, MV* mv, int tam_memoria);
bool InicializarMVI(FILE* file, MV* mv);


int main(int argc, char** argv) {
  MV mv;
  char *vmi_path = NULL;
  char *vmx_path = NULL;
  int tam_memoria = DEF_MEM_SIZE;
  bool d = false;

  if(!ValidarArgumentos(argc, argv, &vmx_path, &vmi_path, &tam_memoria, &d))
    return -1;
  
  if(vmx_path) {
    FILE* file = fopen(vmi_path, "rb");
    if(!file) {
      printf("No se pudo abrir el archivo %s. \n", vmx_path);
      return -1;
    }
    if(!InicializarMVX(file, &mv, tam_memoria)) {
      return -1;
    }
  }else {
    FILE* file = fopen(vmi_path, "rb");
    if(!file) {
      printf("No se pudo abrir el archivo %s. \n", vmx_path);
      return -1;
    }
    if(!InicializarMVI(file, &mv)) {
      return -1;
    }
  }
 
  while(mv.estado == EJECUTANDO) {
    Instruccion instr = LeerProximaInstruccion(&mv);
    if(d && mv.estado == EJECUTANDO) 
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

bool ValidarArgumentos(int argc, char** argv, char** mvx_path, char** mvi_path, int* m, bool* d){

  for(int i = 1; i < argc; i++) {
    char* argumento_completo = strdup(argv[i]);
    if(strcmp(argv[i], "-d") == 0) {
      *d = true;
      continue;
    }
    char* tok;
    tok = strtok(argv[i], "=");
    if(strcmp(tok, argumento_completo) != 0) {
      
      if(strcmp(tok, "M") != 0) {
        printf("Argumento invalido. \n");
        return false;
      }else {
        tok = strtok(NULL, "=");
        // falta verificar que tok sea un numero aca.
        *m = atoi(tok);
        continue;
      }
    }

    tok = strtok(argv[i], ".");

    if(strcmp(tok, argumento_completo) != 0) {
      tok = strtok(NULL, ".");
      if(strcmp(tok, "vmx") == 0) {
        *mvx_path = argumento_completo;
        continue;
      }else if(strcmp(tok, "vmi") == 0) {
        *mvi_path = argumento_completo;
        continue;
      }else {
        printf("No se reconoze la extension del archivo. (%s)\n", argumento_completo);
        return false;
      }
    }
    printf("Argumento desconocido( %s ). \n", argumento_completo);
    return false;
  }  

  if(!(*mvx_path) && !(*mvi_path)) {
    printf("Se debe especifica archivo .mvx o .mvi. \n");
    return false;
  }
  return true;
}

const char* kHeaderID      = "VMX24";
const int   kHeaderVersion = 2;

bool ValidarHeaderVMX(FILE* file) {

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
bool InicializarMVX(FILE* file, MV* mv, int tam_memoria) {

  if(!ValidarHeaderVMX(file)) {
    return false;
  }

  srand(time(0));

  struct {
    uint16_t tam_code;
    uint16_t tam_data;
    uint16_t tam_extra;
    uint16_t tam_stack;
    uint16_t tam_const;
  } tamaños;

  fread(&tamaños, sizeof(tamaños), 1, file);

  int entry_offset;
  fread(&entry_offset, sizeof(uint16_t), 1, file);
  
  if(tamaños.tam_code + tamaños.tam_const + tamaños.tam_data + tamaños.tam_extra + tamaños.tam_stack >= tam_memoria) {
    printf("Memoria insuficiente.\n");
    return false;
  }

  mv->mem = (uint8_t*) malloc(sizeof(uint8_t) * tam_memoria);
  fread(&mv->mem[tamaños.tam_const], sizeof(uint8_t), tamaños.tam_code, file);
  fread(&mv->mem[0], sizeof(uint8_t), tamaños.tam_const, file);
  int tams[5] = {tamaños.tam_const, tamaños.tam_code, tamaños.tam_data, tamaños.tam_extra, tamaños.tam_stack};
  int j = 0;
  int offset = 0;
  for(int i = 0; i < 5; i++) {
    if(tams[i] > 0) {
      mv->segmentos[j].base = offset;
      mv->segmentos[j].size = tams[i];
      offset += tams[i];
      j++;
    }
  }
  
  int aux = 0;
  if(tams[0] <= 0) {
    mv->regs[KS] = -1;
  }else {
    mv->regs[KS] = 0;
    aux++;
  }

  for(int i = 0; i < 4; i++) {
    if(tams[i] > 0) {
      mv->regs[CS + i] = aux;
      mv->regs[CS + i] <<= 16;
      aux++; 
    }else {
      mv->regs[CS + i] = -1;
    }
  }
  mv->regs[SP] = mv->regs[SS] + tamaños.tam_stack; 
  mv->regs[IP] = mv->regs[CS] + entry_offset;
  mv->estado = EJECUTANDO;

  return true;
};


const char* kVMIHeaderID      = "VMI24";
const int   kVMIHeaderVersion = 1;

bool ValidarHeaderVMI(FILE* file) {
  
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
}

bool InicializarMVI(FILE* file, MV* mv) {
  if(!ValidarHeaderVMI(file)) {
    return false;
  }
  uint16_t tam_memoria;
  fread(&tam_memoria, sizeof(uint16_t), 1, file);
  mv->mem = malloc(sizeof(uint8_t) * tam_memoria); // alocar espacio para el programa.
  fread(mv, sizeof(uint32_t), 16, file); // leer 16 registros
  fread(mv, sizeof(uint32_t), 8, file);  // leer segmentos
  fread(mv->mem, sizeof(uint8_t), tam_memoria, file); // leer memoria principal
  mv->estado = EJECUTANDO;
  return true;
}
