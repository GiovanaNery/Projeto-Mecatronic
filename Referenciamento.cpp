#include "Referenciamento.h"
#include "mbed.h"
#include "JOG.h"
#include "printLCD.h"

// Definindo pinos dos sensores de fim de curso dos eixos
DigitalIn endstopX_neg(PB_13); // X: lado esquerdo (zero)
DigitalIn endstopX_pos(PB_15); // X: lado direito (máximo)
DigitalIn endstopY_neg(D6); // Y: frente (zero)
DigitalIn endstopY_pos(D7); // Y: trás (máximo)
DigitalIn endstopZ_neg(PA_4); // Z: topo — define zero
DigitalIn endstopZ_pos(PA_5); // Z: base — define limite inferior

// Variáveis globais para armazenar o curso total
int curso_total_x = 0;
int curso_total_y = 0;
int curso_total_z = 0;
int passos_X = 0;
int passos_Y = 0;
int passos_Z = 0;

// Referenciamento do EIXO Z
void referenciar_EixoZ() {
  endstopZ_neg.mode(PullUp);
  printLCD("   referenciamento   ", 0);
  printLCD("         do         ", 1);
  printLCD("       Eixo Z       ", 2);

  // 1. Sobe até o topo (posição zero)
  while (endstopZ_neg.read() == 1) {
    z(-1);
  }
  passos_Z = 0;
  printLCD("Fim", 0);
}

// Referenciando o EIXO X
void referenciar_EixoX() {
  endstopX_neg.mode(PullUp);

  printLCD("   referenciamento   ", 0);
  printLCD("         do         ", 1);
  printLCD("       Eixo X       ", 2);

  // 1. Vai até o lado negativo
  while (endstopX_neg.read() == 1) {
    x(-1); // move até o fim negativo
  }
  passos_X = 0;

  printLCD("Fim",0);

}

// Referenciamento do EIXO Y
void referenciar_EixoY() {
  endstopY_neg.mode(PullUp);

  printLCD("   referenciamento   ", 0);
  printLCD("         do         ", 1);
  printLCD("       Eixo Y       ", 2);

  // 1. Vai até o lado negativo
  while (endstopY_neg.read() == 1) {
    y(-1); // move até o fim negativo
  }
  passos_Y = 0;
  printLCD("Fim", 0);
}

