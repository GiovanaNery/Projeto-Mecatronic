// === ARQUIVO: setup_pipetagem.cpp ===
#include "IHM.h"
#include "JOG.h"
#include "Referenciamento.h"
#include "mbed.h"
#include "printLCD.h"

extern DigitalIn botaoEmergencia;

// === ESTRUTURAS ===
struct Ponto3D {
  int x, y, z;
};

struct Tubo {
  Ponto3D pos;
  int volumeML;
};

const int MAX_TUBOS = 100;
Tubo tubos[MAX_TUBOS];
int quantidadeTubos = 0;

Ponto3D posBecker;
int volumeBeckerML = 0;

// === FLUXO DE CONFIGURAÇÃO ===
void configurarSistema() {
  setupEncoder();
  // 1. POSICIONAR BÉQUER
  printLCD("    >> Bequer <<", 0);
  printLCD("   Mova a pipeta", 1);
  printLCD("    para coleta", 2);
  modoPosicionamentoManual();
  posBecker.x = passos_X;
  posBecker.y = passos_Y;
  posBecker.z = passos_Z;

  // 3. DEFINIR QUANTIDADE DE TUBOS
  quantidadeTubos =
      selecionarVolumeEncoder("Quantidade de tubos:", 1, 1, 100, 1);

  // 4. DEFINIR POSIÇÃO E VOLUME DE CADA TUBO
  for (int i = 0; i < quantidadeTubos && botaoEmergencia == 0; i++) {
    char buf[21];
    sprintf(buf, "    >> Tubo %d <<", i + 1);
    printLCD(buf, 0);
    printLCD("   Mova a pipeta", 1);
    printLCD("  para este tubo", 2);

    modoPosicionamentoManual();

    tubos[i].pos.x = passos_X;
    tubos[i].pos.y = passos_Y;
    tubos[i].pos.z = passos_Z;

    tubos[i].volumeML = selecionarVolumeEncoder(buf, 1, 1, 100, 0);
  }
}
