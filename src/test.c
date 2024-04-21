#include "instruccion.h"
#include "mv.h"

bool InicializarMV(MV* mv);

int main()  {
    MV mv;
    InicializarMV(&mv);
    int dir = 0;
    mv.mem[dir + 0] = 0xFB;
    mv.mem[dir + 1] = 0xFF;
    mv.mem[dir + 2] = 0xFF;
    mv.mem[dir + 3] = 0xFF;

    int val = GetValor(&mv, INMEDIATO, 0xFFFB);
    printf("%d", val);
    return 0;
}


bool InicializarMV(MV* mv) {
  uint16_t tam_codigo = 0;
  
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
  mv->ejecutando = 0;

  return true;
};
