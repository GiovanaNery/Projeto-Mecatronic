#include "JOG.h"

float tempo = 0.01;   // tempo para os eixos X e Y
float tempo_z = 0.005; // tempo para o eixo Z

// Definindo os pinos do motor de passo de cada eixo (X, Y e Z)
BusOut MOTOR_Y(PC_5, PC_6, PC_7, PC_8);
BusOut MOTOR_X(PB_1, PB_13, PB_14, PB_15);
BusOut MOTOR_Z(PA_8, PA_9, PA_10, PA_11);
// === ENTRADAS ===
AnalogIn joystickX(A0);
AnalogIn joystickY(A1);
DigitalIn botaoZmais(D6);   // Pressionado = HIGH (sem pull-down)
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

void modoPosicionamentoManual(Ponto3D& pos) {
    print("\n=== POSICIONAMENTO MANUAL ===\n");
    print("Use o joystick para mover em X/Y.\n");
    print("Use os botões para subir/descer Z.\n");
    print("Pressione o botão do encoder para salvar.\n");

    while (!confirmado) {
        float xVal = joystickX.read();
        float yVal = joystickY.read();

        const float deadzone = 0.1;

        if (xVal > 0.6) { x(1); pos.x++; }
        else if (xVal < 0.4) { x(-1); pos.x--; }

        if (yVal > 0.6) { y(1); pos.y++;}
        else if (yVal < 0.4) { y(-1); pos.y--; }

        if (botaoZmais.read()) { z(1); pos.z++; }
        if (botaoZmenos.read()) { z(-1); pos.z--; }

        char buf[64];
        sprintf(buf, "Posição -> X:%d Y:%d Z:%d\n", pos.x, pos.y, pos.z);
        print(buf);

        wait(100);
    }

    confirmado = false; // limpa flag, mas não imprime nada
}

