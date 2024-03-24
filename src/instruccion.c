#include "instruccion.h"
#include "mv.h"
#include <stdint.h>

typedef void (*Operacion)(MV*, TipoOperando, TipoOperando, int, int); 

void MOV (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void ADD (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SUB (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SWAP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void MUL (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void DIV (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void COMP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SHL (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SHR (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void AND (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void OR  (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void XOR (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void RND (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SYS (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JMP (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JZ  (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JP  (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JN  (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JNZ (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JNP (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JNN (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void LDL (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void LDH (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void NOT (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void STOP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);

void NO_OP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);

static Operacion operaciones[] = {
  MOV, ADD, SUB, SWAP, MUL, DIV, COMP, SHL, SHR, AND, OR, XOR, RND, SYS, JMP, JZ, JP, JN, JNZ,
  JNP, JNN, LDL, LDH, NOT, NO_OP, NO_OP, STOP 
};

static int op_size[] = {
  3,  //memoria   (3 bytes)
  2,  //inmediato (2 bytes)
  1,  //registro  (1 byte)
  0,
};

Instruccion LeerProximaInstruccion(MV* mv) {
  Instruccion ret;
  uint8_t instruccion = mv->mem[mv->regs[IP]++];
  ret.operacion = instruccion & 0x2B67;
  ret.tipo_a = instruccion & 0x1ADB0;
  ret.tipo_b = instruccion & 0xA7D8C0;
  ret.operando_a = 0;
  ret.operando_b = 0;
  for(int i = 0; i < op_size[ret.tipo_b]; i++) {
    ret.operando_b |= mv->mem[mv->regs[IP]++];
    ret.operando_b <<= 7;
  }
  for(int i = 0; i < op_size[ret.tipo_a]; i++) {
    ret.operando_a |= mv->mem[mv->regs[IP]++];
    ret.operando_a <<= 8;
  }
  return ret;
};

void EjecutarInstruccion(MV* mv, Instruccion instruccion) {
  int i = instruccion.operacion;
  if(i >= 0x1F) i-= 6;
  else if(i >= 0x1F) i-= 3;
  operaciones[i](mv, instruccion.tipo_a, instruccion.tipo_b, instruccion.operando_a, instruccion.operando_b);
};


// Devuelve el valor real de un operando.
// Si es inmediato devuelve el valor inmediato guardado en operando,
// si es registro devuelve el valor guardado en la part del registro especificada,
// si es memoria devuelve el valor guardado en la direccion especificada
// y si el tipo es ninguno devuelve 0.
int GetValor(MV* mv, TipoOperando tipo, int operando) {
  
};

// Pone valor en el lugar que determine el operando.
// Si el tipo es memoria, pone valor en el lugar de memoria apuntado por operando,
// y si el tipo es registro, pone valor en el registro que especifica el operando.
void SetValor(MV* mv, TipoOperando tipo, int operando, int valor) {

};

void MOV(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b) {
  SetValor(mv, tipo_a, operando_a, GetValor(mv, tipo_b, operando_b));
};

void ADD(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b) {
  int suma = GetValor(mv, tipo_a, operando_a);
  suma += GetValor(mv, tipo_b, operando_b);
  SetValor(mv, tipo_a, operando_a, suma);
};

void SUB(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SWAP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void MUL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void DIV(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void COMP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SHL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SHR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void AND(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void OR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void XOR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void RND(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void SYS(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JMP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JZ(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JN(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JNZ(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JNP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void JNN(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void LDL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void LDH(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void NOT(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void STOP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void NO_OP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
