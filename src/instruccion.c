#include "instruccion.h"
#include <stdio.h>
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


static Operacion operaciones[] = {
  MOV, ADD, SUB, SWAP, MUL, DIV, COMP, SHL, SHR, AND, OR, XOR, RND, NULL, NULL, NULL,
  SYS, JMP, JZ, JP, JN, JNZ, JNP, JNN, LDL, LDH, NOT, NULL, NULL, NULL, NULL,
  STOP
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
  ret.cod_op = instruccion;
  ret.operacion = instruccion & 0x1F;
  ret.tipo_a = (instruccion & 0x30) >> 4;
  ret.tipo_b = (instruccion & 0xC0) >> 6;
  ret.operando_a = 0;
  ret.operando_b = 0;
  for(int i = 0; i < op_size[ret.tipo_b]; i++) {
    ret.operando_b |= mv->mem[mv->regs[IP]++];
    ret.operando_b <<= 8;
  }
  ret.operando_b >>=8;
  for(int i = 0; i < op_size[ret.tipo_a]; i++) {
    ret.operando_a |= mv->mem[mv->regs[IP]++];
    ret.operando_a <<= 8;
  }
  ret.operando_a >>=8;
  return ret;
};

void EjecutarInstruccion(MV* mv, Instruccion instruccion) {
  int i = instruccion.operacion;
  if((i >=0x00 && i<= 0x0C) || (i>=0x10 && i<=0x1A) || (i==0x1F))
  operaciones[i](mv, instruccion.tipo_a, instruccion.tipo_b, instruccion.operando_a, instruccion.operando_b);
  else{
    mv->ejecutando=3;
  }
};

int GetRegistro(MV* mv, Registros reg, TipoReg tipo){
  int valor = mv->regs[reg];
  switch (tipo)
  {
  case EX:{
    return valor;
  }
  case X:{
    return ((int16_t)(valor & 0xFFFF));
  }
  case L:{
    return ((int8_t)(valor & 0xFF));
  }
  case H:{
    return ((int8_t)((valor & 0xFF00) >> 8));
  }

  }
}

// Devuelve el valor real de un operando.
// Si es inmediato devuelve el valor inmediato guardado en operando,
// si es registro devuelve el valor guardado en la part del registro especificada,
// si es memoria devuelve el valor guardado en la direccion especificada
// y si el tipo es ninguno devuelve 0.
int GetValor(MV* mv, TipoOperando tipo, int operando) {
  switch (tipo)
  {
  case NINGUNO: return 0;
  case INMEDIATO: return operando;

  case REGISTRO: {
    // Falta extension de signo
    int reg = operando & 0xF;
    int tam = (operando & 0x30) >> 4;
    return GetRegistro(mv, reg, tam);
  }

  case MEMORIA: {
    int reg = mv->regs[(operando&0xFFFF0000)>>16];
    int dir = mv->segmentos[(reg&0x00FF0000)>>16].base + (operando&0xFFFF) + (reg&0xFFFF);

    return *((int*)(&mv->mem[dir]));
  }

  }
};

// Pone valor en el lugar que determine el operando.
// Si el tipo es memoria, pone valor en el lugar de memoria apuntado por operando,
// y si el tipo es registro, pone valor en el registro que especifica el operando.
void SetValor(MV* mv, TipoOperando tipo, int operando, int valor) {
  switch (tipo)
  {
  case REGISTRO: {
    int reg = operando & 0xF;
    int tam = (operando & 0x30) >> 4;
    switch(tam) {
      case 0: {
        //reg entero
        mv->regs[reg] = valor;
        break;
      }
      case 1: {
       //4to byte 
       int8_t aux = (int8_t) valor; 
       int8_t* registro = (uint8_t*)&mv->regs[reg];
       registro[3] = aux;
       break;
      }
      case 2: {
       //3er byte
       int8_t aux = (int8_t) valor; 
       int8_t* registro = (int8_t*)&mv->regs[reg];
       registro[2] = aux;
       break;
      }
      case 3: {
       //dos ultimos bytes
       int16_t aux = (int16_t) valor; 
       int16_t* registro = (int16_t*)&mv->regs[reg];
       registro[1] = aux;
       break;
      }

    }
  }

  case MEMORIA: {
    int reg = mv->regs[(operando&0xFFFF0000)>>16];
    int dir = mv->segmentos[(reg&0x00FF0000)>>16].base + (operando&0xFFFF) + (reg&0xFFFF);

    *((int*)(&mv->mem[dir])) = valor;
  }

  }
};

//Actualiza el valor del registro CC dependiendo de la ultima operacion
void ActualizaCC(int value, MV* mv){
  if(value>0){
    mv->regs[CC] = 0x00000000;
  }else if (value < 0){
    mv->regs[CC] = 0x80000000;
  }else{
    mv->regs[CC] = 0x40000000;
  }
}

int BitN(int cc)
{
    return (cc & 0x80000000);
}

int BitZ(int cc)
{
    return (cc & 0x1);
}

void MOV(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b) {
  SetValor(mv, tipo_a, operando_a, GetValor(mv, tipo_b, operando_b));
};

void ADD(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b) {
  int suma = GetValor(mv, tipo_a, operando_a);
  suma += GetValor(mv, tipo_b, operando_b);
  SetValor(mv, tipo_a, operando_a, suma);
  ActualizaCC(suma,mv);
};

void SUB(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b) {
  int resta = GetValor(mv,tipo_a,operando_a);
  resta -= GetValor(mv,tipo_b,operando_b);
  SetValor(mv, tipo_a, operando_a, resta);
  ActualizaCC(resta,mv);
};


void SWAP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int auxA = GetValor(mv, tipo_a,operando_a);
  int auxB = GetValor(mv, tipo_b,operando_b);
  SetValor(mv,tipo_a,operando_a,auxB);
  SetValor(mv,tipo_b,operando_b,auxA);
}


void MUL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int aux = GetValor(mv, tipo_a,operando_a);
  aux *= GetValor(mv,tipo_b,operando_b);
  SetValor(mv,tipo_a,operando_a,aux);
  ActualizaCC(aux,mv);
}

void DIV(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv,tipo_b,operando_b);
  if (opb==0)
    mv->ejecutando=1;
  else{
  int value = opa / opb;
  int resto = opa % opb;
  SetValor(mv,tipo_a, operando_a,value);
  mv->regs[AC] = resto;
  ActualizaCC(value,mv);
  }
}

void COMP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv,tipo_b,operando_b);
  ActualizaCC(opa-opb,mv);
}

void SHL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv,tipo_b,operando_b);
  int res = opa << opb;
  SetValor(mv,tipo_a,operando_a,res);
  ActualizaCC(res,mv);
}

void SHR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv,tipo_b,operando_b);
  int res = opa >>  opb;
  SetValor(mv,tipo_a,operando_a,res);
  ActualizaCC(res,mv);
}

void AND(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv,tipo_b,operando_b);
  int res = opa & opb;
  SetValor(mv,tipo_a,operando_a,res);
  ActualizaCC(res,mv);
}

void OR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv,tipo_b,operando_b);
  int res = opa | opb;
  SetValor(mv,tipo_a,operando_a,res);
  ActualizaCC(res,mv);
}

void XOR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv,tipo_b,operando_b);
  int res = opa ^ opb;
  SetValor(mv,tipo_a,operando_a,res);
  ActualizaCC(res,mv);
}

void RND(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opb = GetValor(mv,tipo_b,operando_b);
  int res = rand() % (opb +1);
  SetValor(mv,tipo_a,operando_a,res);
}

void SYS(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
int value,aux,cl_val,ch_val,mode,i=0,dir;
    //int cod_Seg,ofset;
    value = GetValor(mv, tipo_a, operando_a);
    mode = mv->regs[EAX];  //modo almacenado en AL
    mode = mode & 0x00FF;


    dir = mv->regs[EDX];

    aux = mv->regs[EDX];
    cl_val = aux & 0x000000FF;          // cantidad de celdas/ cantidad de veces que se va a escribir x teclado
    ch_val = (aux & 0X0000FF00) >> 8;   // tamamnio de las celdas

    if(cl_val !=0 && ch_val != 0){
        switch(value){
            case 1:
                while(i<cl_val){
                    switch(mode){
                        case 1:
                            scanf("%d",&aux);
                            break;
                        case 2:
                            scanf("%c",&aux);
                            break;
                        case 4:
                            scanf("%o",&aux);
                            break;
                        case 8:
                            scanf("%x",&aux);
                            break;
                    }
                    for (int j = 0; j < ch_val; j++) {
                        mv->mem[dir + ch_val - j - 1] = (aux >> ((ch_val - j - 1) * 8)) & 0xFF;
                    }

                    //if(aux<0) // ver
                    //    memoria[dir] |= 0xFFFFFFFF; // Amtes tenia 0x80
                    dir += ch_val;
                    i++;
                }
                break;
            case 2:
                printf("i:%d Cval:%d",i,cl_val);
                while(i<cl_val){
                    for(int j = 0;j < ch_val; j++){
                        aux = aux << 8;
                        aux |= mv->mem[dir + j];
                    }
        printf("[%04X]",dir);
        if(mode & 0x01)
          printf("# %d\t",aux);
        if(mode & 0x02){
          if(isascii(aux)){
            printf(" ' %c\t ' ",aux);
          }else{
            printf("' . '");
          }
        }
        if(mode & 0x04)
          printf("@ %o\t",aux);

        if(mode &0x08)
          printf("% %x\t",aux);

        printf("\n");
        dir += ch_val;
        i++;
      }
      break;

    }
  }
}

void JMP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  mv->regs[IP] = opa;
}

void JZ(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){

    if (BitZ(mv->regs[CC])) // bit cero en 1
        mv->regs[IP] = GetValor(mv, tipo_a, operando_a);
    else
        mv->regs[IP]++;
}

void JP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (!BitN(mv->regs[CC]) && !BitZ(mv->regs[CC])) // bit signo en 0 y bit cero en 0
    mv->regs[IP] = GetValor(mv, tipo_a, operando_a);
  else
    mv->regs[IP]++;
}

void JN(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (BitN(mv->regs[CC])) // bit signo en 1
    mv->regs[CC] = GetValor(mv, tipo_a, operando_a);
  else
    mv->regs[IP]++;
}

void JNZ(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
    if (!BitZ(mv->regs[CC])) // bit cero en 0
      mv->regs[IP] = GetValor(mv, tipo_a, operando_a);
    else
      mv->regs[IP]++;
}

void JNP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (BitN(mv->regs[CC]) || BitZ(mv->regs[CC])) // no positivo
    mv->regs[IP] = GetValor(mv, tipo_a, operando_a);
  else
    mv->regs[IP]++;
}

void JNN(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (!BitN(mv->regs[CC]) || BitZ(mv->regs[CC])) // no negativo
    mv->regs[IP] = GetValor(mv, tipo_a, operando_a);
  else
    mv->regs[IP]++;
}

void LDL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  mv->regs[AC] &= 0xFFFF0000;
  opa &= 0x0000FFFF;
  mv->regs[AC] |= opa;
}

void LDH(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  mv->regs[AC] &= 0x0000FFFF;
  opa = opa << 16;
  mv->regs[AC] |= opa;
}

void NOT(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int res = ~opa;
  SetValor(mv,tipo_a,operando_a,res);
  ActualizaCC(res,mv);
}

void STOP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  mv->ejecutando = -1;  //se termina la ejecucion pero sin ningun error
}
