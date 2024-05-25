#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    FILE* file = fopen(vmx_path, "rb");
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
  printf("Inicializacion completada!\n");
  while(mv.estado == EJECUTANDO) {
    Instruccion instr = LeerProximaInstruccion(&mv);
    printf("[%8x] %x | %x", mv.regs[IP], instr.cod_op, instr.operacion);
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
    case ERR_MEMORIA_INSUFICIENTE:
      printf("Error: Memoria Insuficeinte.\n");
      break;
    case ERR_STACK_OVERFLOW:
      printf("Error: Stack Overflow.\n");
      break;
    case ERR_STACK_UNDERFLOW:
      printf("Error: Stack Underflow.\n");
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
  } tamanos;

  fread(&tamanos, sizeof(tamanos), 1, file);
  uint8_t *ptra = &tamanos;
  for(int i = 0; i < 5;  i++) {
    int tmp = ptra[i*2];
    ptra[i*2] = ptra[i*2+1];
    ptra[i*2+1] = tmp;
  }
  uint16_t entry_offset;
  fread(&entry_offset, sizeof(uint8_t), 2, file);
  ptra = (uint8_t*) (&entry_offset);
  int tmp = ptra[1];
  ptra[1] = ptra[0];
  ptra[0] = tmp;

  if(tamanos.tam_code + tamanos.tam_const + tamanos.tam_data + tamanos.tam_extra + tamanos.tam_stack >= tam_memoria) {
    printf("Memoria insuficiente.\n");
    return false;
  }

  mv->mem = (uint8_t*) malloc(sizeof(uint8_t) * tam_memoria);
  fread(&mv->mem[tamanos.tam_const], sizeof(uint8_t), tamanos.tam_code, file);
  fread(&mv->mem[0], sizeof(uint8_t), tamanos.tam_const, file);
  //for(int i = 0; i < 105; i++){
  //  printf("MEM[%d] : %c | %x \n", i, mv->mem[i], mv->mem[i]& 0x1F);
 // }
  int tams[5] = {tamanos.tam_const, tamanos.tam_code, tamanos.tam_data, tamanos.tam_extra, tamanos.tam_stack};
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
    if(tams[i+1] > 0) {
      mv->regs[CS + i] = aux;
      mv->regs[CS + i] <<= 16;
      aux++; 
    }else {
      mv->regs[CS + i] = -1;
    }
  }
  mv->regs[SP] = mv->regs[SS] + tamanos.tam_stack; 
  printf("tamaño stack: %d", tamanos.tam_stack);
  mv->regs[IP] = mv->regs[CS] + entry_offset;
  printf("CS: %8x\n", mv->regs[CS]);
  printf("IP: %8x\n", mv->regs[IP]);
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
    if(kVMIHeaderID[i] != header.id[i]) {
      id_ok = false;
      break;
    }
  }

  if(!id_ok) {
    printf("Header invalido. Identificador incorrecto. \n");
    return false;
  }

  if(kVMIHeaderVersion != header.version){ 
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
  for(int i = 0; i < 16; i++) {
    printf("[%d]: %8x \n", i, mv->regs[i]);

  }
  return true;

  
}
