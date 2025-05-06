#include "Salvar_Posicoes_Volume.h"
#include "mbed.h"
#include "printLCD.h"

// Pinos para os LEDs
DigitalOut ledVermelho(A2);
DigitalOut ledAmarelo(A4);
DigitalOut ledVerde(A5);
// Pino fixo da chave seletora
DigitalIn seletor(D5);
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

// === VARIÁVEIS ===
volatile int encoderValor = 1;
volatile int contadorCliques = 0;
volatile bool confirmado = false;
int valorAnterior = encoderValor;
bool ultimoEstadoBotao = 1; // botão não pressionado

// === Função de leitura de giro ===
void encoderGiro() {
  if (encoderCLK.read() == encoderDT.read()) {
    contadorCliques++;
  } else {
    contadorCliques--;
  }

  if (contadorCliques >= 5) {
    encoderValor++;
    contadorCliques = 0;
  } else if (contadorCliques <= -5) {
    encoderValor--;
    contadorCliques = 0;
  }
}

// === Função de confirmação via botão ===
void aoConfirmar() { confirmado = true; }

// === Função principal de seleção com LCD ===
int selecionarVolumeEncoder(const char *mensagem, int valorInicial,
                            int minValor, int maxValor, int ind) {
  // ind = 0 → seleciona volume (exibe “Volume: X mL”)
  // ind = 1 → apenas exibe o número
  encoderValor = valorInicial;
  contadorCliques = 0;
  confirmado = false;
  int valorAnterior = encoderValor;

  printLCD(mensagem, 0); // mensagem na linha 0

  while (!confirmado) {
    // limita entre minValor e maxValor
    if (encoderValor < minValor)
      encoderValor = minValor;
    else if (encoderValor > maxValor)
      encoderValor = maxValor;

    if (encoderValor != valorAnterior) {
      char buffer[20];
      if (ind == 0) {
        sprintf(buffer, "Volume: %d mL", encoderValor);
      } else {
        sprintf(buffer, "%d", encoderValor);
      }
      printLCD(buffer, 1); // valor na linha 1
      valorAnterior = encoderValor;
    }

    wait_ms(10); // debounce suave
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

// chave seletora - para definir a velocidade que quero usar
extern float velocidade_jog;
void chaveseletora() {
  if (seletor == 1) {
      velocidade_jog = 0.01; //devagar
  } else {
      velocidade_jog = 0.001; // Rápido
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

// piscar LEDS por cor
void piscarLed(char cor, bool &parar) {
  DigitalOut *led = NULL;

  if (cor == 'r')
    led = &ledVermelho;
  else if (cor == 'g')
    led = &ledVerde;
  else if (cor == 'y')
    led = &ledAmarelo;

  if (led != NULL) { // led liga e desliga de 0.3 em 0.3
    while (!parar) {
      *led = 1; //*led acende o valor apontado, ou seja, o LED que foi escolhido
                // com base no char cor.
      wait(0.3); // LED ligado
      *led = 0;
      wait(0.3); // LED desligado
    }
    *led = 0; // Garante que o LED fique apagado no final
  }
}
// COMO USAR A FUNCAO:
// pararPiscar = false;
// piscarLed('r', pararPiscar); // Enquanto pisca led vermelho, outra lógica
// pode rodar em paralelo
//  Quando quiser parar:
// pararPiscar = true;