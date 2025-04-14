// === ARQUIVO: setup_pipetagem.cpp ===
#include "JOG.h"
#include "mbed.h"
#include "TextLCD.h"

// Definindo pinos
InterruptIn encoderCLK(D3);
DigitalIn encoderDT(PA_10);
InterruptIn encoderBotao(PA_6);
TextLCD lcd(D8, D9, D4, D5, D6, D7); //rs,e,d0,d1,d2,d3

//Definindo variáveis
volatile int encoderValor = 1;
volatile int contadorCliques = 0;
volatile bool confirmado = false;

// === CALLBACKS DO ENCODER ===
void encoderGiro() {
  if (encoderDT.read() == 0)
    contadorCliques++;
  else
    contadorCliques--;

  if (contadorCliques >= 5) {
    encoderValor++;
    contadorCliques = 0;
  } else if (contadorCliques <= -5) {
    encoderValor--;
    contadorCliques = 0;
  }
}

void aoConfirmar() { confirmado = true; }

int selecionarVolumeEncoder(const char *mensagem, int valorInicial, int minValor, int maxValor) {
  encoderValor = valorInicial;
  contadorCliques = 0;
  confirmado = false;
  int valorAnterior = encoderValor;

  while (!confirmado) {
    if (encoderValor != valorAnterior) {
      lcd.printf("%s: %d mL\n", mensagem, encoderValor);
      valorAnterior = encoderValor;
    }

    if (encoderValor < minValor)
      encoderValor = minValor;
    if (encoderValor > maxValor)
      encoderValor = maxValor;

    wait_ms(150);
  }

  confirmado = false;
  return encoderValor;
}

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

// === SETUP DO ENCODER ===
void setupEncoder() {
  encoderCLK.rise(&encoderGiro);
  encoderDT.mode(PullUp);
  encoderBotao.fall(&aoConfirmar);
  encoderBotao.mode(PullUp);
}

// === FLUXO DE CONFIGURAÇÃO ===
void configurarSistema() {
  setupEncoder();
  // print("=== CONFIGURACAO DO SISTEMA DE PIPETAGEM ===\n");

  // 1. POSICIONAR BÉQUER
  // print("1) Mova a pipeta até o béquer\n");
  modoPosicionamentoManual(posBecker);

  // 2. DEFINIR VOLUME A SER COLETADO
  volumeBeckerML = selecionarVolumeEncoder("Volume a coletar", 1, 1, 20);

  // 3. DEFINIR QUANTIDADE DE TUBOS
  quantidadeTubos =
      selecionarVolumeEncoder("Quantidade de tubos", 1, 1, MAX_TUBOS);

  // 4. DEFINIR POSIÇÃO E VOLUME DE CADA TUBO
  for (int i = 0; i < quantidadeTubos; i++) {
    lcd.printf("\nTubo %d: mova a pipeta até o tubo\n", i + 1);
    modoPosicionamentoManual(tubos[i].pos);
    tubos[i].volumeML = selecionarVolumeEncoder("Volume para o tubo", 1, 1, 20);
  }
}
