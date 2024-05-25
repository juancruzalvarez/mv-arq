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
void PUSH (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void POP (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void RET (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);
void CALL (MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);

void STOP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b);


static Operacion operaciones[] = {
  MOV, ADD, SUB, SWAP, MUL, DIV, COMP, SHL, SHR, AND, OR, XOR, RND, NULL, NULL, NULL,
  SYS, JMP, JZ, JP, JN, JNZ, JNP, JNN, LDL, LDH, NOT, PUSH, POP, CALL, RET,
  STOP
};

static int op_size[] = {
  3,  //memoria   (3 bytes)
  2,  //inmediato (2 bytes)
  1,  //registro  (1 byte)
  0,
};


void GenerarVMI(MV* mv) {
  FILE* archivo_vmi = fopen(mv->path_vmi, "wb");
  const char* id = "VMI24";
  uint16_t version = 2;
  fwrite(id, 1, 5, archivo_vmi);
  fwrite(&version, 1, 1, archivo_vmi);
  fwrite(&mv->tam_memoria, 2, 1, archivo_vmi);
  fwrite(mv->regs, 4, 16, archivo_vmi);
  fwrite(mv->segmentos, 4, 8, archivo_vmi);
  fwrite(mv->mem, 1, mv->tam_memoria, archivo_vmi);
  fclose(archivo_vmi);
}

Instruccion LeerProximaInstruccion(MV* mv) {
  Instruccion ret;
  printf("Leyendo instruccion\n");
  int code_seg_id = (mv->regs[IP]>>16)&0xFFFF;
  int dir = mv->segmentos[code_seg_id].base + (mv->regs[IP]&0xFFFF);
  if(dir >= mv->segmentos[code_seg_id].base + mv->segmentos[code_seg_id].size) {
    mv->estado = FINALIZADO;
    return ret;
  }
 
  uint8_t instruccion = mv->mem[dir];
  mv->regs[IP]++;
  dir++;
  ret.cod_op = instruccion;
   
  ret.operacion = instruccion & 0x1F;
  ret.tipo_a = (instruccion & 0x30) >> 4;
  ret.tipo_b = (instruccion & 0xC0) >> 6;
  ret.operando_a = 0;
  ret.operando_b = 0;
  printf("Tipos: %d, %d\n", ret.tipo_a, ret.tipo_b);

  for(int i = 0; i < op_size[ret.tipo_b]; i++) {
    ret.operando_b |= mv->mem[dir];
    ret.operando_b <<= 8;
    mv->regs[IP]++;
    dir++;
  }
  if(ret.tipo_b != NINGUNO)
    ret.operando_b >>=8;
  for(int i = 0; i < op_size[ret.tipo_a]; i++) {
    ret.operando_a |= mv->mem[dir];
    ret.operando_a <<= 8;
    mv->regs[IP]++;
    dir++;
  }
  if(ret.tipo_a != NINGUNO)
    ret.operando_a >>=8;

  return ret;
};

void EjecutarInstruccion(MV* mv, Instruccion instruccion) {
  int i = instruccion.operacion;
  if((i >=0x00 && i<= 0x0C) || (i>=0x10 && i<=0x1f))
    operaciones[i](mv, instruccion.tipo_a, instruccion.tipo_b, instruccion.operando_a, instruccion.operando_b);
  else{
    mv->estado = ERR_INSTRUCCION_INVALIDA;
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
    int aux = valor&0xFFFF;
    int es_negativo = aux&0x8000;
    return es_negativo? (aux|0xFFFF0000) : aux;
  }
  case L:{
    int aux = valor&0xFF;
    int es_negativo = aux&0x80;
    return es_negativo? (aux|0xFFFFFF00) : aux;
  }
  case H:{
    int aux = (valor>>8)&0xFF;
    int es_negativo = aux&0x80;
    return es_negativo? (aux|0xFFFFFF00) : aux;
  }

  }
}

//Devuelve la direccion fisica apuntada.
int ResolverDireccion(MV* mv, int puntero, int offset) {
  printf("Puntero: %8x \n", puntero);
  int cod_seg = (puntero&0xFFFF0000) >> 16;
  printf("cod_seg: %d", cod_seg);
  int offset_puntero = puntero&0xFFFF;
  Segmento seg = mv->segmentos[cod_seg];
  printf("base: %d, size:%d", seg.base, seg.size);
  if(offset_puntero + offset >= seg.size) {
    mv->estado = ERR_SEGMENTACION;
    return 0;
  }
  return seg.base + offset_puntero + offset;
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
  case INMEDIATO: {
    int aux = operando;
    int es_negativo = operando&0x8000;
    return es_negativo?(operando|0xFFFF0000) : operando;
  }
  case REGISTRO: {
    int reg = operando & 0xF;
    int tam = (operando & 0x30) >> 4;
    return GetRegistro(mv, reg, tam);
  }

  case MEMORIA: {
    int tam_celda = (operando&0xF0000000) >>30;
    int cod_reg = (operando&0x00FF0000) >> 16;
    int offset = operando&0xFFFF;
    int err = 0;
    int dir = ResolverDireccion(mv, mv->regs[cod_reg], offset);
    int v = *((int*)(&mv->mem[dir]));
    int res;
    uint8_t *n1, *n2;
    n1 = (uint8_t *) &v;
    n2 = (uint8_t *) &res;
    if(tam_celda == 0) {
      n2[0] = n1[3];
      n2[1] = n1[2];
      n2[2] = n1[1];
      n2[3] = n1[0];
    }else if(tam_celda == 1) {
      n2[0] = n1[1];
      n2[1] = n1[0];
      res &= 0xFFFF;
    }else{
      res &= 0xFF;
    } 

    return res;
  }

  }
};

// Pone valor en el lugar que determine el operando.
// Si el tipo es memoria, pone valor en el lugar de memoria apuntado por operando,
// y si el tipo es registro, pone valor en el registro que especifica el operando.
void SetValor(MV* mv, TipoOperando tipo, int operando, int valor) {
  //printf("set valor: %d en: %x de tipo: %x", valor, operando, tipo);
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
       mv->regs[reg] &= 0xFFFFFF00;
       mv->regs[reg] |= (valor&0xFF);
       break;
      }
      case 2: {
       //3er byte
       mv->regs[reg] &= 0xFFFF00FF;
       mv->regs[reg] |= ((valor&0xFF)<<8);
       break;
      }
      case 3: {
       //dos ultimos bytes
       mv->regs[reg] &= 0xFFFF0000;
       mv->regs[reg] |= (valor&0xFFFF);
       break;
      }

    }
    break;
  }

  case MEMORIA: {
    int tam_celda = (operando&0xF0000000) >>30;
    int cod_reg = (operando&0x00FF0000) >> 16;
    int offset = operando&0xFFFF;
    int err = 0;
    int dir = ResolverDireccion(mv, mv->regs[cod_reg], offset);
    int res;
    uint8_t *n1, *n2;
    n1 = (uint8_t *) &valor;
    n2 = (uint8_t *) &res;
    if(tam_celda == 0) {
      n2[0] = n1[3];
      n2[1] = n1[2];
      n2[2] = n1[1];
      n2[3] = n1[0];
    }else if(tam_celda == 1) {
      n2[0] = n1[1];
      n2[1] = n1[0];
      res &= 0xFFFF;
    }else{
      res &= 0xFF;
    } 
    *((int*)(&mv->mem[dir])) = res;
    break;
  }

  }
};

//Actualiza el valor del registro CC dependiendo de la ultima operacion
void ActualizaCC(int value, MV* mv){
  mv->regs[CC] = 0;
  if(value < 0) {
    mv->regs[CC] |= 0x80000000;
  }
  if(value == 0) {
    mv->regs[CC] |= 0x40000000;
  }
}

int BitN(int cc)
{
    return (cc & 0x80000000);
}

int BitZ(int cc)
{
    return (cc & 0x40000000);
}

void MOV(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b) {
  SetValor(mv, tipo_a, operando_a, GetValor(mv, tipo_b, operando_b));
};

void ADD(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b) {
  int suma = GetValor(mv, tipo_a, operando_a);
  suma += GetValor(mv, tipo_b, operando_b);
  SetValor(mv, tipo_a, operando_a, suma);
  ActualizaCC(suma, mv);
};

void SUB(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b) {
  int resta = GetValor(mv, tipo_a, operando_a);
  resta -= GetValor(mv, tipo_b, operando_b);
  SetValor(mv, tipo_a, operando_a, resta);
  ActualizaCC(resta, mv);
};


void SWAP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int auxA = GetValor(mv, tipo_a, operando_a);
  int auxB = GetValor(mv, tipo_b, operando_b);
  SetValor(mv, tipo_a, operando_a, auxB);
  SetValor(mv, tipo_b, operando_b, auxA);
}


void MUL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int aux = GetValor(mv, tipo_a, operando_a);
  aux *= GetValor(mv, tipo_b, operando_b);
  SetValor(mv, tipo_a, operando_a, aux);
  ActualizaCC(aux, mv);
}

void DIV(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv, tipo_b, operando_b);
  if (opb == 0)
    mv->estado = ERR_DIV_CERO;
  else{
    int value = opa / opb;
    int resto = opa % opb;
    SetValor(mv, tipo_a, operando_a, value);
    mv->regs[AC] = resto;
    ActualizaCC(value, mv);
  }
}

void COMP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv, tipo_b, operando_b);
  ActualizaCC(opa-opb, mv);
}

void SHL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv, tipo_b, operando_b);
  int res = opa << opb;
  SetValor(mv, tipo_a, operando_a, res);
  ActualizaCC(res,mv);
}

void SHR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv, tipo_b, operando_b);
  int res = opa >> opb;
  SetValor(mv, tipo_a, operando_a, res);
  ActualizaCC(res, mv);
}

void AND(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv, tipo_b, operando_b);
  int res = opa & opb;
  SetValor(mv, tipo_a, operando_a, res);
  ActualizaCC(res, mv);
}

void OR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv, tipo_b, operando_b);
  int res = opa | opb;
  SetValor(mv, tipo_a, operando_a, res);
  ActualizaCC(res, mv);
}

void XOR(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_a, operando_a);
  int opb = GetValor(mv, tipo_b, operando_b);
  int res = opa ^ opb;
  SetValor(mv, tipo_a, operando_a, res);
  ActualizaCC(res, mv);
}

void RND(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opb = GetValor(mv, tipo_b, operando_b);
  int res = rand() % (opb + 1);
  SetValor(mv, tipo_a, operando_a, res);
}

void SYS(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int valor = GetValor(mv, tipo_b, operando_b);
  int puntero = mv->regs[EDX];
  int direccion = mv->segmentos[(puntero&0x00FF0000)>>16].base + (puntero&0xFFFF);
  int modo = GetRegistro(mv, EAX, L);
  modo = modo & 0x0F;
  int cantidad = GetRegistro(mv, ECX, L);
  int tam = GetRegistro(mv, ECX, H);
   switch(valor) {

  case 2: {
    for(int i = 0; i < cantidad; i++) {
      printf("[%04X]", direccion);
      int aux = 0;
      for(int j = 0; j < tam; j++) {
        aux |= mv->mem[direccion++];
        aux <<= 8;
      }
      aux>>=8;

      if(modo & 0x01)
        printf("# %d ", aux);
      if(modo & 0x02){
       if((aux & ~0x7f) != 0) {
            printf(".\n");
          }else{
            printf("%c ", aux);
          }

        }
       if(modo & 0x04)
         printf("@ %o ",aux);
       if(modo & 0x08)
        printf("0x%x ",aux);

      printf("\n");
    }
    break;
  }

  case 1:{
    int aux;
    for(int i = 0; i < cantidad; i++) {
      printf("[%04X]", direccion);
      switch(modo) {
        case 1:{
          scanf("%d", &aux);
          break;
        }
        case 2: {
          scanf("%c", &aux);
          break;
        }
        case 4:{
          scanf("%o", &aux);
          break;
        }
        case 8: {
          printf("%x", &aux);
          break;
        }
      }
      char* aux_ptr = (char*)&aux;
      for(int j = 0; j < tam; j++) {
        mv->mem[direccion+tam-j-1] = aux_ptr[j];
      }
      direccion += tam;
      }
    break;
  }
  case 3:{
                int cant_char;
                cant_char = (((mv->regs[ECX]) >>16) & 0x0000FFFF);
                if(cant_char==-1)
                    cant_char=99;    //si CX es -1 no se limita entonces le da el valor maximo de la cadena
                char cadena[100];
                gets(cadena);
                int i = 0;
                while(cadena[i] != 255 && i < cant_char && i<strlen(cadena)){
                    mv->mem[puntero + i] = cadena[i];
                    i++;
                }
                break;
            }
    case 4:{
                int i=0;
                while(mv->mem[puntero+i] != 0){
                    printf("%c",mv->mem[puntero+i]);
                    i++;
                }
                  //si se llega al caracter nulo se hace el salto de linea
                   printf("\n");

                break;
            }
     case 7:{
                system("cls");
              break;
     }
     case 0xF:{
                char linea='a';
                if(mv->path_vmi != NULL){  //si no se especifica archVMI se omite el breakpoint


                    GenerarVMI(mv);//generar archivo imagen

                    while(linea != '\n' && linea != 'g' && linea != 'q'){
                        fflush(stdin);
                        gets(&linea);
                    }

                   if(linea =='\n'){ // siguiente instrucción y luego volver a realizar un nuevo breakpoint
                         Instruccion instr = LeerProximaInstruccion(mv);
                         EjecutarInstruccion(mv, instr);
                         SYS(mv,INMEDIATO,INMEDIATO,0,0xF);
                            
                        }

                   
                   else
                     if(linea =='q'){ // 'q' (quit) aborta la ejecución
                        mv->estado=FINALIZADO;

                     }
                    

                }
            break;
                }
  }

}

void JMP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opb = GetValor(mv, tipo_b, operando_b);
  mv->regs[IP] = opb;
}

void JZ(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (BitZ(mv->regs[CC])) // bit cero en 1
    mv->regs[IP] = GetValor(mv, tipo_b, operando_b);
}

void JP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (!BitN(mv->regs[CC]) && !BitZ(mv->regs[CC])) // bit signo en 0 y bit cero en 0
    mv->regs[IP] = GetValor(mv, tipo_b, operando_b);
}

void JN(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (BitN(mv->regs[CC])) // bit signo en 1
    mv->regs[IP] = GetValor(mv, tipo_b, operando_b);
}

void JNZ(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (!BitZ(mv->regs[CC])) // bit cero en 0
    mv->regs[IP] = GetValor(mv, tipo_b, operando_b);
}

void JNP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (BitN(mv->regs[CC]) || BitZ(mv->regs[CC])) // no positivo
    mv->regs[IP] = GetValor(mv, tipo_b, operando_b);
}

void JNN(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  if (!BitN(mv->regs[CC]) || BitZ(mv->regs[CC])) // no negativo
    mv->regs[IP] = GetValor(mv, tipo_b, operando_b);
}

void LDL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_b, operando_b);
  mv->regs[AC] &= 0xFFFF0000;
  opa &= 0xFFFF;
  mv->regs[AC] |= opa;
}

void LDH(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_b, operando_b);
  mv->regs[AC] &= 0xFFFF;
  opa = (opa&0xFFFF) << 16;
  mv->regs[AC] |= opa;
}

void NOT(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  int opa = GetValor(mv, tipo_b, operando_b);
  int res = ~opa;
  SetValor(mv, tipo_b, operando_b, res);
  ActualizaCC(res, mv);
}

void STOP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
  mv->estado = FINALIZADO;  //se termina la ejecucion pero sin ningun error
}

//void PUSH(int op1, int op2, char type1, char type2, registro registros[], char memoria[], int TDS[], int *error,char modificador1,char modificador2){
void PUSH(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
/*  PUSH: Permite almacenar un dato en el tope de la pila. Requiere un solo operando que es el dato a
    almacenar. El mismo puede ser de cualquier tipo.

    PUSH AX ;decrementa en uno el valor de SP y almacena en la posici�n apuntada por este registro
    el valor de AX.*/
    printf("Adele!\n");
    printf("SP: %8x, SS: %8x \n", mv->regs[SP], mv->regs[SS]);
    int value;
    mv->regs[SP] -= 4;
    if(mv->regs[SP] < mv->regs[SS]){
      mv->estado = ERR_STACK_OVERFLOW;
    }
    else{
      printf("hello its me \n");
        value = GetValor(mv,tipo_b,operando_b);
              printf("i been wondering if after all this years %d \n", value);

        SetValor(mv, MEMORIA, mv->regs[SP], value);
              printf("you would like to meet \n");

    }
}
void POP(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
/*  POP: Extrae el dato del tope de pila y lo almacena en el operando (que puede ser registro, memoria o
    indirecto).

    POP [1000] ;el contenido de la celda apuntada por SP se almacena en la celda 1000 y luego el
    valor de SP se incrementa en 1.*/
    int aux,tamSS,value;
    aux=mv->regs[SS] >>16 & 0x0000FFFF;
    tamSS=(mv->segmentos[aux].size)+(mv->segmentos[aux].base);

    if(mv->regs[SP]+4>tamSS)
     mv->estado= ERR_STACK_UNDERFLOW;  //stack under
    else{

       value = GetValor(mv,MEMORIA,mv->regs[SP]);  //VER SI ESTA BIEN PASAR EL VALOR DEL SP PARA BUSCAR EN LA MEMORIA EL ULTIMO VALOR DE LA PILA
       SetValor(mv,tipo_b,operando_b, value);

       mv->regs[SP]+=4;

    }
}

void CALL(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
/*  CALL: Permite efectuar un llamado a una subrutina. Requiere un solo par�metro que es la posici�n de
    memoria a donde se desea saltar (por supuesto, puede ser un r�tulo).

    CALL PROC1 ;se salta a la instrucci�n rotulada como PROC1. Previamente se guarda en la pila la
    direcci�n memoria siguiente a esta instrucci�n (direcci�n de retorno).*/

    PUSH(mv,INMEDIATO,0,mv->regs[IP],0);
    JMP(mv,tipo_a,tipo_b,operando_a,operando_b);
}

void RET(MV* mv, TipoOperando tipo_a, TipoOperando tipo_b, int operando_a, int operando_b){
/*  RET: Permite efectuar una retorno desde una subrutina. No requiere par�metros. Extrae el valor del
    tope de la pila y efect�a un salto a dicha direcci�n.*/
    int aux = mv->regs[EAX];
    int reg = 0x0A;
    POP(mv,INMEDIATO,REGISTRO,0,reg);
    JMP(mv,INMEDIATO,REGISTRO,0,reg);
    mv->regs[EAX]=aux;
}
