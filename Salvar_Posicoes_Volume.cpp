// === ARQUIVO: setup_pipetagem.cpp ===
#include "JOG.h"
#include "mbed.h"
#include "IHM.h"
#include "printLCD.h"

// === ESTRUTURAS ===
struct Ponto3D {
  int x, y, z;
};

struct Tubo {
  Ponto3D pos;
  int volumeML;
};

const int MAX_TUBOS = 10;
Tubo tubos[MAX_TUBOS];
int quantidadeTubos = 0;

Ponto3D posBecker;
int volumeBeckerML = 0;


// === FLUXO DE CONFIGURAÇÃO ===
void configurarSistema() {
  setupEncoder();
  // 1. POSICIONAR BÉQUER
  printLCD("Mova a pipeta até o béquer",0);
  modoPosicionamentoManual(posBecker);

  // 3. DEFINIR QUANTIDADE DE TUBOS
  quantidadeTubos = selecionarVolumeEncoder("Quantidade de tubos:", 1, 1, MAX_TUBOS,1);

  // 4. DEFINIR POSIÇÃO E VOLUME DE CADA TUBO
  for (int i = 0; i < quantidadeTubos; i++) {
    char buf[21];
    sprintf(buf, "Tubo %d", i + 1);
    printLCD(buf, 0);  // ajusta o ‘0’ pra linha que você quiser
    printLCD("mova a pipeta até o tubo",1);
    modoPosicionamentoManual(tubos[i].pos);
    tubos[i].volumeML = selecionarVolumeEncoder("Volume para o tubo:", 1, 1, 20,0);
  }
}
