#include "IHM.h"
#include "Referenciamento.h"
#include "mbed.h"
#include "printLCD.h"

float tempo = 0.001;   // tempo para os eixos X e Y
float tempo_z = 0.001; // tempo para o eixo Z

// Definindo os pinos do motor de passo de cada eixo (X, Y e Z)
BusOut MOTOR_Z(D10, D11, D12, A5);
Serial pc(USBTX, USBRX, 9600);
DigitalOut Enable(D3); // Pino enable de liga e desliga

// Pinos conectados ao driver do motor no eixo X
DigitalOut DIR_X(A3);  // Pino de direção (DIR)
DigitalOut STEP_X(D4); // Pino de passo (STEP)

// Pinos conectados ao driver do eixo Y
DigitalOut DIR_Y(D8);  // Direção
DigitalOut STEP_Y(D9); // Passo

// === ENTRADAS ===
AnalogIn joystickX(A5);
AnalogIn joystickY(A4);
extern DigitalIn botaoZmais; // Pressionado = HIGH (sem pull-down)
extern DigitalIn botaoZmenos;

// Criando parametros
int Z_passo = 0;

// ACIONAR MOTOR EIXO Z - MOSFET
void z(int direcao) // -1(subir) , +1(descer)
{
  // subir
  if (direcao < 0) {
    MOTOR_Z = 1 << Z_passo;
    Z_passo++;
    if (Z_passo > 3) {
      Z_passo = 0;
    }
    passos_Z++;
    wait(tempo);
  }
  // Descer
  if (direcao > 0) {
    MOTOR_Z = 1 << Z_passo;
    Z_passo--;
    if (Z_passo < 0) {
      Z_passo = 3;
    }
    passos_Z--;
    wait(tempo);
  }
}

// ACIONAR MOTOR EIXO X - DRIVER
void x(int direcao) {
  if (direcao == 0)
    return; // Sem movimento se passos = 0
  // Define a direção:
  // Direita se passos > 0  → DIR_X = 1 (horario)
  // Esquerda se passos < 0 → DIR_X = 0 (anti-horario)
  DIR_X = (direcao > 0) ? 1 : 0;
  // Garante tempo para o driver "ler" a direção
  wait_us(100);

  // Executa a quantidade absoluta de passos
  int totalPassos = abs(direcao);
  Enable = 0; // ligado
  for (int i = 0; i < totalPassos; i++) {
    STEP_X = 1;
    wait_us(100); // Pulso HIGH
    STEP_X = 0;
    wait_us(tempo * 1e6); // Tempo entre passos
                          // Conta o passo
    if (direcao > 0) {
      passos_X++;
    } else {
      passos_X--;
    }
  }
  Enable = 1; // desliga
}

// ACIONAR MOTOR EIXO Y - DRIVER
void y(int direcao) {
  if (direcao == 0)
    return;

  // Define a direção com base no sinal
  // Frente (negativo) = 0 --> anti-horario
  // Trás (positivo) = 1 --> horario
  DIR_Y = (direcao > 0) ? 1 : 0;
  wait_us(100); // Delay para estabilizar o sinal DIR

  // Executa os passos
  int totalPassos = abs(direcao);
  Enable = 0; // ligado
  for (int i = 0; i < totalPassos; i++) {
    STEP_Y = 1;
    wait_us(100); // Pulso HIGH
    STEP_Y = 0;
    wait_us(tempo * 1e6); // Tempo entre passos
    if (direcao > 0) {
      passos_Y++;
    } else {
      passos_Y--;
    }
  }
  Enable = 1; // desliga
}

// desligar as bobinas
void desliga_motor_x() {
  STEP_X = 0;
  wait_us(tempo * 1e6);
}

void desliga_motor_y() {
  STEP_Y = 0;
  wait_us(tempo * 1e6);
}

void desliga_motor_z() {
  MOTOR_Z = 0;
  wait(tempo);
}

void pararMotores() {
  STEP_X = 0;
  STEP_Y = 0;
  MOTOR_Z = 0;
}

// === INTERPOLAÇÃO LINEAR ENTRE DOIS PONTOS (X E Y) ===
// Função de interpolação XY usando os contadores atuais
void moverInterpoladoXY(int xDestino, int yDestino, int passos) {
  // posição inicial
  int xInicio = passos_X;
  int yInicio = passos_Y;

  for (int i = 1; i <= passos; i++) {
    float t = (float)i / passos;
    int xAlvo = (int)(xInicio + t * (xDestino - xInicio) + 0.5f);
    int yAlvo = (int)(yInicio + t * (yDestino - yInicio) + 0.5f);

    int dx = xAlvo - passos_X;
    int dy = yAlvo - passos_Y;

    if (dx > 0) {
      x(+1);
    } else if (dx < 0) {
      x(-1);
    }

    if (dy > 0) {
      y(+1);
    } else if (dy < 0) {
      y(-1);
    }
  }
}

// === POSICIONAMENTO MANUAL COM INTERPOLAÇÃO E JOYSTICK ===
extern volatile bool confirmado;

struct Ponto3D {
  int x, y, z;
};
float TEMPO_BASE = 0.005f; // intervalo base entre passos (s)
float DEADZONE = 0.2f;

void modoPosicionamentoManual(Ponto3D &pos) {
  const int passosInterpol = 50;
  const int passosEntrePrints = 10;
  int passosDesdeUltimoPrint = passosEntrePrints;

  while (!confirmado) {
    // leitura centrada em 0
    float xVal = joystickX.read() - 0.5f;
    float yVal = joystickY.read() - 0.5f;

    // determina direção se passar da deadzone
    int dirX = (std::fabs(xVal) > DEADZONE) ? (xVal > 0 ? +1 : -1) : 0;
    int dirY = (std::fabs(yVal) > DEADZONE) ? (yVal > 0 ? +1 : -1) : 0;

    // aciona passos nos eixos adequados
    if (dirX) {
      x(dirX);
    }
    if (dirY) {
      y(dirY);
    }

    // debug serial a cada passo
    if (dirX || dirY) {
      char buf[32];
      sprintf(buf, "Pos X=%d Y=%d", pos.x, pos.y);
      printLCD(buf, 0);
      // pc.printf("Pos X=%d Y=%d\r\n", pos.x, pos.y);
    }
  }

  // print final
  char buf[32];
  sprintf(buf, "Final X:%d Y:%d", pos.x, pos.y);
  printLCD(buf, 0);
}