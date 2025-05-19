// === ARQUIVO: setup_pipetagem.cpp ===
#include "JOG.h"
#include "mbed.h"
#include "IHM.h"
#include "printLCD.h"
#include "Referenciamento.h"

extern DigitalIn botaoEmergencia;

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
  printLCD("   Mova a pipeta   ",0);
  printLCD("    ate o bequer    ",1);
  modoPosicionamentoManual();
  posBecker.x = passos_X;
  posBecker.y = passos_Y;
  posBecker.z = passos_Z;

  // 3. DEFINIR QUANTIDADE DE TUBOS
  quantidadeTubos = selecionarVolumeEncoder("Quantidade de tubos:", 1, 1, MAX_TUBOS,1);

  // 4. DEFINIR POSIÇÃO E VOLUME DE CADA TUBO
  for (int i = 0; i < quantidadeTubos; i++ && botaoEmergencia==0) {
    char buf[21];
    sprintf(buf, "Tubo %d", i + 1);
    printLCD(buf, 0);  // ajusta o ‘0’ pra linha que você quiser
    printLCD("   Mova a pipeta   ",0);
    printLCD("    ate o tubo    ",1);
    modoPosicionamentoManual();
    tubos[i].pos.x = passos_X;           // usa o contador global
    tubos[i].pos.y = passos_Y;
    tubos[i].pos.z = passos_Z;
    tubos[i].volumeML = selecionarVolumeEncoder("Volume para o tubo:", 1, 1, 20,0);
  }
}
