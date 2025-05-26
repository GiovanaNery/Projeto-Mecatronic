#include "Referenciamento.h"
#include "JOG.h"
#include "mbed.h"
#include "printLCD.h"

// Definindo pinos dos sensores de fim de curso dos eixos
DigitalIn endstopX_neg(PB_13); // X: lado esquerdo (zero)
DigitalIn endstopX_pos(PB_15); // X: lado direito (máximo)
DigitalIn endstopY_neg(D6);    // Y: frente (zero)
DigitalIn endstopY_pos(D7);    // Y: trás (máximo)
DigitalIn endstopZ_neg(PC_12); // Z: topo — define zero
DigitalIn endstopZ_pos(PC_10); // Z: base — define limite inferior
extern DigitalIn botaoEmergencia;

// Variáveis globais para armazenar o curso total
int curso_total_x = 0;
int curso_total_y = 0;
int curso_total_z = 0;
int passos_X = 1000000;
int passos_Y = 1000000;
int passos_Z = 1000000;
bool reset;
// Referenciamento do EIXO Z
void referenciar_EixoZ() {
  endstopZ_pos.mode(PullUp);

  if (botaoEmergencia == 0) {
    printLCD("   Referenciamento   ", 0);
    printLCD("         do         ", 1);
    printLCD("       Eixo Z       ", 2);
  }

  // 1. Sobe até o topo (posição zero)
  while (endstopZ_pos.read() == 1 && botaoEmergencia == 0) {
    z(-1);
  }
  passos_Z = 3000;
  mover_Z(3150);
  passos_Z = 150;
  if (botaoEmergencia == 0) {
    printLCD("Fim do Referenciamento", 0);
  }
}

// Referenciando o EIXO X
void referenciar_EixoXY() {
  if (botaoEmergencia == 0) {
    printLCD("   Referenciamento   ", 0);
    printLCD("         dos         ", 1);
    printLCD("     Eixos X e Y     ", 2);
    x(0, tempo_interpolado); // reseta as rampas
    y(0, tempo_interpolado);
    reset = true;
  }
  // 1. Vai até o lado negativo
  while ((endstopX_neg.read() == 1 && botaoEmergencia == 0) ||
         (endstopY_neg.read() == 1 && botaoEmergencia == 0)) {
    bool moverX = (endstopX_neg.read() == 1);
    bool moverY = (endstopY_neg.read() == 1);

    if (moverX && moverY) {
      // Movimento interpolado quando os dois estão ativos
      x(-1, tempo_interpolado); // reduz tempo para acompanhar o interpolado
      y(-1, tempo_interpolado);
    } else {
      if (reset) {
        x(0, 0); // reseta as rampas
        y(0, 0);
        reset = false;
      }
      if (moverX) {
        x(-1, tempo);
      }
      if (moverY) {
        y(-1, tempo);
      }
    }
  }
  if (botaoEmergencia == 0) {
    passos_X = 100000; // passos criados apenas para permitir o movimento até o
                       // fim de curso virtual
    passos_Y = 100000;
    wait(0.5);
    moverInterpoladoXY(112000,
                       112000); // move para o fim de curso virtual + uma folga
    passos_X = 12000;           // define o ponto real que está
    passos_Y = 12000;

    printLCD("Fim do Refenciamento", 0);
  }
}