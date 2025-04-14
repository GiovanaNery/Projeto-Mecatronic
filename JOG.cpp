#include "mbed.h"

float tempo = 0.01;    // tempo para os eixos X e Y
float tempo_z = 0.005; // tempo para o eixo Z

// Definindo os pinos do motor de passo de cada eixo (X, Y e Z)
BusOut MOTOR_Y(PC_5, PC_6, PC_7, PC_8);
BusOut MOTOR_X(PA_10, PB_3, PB_5, PA_4);
BusOut MOTOR_Z(PA_8, PA_9, PA_10, PA_11);

// === ENTRADAS ===
AnalogIn joystickX(A0);
AnalogIn joystickY(A1);
DigitalIn botaoZmais(D6); // Pressionado = HIGH (sem pull-down)
DigitalIn botaoZmenos(D7);

// Subir
void z(float direcao) // -1(subir) , +1(descer)
{
  // subir
  if (direcao < 1) {
    for (int i = 3; i > -1; i--) {
      MOTOR_Z = 1 << i;
      wait(tempo_z);
    }
  }
  // Descer
  if (direcao > 1) {
    for (int i = 0; i < 4; i++) {
      MOTOR_Z = 1 << i;
      wait(tempo_z);
    }
  }
}

// esquerda
void x(float direcao) // -1(esquerda) , +1(direita)
// esquerda
{
  if (direcao < 1) {
    for (int i = 0; i < 4; i++) {
      MOTOR_X = 1 << i;
      wait(tempo);
    }
  }
  // Direta
  if (direcao > 1) {
    for (int i = 3; i > -1; i--) {
      MOTOR_X = 1 << i;
      wait(tempo);
    }
  }
}

void y(float direcao) // -1(frente) , +1(tras)
{
  // frente
  if (direcao < 1) {
    for (int i = 0; i < 4; i++) {
      MOTOR_Y = 1 << i;
      wait(tempo);
    }
  }
  // Tras
  if (direcao > 1) {
    for (int i = 3; i > -1; i--) {
      MOTOR_Y = 1 << i;
      wait(tempo);
    }
  }
}

// desligar as bobinas
void desliga_motor_x() {

  MOTOR_X = 0;
  wait(tempo);
}

void desliga_motor_y() {

  MOTOR_Y = 0;
  wait(tempo);
}
void desliga_motor_z() {

  MOTOR_Z = 0;
  wait(tempo);
}

// === INTERPOLAÇÃO LINEAR ENTRE DOIS PONTOS (X E Y) ===
void moverInterpoladoXY(int x0, int y0, int x1, int y1, int passos) {
  int xAtual = x0;
  int yAtual = y0;
  for (int i = 1; i <= passos; i++) {
    float t = (float)i / passos;
    int xDestino = (int)(x0 + t * (x1 - x0) + 0.5f);
    int yDestino = (int)(y0 + t * (y1 - y0) + 0.5f);

    int dx = xDestino - xAtual;
    int dy = yDestino - yAtual;

    if (dx > 0) {
      x(+1);
      xAtual++;
    } else if (dx < 0) {
      x(-1);
      xAtual--;
    }

    if (dy > 0) {
      y(+1);
      yAtual++;
    } else if (dy < 0) {
      y(-1);
      yAtual--;
    }

    wait_ms(5);
  }
}

// === POSICIONAMENTO MANUAL COM INTERPOLAÇÃO E JOYSTICK ===
extern volatile bool confirmado;

struct Ponto3D {
  int x, y, z;
};

void modoPosicionamentoManual(Ponto3D &pos) {
  const int passosInterpol = 10;
  int xDestino = pos.x;
  int yDestino = pos.y;

  while (!confirmado) {
    float xVal = joystickX.read();
    float yVal = joystickY.read();

    if (xVal > 0.6) {
      xDestino++;
      moverInterpoladoXY(pos.x, pos.y, xDestino, yDestino, passosInterpol);
      pos.x = xDestino;
    } else if (xVal < 0.4) {
      xDestino--;
      moverInterpoladoXY(pos.x, pos.y, xDestino, yDestino, passosInterpol);
      pos.x = xDestino;
    }

    if (yVal > 0.6) {
      yDestino++;
      moverInterpoladoXY(pos.x, pos.y, xDestino, yDestino, passosInterpol);
      pos.y = yDestino;
    } else if (yVal < 0.4) {
      yDestino--;
      moverInterpoladoXY(pos.x, pos.y, xDestino, yDestino, passosInterpol);
      pos.y = yDestino;
    }

    if (botaoZmais.read()) {
      z(+1);
      pos.z++;
      wait_ms(100);
    }

    if (botaoZmenos.read()) {
      z(-1);
      pos.z--;
      wait_ms(100);
    }

    //char buf[64];
    //sprintf(buf, "Posição -> X:%d Y:%d Z:%d\n", pos.x, pos.y, pos.z); //Printa a posicão X, Y, Z
    //wait_ms(100);
  }

  confirmado = false;
}
