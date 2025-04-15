#include "TextLCD.h"
#include "mbed.h"

float tempo = 0.002;   // tempo para os eixos X e Y
float tempo_z = 0.001; // tempo para o eixo Z

// Definindo os pinos do motor de passo de cada eixo (X, Y e Z)
BusOut MOTOR_Y(D14, D15, D8, D9);
BusOut MOTOR_X(D2, D3, D4, D5);
BusOut MOTOR_Z(D10, D11, D12, A5);
Serial pc(USBTX, USBRX, 9600);

// === ENTRADAS ===
AnalogIn joystickX(A0);
AnalogIn joystickY(A1);
DigitalIn botaoZmais(D6); // Pressionado = HIGH (sem pull-down)
DigitalIn botaoZmenos(D7);
// TextLCD lcd(D8, D9, D4, D5, D6, D7); //rs,e,d0,d1,d2,d3

// Criando parametros
int j;
int Y_passo = 0;
int X_passo = 0;
int Z_passo = 0;

// Subir
void z(int direcao) // -1(subir) , +1(descer)
{
  // subir
  if (direcao < 0) {
    MOTOR_Z = 1 << Z_passo;
    Z_passo++;
    if (Z_passo > 3) {
      Z_passo = 0;
    }
    wait(tempo); 
  }
  // Descer
  if (direcao > 0) {
    MOTOR_Z = 1 << Z_passo;
    Z_passo--;
    if (Z_passo < 0) {
      Z_passo = 3;
    }
    wait(tempo);
  }
}

// esquerda
void x(int direcao) // -1(esquerda) , +1(direita)
// esquerda
{
  // esquerda
  if (direcao < 0) {
    MOTOR_X = 1 << X_passo;
    X_passo++;
    if (X_passo > 3) {
      X_passo = 0;
    }
    wait(tempo);
  }

  // Direta
  if (direcao > 0) {
    MOTOR_X = 1 << X_passo;
    X_passo--;
    if (X_passo < 0) {
      X_passo = 3;
    }
    wait(tempo);
  }
}

void y(int direcao) // -1(frente) , +1(tras)
{
  // frente
  if (direcao < 0) {
    MOTOR_Y = 1 << Y_passo;
    Y_passo++;
    if (Y_passo > 3) {
      Y_passo = 0;
    }
    wait(tempo);
  }
  // Tras
  if (direcao > 0) {
    MOTOR_Y = 1 << Y_passo;
    Y_passo--;
    if (Y_passo < 0) {
      Y_passo = 3;
    }
    wait(tempo);
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

// Moimentacao pelo JoyStrick
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

    // pc.printf("Posição -> X:%d Y:%d Z:%d\n", pos.x, pos.y, pos.z);
    // lcd.printf("Posição -> X:%d Y:%d Z:%d\n", pos.x, pos.y, pos.z); //Printa
    // a posicão X, Y, Z
    wait_ms(100);
  }

  confirmado = false;
}
