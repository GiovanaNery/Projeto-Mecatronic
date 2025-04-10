#include "mbed.h"
#include "JOG.h"

// Definindo pinos dos sensores de fim de curso dos eixos
DigitalIn endstopX_neg(PA_0); // X: lado esquerdo (zero)
DigitalIn endstopX_pos(PA_1); // X: lado direito (máximo)
DigitalIn endstopY_neg(PA_2); // Y: frente (zero)
DigitalIn endstopY_pos(PA_3); // Y: trás (máximo)
DigitalIn endstopZ_neg(PA_4); // Z: topo — define zero
DigitalIn endstopZ_pos(PA_5); // Z: base — define limite inferior

// Variáveis globais para armazenar o curso total
int curso_total_x = 0;
int curso_total_y = 0;
int curso_total_z = 0;

// Referenciamento do EIXO Z
void referenciar_EixoZ() {
  endstopZ_neg.mode(PullUp);
  endstopZ_pos.mode(PullUp);

  printf("Início do referenciamento do eixo Z...\n");

  // 1. Sobe até o topo (posição zero)
  while (endstopZ_neg.read() == 1) {
    z(-1);
  }

  // 2. Conta passos até o fim de curso inferior (base)
  int passos = 0;
  while (endstopZ_pos.read() == 1) {
    z(1);
    passos++;
  }
  curso_total_z = passos;

  // 3. Voltar ao zero (subir até o topo novamente)
  while (endstopZ_neg.read() == 1) {
    z(-1);
  }

  printf("Fim do referenciamento do eixo Z.\n");
}

// Referenciando o EIXO X
void referenciar_EixoX() {
  endstopX_neg.mode(PullUp);
  endstopX_pos.mode(PullUp);

  printf("Início do referenciamento do eixo X...\n");

  // 1. Vai até o lado negativo
  while (endstopX_neg.read() == 1) {
    x(-1); // move até o fim negativo
  }

  // 2. Conta os passos até o lado positivo
  int passos = 0;
  while (endstopX_pos.read() == 1) {
    x(1); // move até o fim positivo
    passos++;
  }
  curso_total_x = passos;

  printf("Fim do referenciamento do eixo X.\n");
}

// Referenciamento do EIXO Y
void referenciar_EixoY() {
  endstopY_neg.mode(PullUp);
  endstopY_pos.mode(PullUp);

  printf("Início do referenciamento do eixo Y...\n");

  // 1. Vai até o lado negativo
  while (endstopY_neg.read() == 1) {
    y(-1); // move até o fim negativo
  }

  // 2. Conta os passos até o lado positivo
  int passos = 0;
  while (endstopY_pos.read() == 1) {
    y(1); // move até o fim positivo
    passos++;
  }
  curso_total_y = passos;

  printf("Fim do referenciamento do eixo Y.\n");
}

