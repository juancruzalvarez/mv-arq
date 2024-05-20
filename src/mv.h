#ifndef MV_H_
#define MV_H_

#include <stdint.h>

#define DEF_MEM_SIZE 16384 // en bytes

typedef enum {
  CS = 0,
  DS,
  ES,
  SS,
  KS,
  IP,
  SP,
  BP,
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
  EJECUTANDO,
  FINALIZADO,
  ERR_SEGMENTACION,
  ERR_INSTRUCCION_INVALIDA,
  ERR_DIV_CERO,
  ERR_MEMORIA_INSUFICIENTE,
  ERR_STACK_OVERFLOW,
  ERR_STACK_UNDERFLOW,
  } Estado;

typedef struct {
  uint16_t base;
  uint16_t size;
} Segmento;

typedef struct {
  uint32_t regs[16];
  Segmento segmentos [8];
  uint8_t* mem; 
  Estado estado;
} MV;




#endif // !MV_H_

