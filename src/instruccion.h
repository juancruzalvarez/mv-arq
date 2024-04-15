#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

#include <stdint.h>

#include "mv.h"

typedef enum {
  MEMORIA,
  INMEDIATO,
  REGISTRO,
  NINGUNO,
} TipoOperando;

typedef enum {
  EX,       // Registro completo (4 bytes).
  L,        // Cuarto byte.
  H,        // Tercer byte.
  X,        // Tercer y cuarto byte.
} TipoReg;

typedef struct Instruccion{
  int cod_op; 
  int operacion;
  TipoOperando tipo_a;
  TipoOperando tipo_b;
  int operando_a;
  int operando_b;
} Instruccion;

Instruccion LeerProximaInstruccion(MV* mv) ;
void EjecutarInstruccion(MV* mv, Instruccion instruccion);

int GetRegistro(MV* mv, Registros reg, TipoReg tipo);
int GetValor(MV* mv, TipoOperando tipo, int operando);
void SetValor(MV* mv, TipoOperando tipo, int operando, int valor);
#endif // !INSTRUCCION_H_
