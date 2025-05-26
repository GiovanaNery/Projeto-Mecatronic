#include "Salvar_Posicoes_Volume.h"
#include "mbed.h"
#include "printLCD.h"

// Pinos para os LEDs
DigitalOut ledVermelho(A2);
DigitalOut ledAmarelo(A4);
DigitalOut ledVerde(A5);
// Pino fixo da chave seletora
DigitalIn seletor(PC_4);
// Pinos dos botões
DigitalIn botaoZmais(PC_2);  // z no sentido positivo
DigitalIn botaoZmenos(PC_3); // z no sentido negativo
// Pinos do motor - drivers
DigitalOut passoZ(D6);
DigitalOut direcaoZ(D5);
// PINOS DO ENCODER
InterruptIn encoderCLK(PB_12);   // sinal A
DigitalIn encoderDT(PB_11);      // sinal B
InterruptIn encoderBotao(PA_11); // botão (leitura manual)
extern DigitalIn botaoEmergencia;

// === VARIÁVEIS ===
volatile int encoderValor = 1;
volatile int contadorCliques = 0;
volatile bool confirmado = false;
int valorAnterior = encoderValor;
bool ultimoEstadoBotao = 1; // botão não pressionado

void encoderGiro() {
  // agora, se CLK ≠ DT (sentido horário) a gente incrementa
  if (encoderCLK.read() != encoderDT.read()) {
    contadorCliques++;
  } else {
    contadorCliques--;
  }

  if (contadorCliques >= 2) {
    encoderValor++;
    contadorCliques = 0;
  } else if (contadorCliques <= -2) {
    encoderValor--;
    contadorCliques = 0;
  }
}

// === Função de confirmação via botão ===
void aoConfirmar() { confirmado = true; }

// === Função principal de seleção com LCD ===
int selecionarVolumeEncoder(const char *mensagem, int valorInicial,
                            int minValor, int maxValor, int ind) {
  encoderValor = valorInicial; // começa com 1
  contadorCliques = 0;
  confirmado = false;
  int valorAnterior = -1; // força print inicial mesmo se começar em 1

  printLCD(mensagem, 0); // mensagem na linha 0

  while (!confirmado && botaoEmergencia == 0) {
    // aplica os limites
    if (encoderValor < minValor)
      encoderValor = minValor;
    else if (encoderValor > maxValor)
      encoderValor = maxValor;

    if (encoderValor != valorAnterior) {
      char buffer[20];
      if (ind == 0) {
        sprintf(buffer, "Volume: %d mL ", encoderValor);
      } else {
        sprintf(buffer, "%d ", encoderValor);
      }
      printLCD(buffer, 1); // exibe o valor
      valorAnterior = encoderValor;
    }

    wait_ms(10); // debounce
  }

  confirmado = false;
  return encoderValor;
}

// === Setup inicial ===
void setupEncoder() {
  encoderDT.mode(PullUp);
  encoderBotao.mode(PullUp);
  encoderCLK.fall(&encoderGiro);   // gira
  encoderBotao.fall(&aoConfirmar); // botão confirma
}
bool confirmado_5seg = false;

void verificarPressionamentoLongo() {
  const int tempoPressionado_ms = 5000;  // 5 segundos
  int tempoPressionado = 0;

  if (encoderBotao.read() == 0) {  // Botão pressionado (LOW, assumindo PullUp)
    while (encoderBotao.read() == 0) {
      wait_ms(10);
      tempoPressionado += 10;

      if (tempoPressionado >= tempoPressionado_ms) {
        confirmado_5seg = true;
        break;
      }
    }
  }
}

// chave seletora - para definir a velocidade que quero usar
extern float velocidade_jog;
void chaveseletora() {
  if (seletor == 1) {
      velocidade_jog = (0.001/16.0); //devagar
  } else {
      velocidade_jog = (0.0004/16.0); // Rápido
  }
}

// acender LEDS por cor
void acenderLed(
    char cor) { // acender apenas indicando apenas a incial correspondente a cor
  // Apaga todos
  ledVermelho = 0;
  ledVerde = 0;
  ledAmarelo = 0;

  // Acende o LED escolhido
  if (cor == 'r') {
    ledVermelho = 1;
  } else if (cor == 'g') {
    ledVerde = 1;
  } else if (cor == 'y') {
    ledAmarelo = 1;
  }
}
