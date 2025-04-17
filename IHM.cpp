#include "Salvar_Posicoes_Volume.h"
#include "TextLCD.h"
#include "mbed.h"

// Definindo pinos
InterruptIn encoderCLK(D3);
DigitalIn encoderDT(D4);
InterruptIn encoderBotao(D5);
// Pinos para os LEDs
DigitalOut ledVermelho(D2);
DigitalOut ledVerde(D3);
DigitalOut ledAmarelo(D4);
// Pino fixo da chave seletora
DigitalIn seletor(D5);

// Definindo variáveis
volatile int encoderValor = 1;
volatile int contadorCliques = 0;
volatile bool confirmado = false;

void setupEncoder() {
  encoderCLK.rise(&encoderGiro); // Quando o sinal CLK sobe (borda de subida),
                                 // chama a função encoderGiro()
  encoderDT.mode(PullUp); // Coloca um resistor de pull-up no pino DT (para
                          // leitura estável)
  encoderBotao.fall(&aoConfirmar); // Quando o botão for pressionado (borda de
                                   // descida), chama aoConfirmar()
  encoderBotao.mode(PullUp); // Coloca resistor de pull-up no botão também
}

void encoderGiro() {
  if (encoderDT.read() == 0) // Verifica a direção do giro (baseado no DT)
    contadorCliques++;       // Sentido horário
  else
    contadorCliques--; // Sentido anti-horário

  if (contadorCliques >= 5) {         // Acumulou 5 cliques para a direita
    encoderValor++;                   // Aumenta o valor mostrado
    contadorCliques = 0;              // Zera os cliques
  } else if (contadorCliques <= -5) { // Acumulou 5 cliques para a esquerda
    encoderValor--;                   // Diminui o valor mostrado
    contadorCliques = 0;
  }
}

void aoConfirmar() {
  confirmado = true; // Indica que o botão foi pressionado
}

int selecionarVolumeEncoder(const char *mensagem, int valorInicial,
                            int minValor, int maxValor) {
  encoderValor = valorInicial; // Define valor inicial
  contadorCliques = 0;
  confirmado = false; // Aguarda clique do botão
  int valorAnterior = encoderValor;

  while (!confirmado) { // Fica nesse loop até o botão ser apertado
    if (encoderValor != valorAnterior) {
      lcd.printf("%s: %d mL\n", mensagem, encoderValor);
      valorAnterior = encoderValor; // Atualiza o valor mostrado
    }

    if (encoderValor < minValor)
      encoderValor = minValor;
    if (encoderValor > maxValor)
      encoderValor = maxValor;

    wait_ms(150); // Espera um pouco pra evitar rebotes rápidos
  }

  confirmado = false;  // Limpa a flag pro próximo uso
  return encoderValor; // Retorna o valor final escolhido
}

// chave seletora - para definir a velocidade que quero usar 
void chaveseletora(float &tempo) {
    if (seletor == 0) {
        tempo = 0.01;  // Devagar
    } else {
        tempo = 0.002; // Rápido
    }
}

// acender LEDS por cor 
void acenderLed(char cor) {  //acender apenas indicando apenas a incial correspondente a cor
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
void piscarLed(char cor, bool& parar) {
    DigitalOut* led = NULL;

    if (cor == 'r') led = &ledVermelho;
    else if (cor == 'g') led = &ledVerde;
    else if (cor == 'y') led = &ledAmarelo;

    if (led != NULL) { //led liga e desliga de 0.3 em 0.3
        while (!parar) {
            *led = 1;  //*led acende o valor apontado, ou seja, o LED que foi escolhido com base no char cor.
            wait(0.3);       // LED ligado 
            *led = 0;
            wait(0.3);       // LED desligado
        }
        *led = 0; // Garante que o LED fique apagado no final
    }
}
//COMO USAR A FUNCAO:
//pararPiscar = false;
//piscarLed('r', pararPiscar); // Enquanto pisca led vermelho, outra lógica pode rodar em paralelo
// Quando quiser parar:
//pararPiscar = true;
