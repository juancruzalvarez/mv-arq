#ifndef MV_H_
#define MV_H_

#include <stdbool.h>
#include <stdint.h>

#define MEM_SIZE 16384 // en bytes

typedef enum {
  CS = 0,
  DS,
  __RES1,
  __RES2,
  __RES3,
  IP,
  __RES4,
  __RES5,
  CC,
  AC,
  EAX,
  EBX,
  ECX,
  EDX,
  EEX,
  EFX,
} Registros;

typedef enum {
  CODE,
  DATA,
} Segmentos;

typedef struct {
  uint16_t base;
  uint16_t size;
} Segmento;

typedef struct {
  Segmento segmentos[8];
  uint32_t regs[16];
  uint8_t* mem; 
  int ejecutando;
} MV;




#endif // !MV_H_

