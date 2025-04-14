#include "mbed.h"
#include "Salvar_Posicoes_Volume.h"
#include "TextLCD.h"

// Definindo pinos
InterruptIn encoderCLK(D3);
DigitalIn encoderDT(D4);
InterruptIn encoderBotao(D5);
TextLCD lcd(D8, D9, D4, D5, D6, D7); //rs,e,d0,d1,d2,d3

// Definindo variáveis 
volatile int encoderValor = 1;
volatile int contadorCliques = 0;
volatile bool confirmado = false;

void setupEncoder() {
  encoderCLK.rise(&encoderGiro);         // Quando o sinal CLK sobe (borda de subida), chama a função encoderGiro()
  encoderDT.mode(PullUp);                // Coloca um resistor de pull-up no pino DT (para leitura estável)
  encoderBotao.fall(&aoConfirmar);       // Quando o botão for pressionado (borda de descida), chama aoConfirmar()
  encoderBotao.mode(PullUp);             // Coloca resistor de pull-up no botão também
}

void encoderGiro() {
  if (encoderDT.read() == 0)             // Verifica a direção do giro (baseado no DT)
    contadorCliques++;                   // Sentido horário
  else
    contadorCliques--;                   // Sentido anti-horário

  if (contadorCliques >= 5) {            // Acumulou 5 cliques para a direita
    encoderValor++;                      // Aumenta o valor mostrado
    contadorCliques = 0;                 // Zera os cliques
  } else if (contadorCliques <= -5) {    // Acumulou 5 cliques para a esquerda
    encoderValor--;                      // Diminui o valor mostrado
    contadorCliques = 0;
  }
}

void aoConfirmar() {
  confirmado = true;                     // Indica que o botão foi pressionado
}

int selecionarVolumeEncoder(const char *mensagem, int valorInicial, int minValor, int maxValor) {
  encoderValor = valorInicial;          // Define valor inicial
  contadorCliques = 0;
  confirmado = false;                   // Aguarda clique do botão
  int valorAnterior = encoderValor;

  while (!confirmado) {                 // Fica nesse loop até o botão ser apertado
    if (encoderValor != valorAnterior) {
      lcd.printf("%s: %d mL\n", mensagem, encoderValor);
      valorAnterior = encoderValor;    // Atualiza o valor mostrado
    }

    if (encoderValor < minValor)
      encoderValor = minValor;
    if (encoderValor > maxValor)
      encoderValor = maxValor;

    wait_ms(150);                       // Espera um pouco pra evitar rebotes rápidos
  }

  confirmado = false;                   // Limpa a flag pro próximo uso
  return encoderValor;                  // Retorna o valor final escolhido
}

//chave seletora 


