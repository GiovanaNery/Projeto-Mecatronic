// === ARQUIVO: setup_pipetagem.cpp ===
#include "JOG.h"
#include "mbed.h"
#include "IHM.h"
#include "TextLCD.h"

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
  //lcd.printf("=== CONFIGURACAO DO SISTEMA DE PIPETAGEM ===\n");

  // 1. POSICIONAR BÉQUER
  //lcd.printf("1) Mova a pipeta até o béquer\n");
  modoPosicionamentoManual(posBecker);

  // 2. DEFINIR VOLUME A SER COLETADO
  volumeBeckerML = selecionarVolumeEncoder("Volume a coletar", 1, 1, 20);

  // 3. DEFINIR QUANTIDADE DE TUBOS
  quantidadeTubos = selecionarVolumeEncoder("Quantidade de tubos", 1, 1, MAX_TUBOS);

  // 4. DEFINIR POSIÇÃO E VOLUME DE CADA TUBO
  for (int i = 0; i < quantidadeTubos; i++) {
    //lcd.printf("\nTubo %d: mova a pipeta até o tubo\n", i + 1);
    modoPosicionamentoManual(tubos[i].pos);
    tubos[i].volumeML = selecionarVolumeEncoder("Volume para o tubo", 1, 1, 20);
  }
}
