#include "IHM.h"
#include "Referenciamento.h"
#include "mbed.h"
#include "printLCD.h"
#include <math.h>
#define PI 3.14159265f
#define T_BASE_MS 2.0f

float tempo = 0.001;
float tempo_interpolado = 0.001 / 2.0; // tempo para os eixos X e Y
//float tempo_z = 0.001;                 // tempo para o eixo Z

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
AnalogIn joystickX(A1);
AnalogIn joystickY(A0);
extern DigitalIn botaoZmais; // Pressionado = HIGH (sem pull-down)
extern DigitalIn botaoZmenos;
extern DigitalIn seletor;

// Criando parametros
int Z_passo = 0;

// ACIONAR MOTOR EIXO Z - MOSFET
void z(int direcao, float velocidade) // -1(subir) , +1(descer)
{
  // subir
  if (direcao < 0) {
    MOTOR_Z = 1 << Z_passo;
    Z_passo++;
    if (Z_passo > 3) {
      Z_passo = 0;
    }
    passos_Z++;
    wait_us(int(velocidade * 1e6f));
  }
  // Descer
  if (direcao > 0) {
    MOTOR_Z = 1 << Z_passo;
    Z_passo--;
    if (Z_passo < 0) {
      Z_passo = 3;
    }
    passos_Z--;
    wait_us(int(velocidade * 1e6f));
  }
}

// ACIONAR MOTOR EIXO X - DRIVER
void x(int direcao, float velocidade) {
// void x(int direcao)
  if (direcao == 0)
    return; // Sem movimento se direção = 0

 // escolhe a velocidade pela chave
    //float velocidade;
   // chaveseletora(velocidade);

  // Define a direção (1 = horário, 0 = anti-horário)
  DIR_X = (direcao > 0) ? 1 : 0;
  // Tempo para o driver captar a direção
  wait_us(100);

  int totalPassos = abs(direcao);
  // Executa todos os pulsos sem ficar re-habilitando o driver
  for (int i = 0; i < totalPassos; i++) {
    STEP_X = 1;
    wait_us(100); // largura do pulso
    STEP_X = 0;
    wait_us(int(velocidade * 1e6f)); // intervalo entre pulsos
    // Atualiza contador de passos
    passos_X += (direcao > 0) ? +1 : -1;
  }
}

// Função de acionamento do motor no eixo Y
void y(int direcao, float velocidade) {
    //void y(int direcao)
  if (direcao == 0)
    return; // Sem movimento se direção = 0

    //float velocidade;
    //chaveseletora(velocidade);

  // Define a direção (1 = horário, 0 = anti-horário)
  DIR_Y = (direcao > 0) ? 1 : 0;
  // Tempo para o driver captar a direção
  wait_us(100);

  int totalPassos = abs(direcao);
  // Executa todos os pulsos sem re-habilitar o driver
  for (int i = 0; i < totalPassos; i++) {
    STEP_Y = 1;
    wait_us(100); // largura do pulso
    STEP_Y = 0;
    wait_us(int(velocidade * 1e6f)); // intervalo entre pulsos
    // Atualiza contador de passos
    passos_Y += (direcao > 0) ? +1 : -1;
  }
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
void moverInterpoladoXY(int xDestino, int yDestino) {
  // captura posição inicial
  int xInicio = passos_X;
  int yInicio = passos_Y;
  int deltaX = xDestino - xInicio;
  int deltaY = yDestino - yInicio;
  int passos = max(abs(deltaX), abs(deltaY));

  for (int i = 1; i <= passos; i++) {
    // calcula o ponto interpolado na reta (hipotenusa)
    float t = float(i) / passos;
    int xAlvo = int(xInicio + t * deltaX + 0.5f);
    int yAlvo = int(yInicio + t * deltaY + 0.5f);

    // quantos passos faltam em cada eixo
    int dx = (xAlvo > passos_X) ? +1 : (xAlvo < passos_X) ? -1 : 0;
    int dy = (yAlvo > passos_Y) ? +1 : (yAlvo < passos_Y) ? -1 : 0;

    // seleciona delay; em diagonal, metade do tempo para cada eixo
    float vel = (dx && dy) ? (tempo / 2.0f) : tempo;

    // se for mover no X, habilita driver, dá o pulso e desabilita
    if (dx) {
      Enable = 0; // ativa driver
      x(dx, vel); // um pulso em X
      Enable = 1; // desativa driver
    }
    // se for mover no Y, mesmo esquema
    if (dy) {
      Enable = 0; // ativa driver
      y(dy, vel); // um pulso em Y
      Enable = 1; // desativa driver
    }
  }

  // garante driver desligado ao final
  Enable = 1;
}

// === POSICIONAMENTO MANUAL COM INTERPOLAÇÃO E JOYSTICK ===
extern volatile bool confirmado;
extern DigitalIn endstopX_pos; // ativo em 0 quando bate no limite direito
extern DigitalIn endstopY_pos; // ativo em 0 quando bate no limite traseiro
extern DigitalIn endstopX_neg;
extern DigitalIn endstopY_neg;

struct Ponto3D {
  int x, y, z;
};
float TEMPO_BASE = 0.005f; // intervalo base entre passos (s)
float DEADZONE = 0.2f;

// Modo de posicionamento manual
void modoPosicionamentoManual() {
  Enable = 0;         // habilita o driver
  confirmado = false; // reseta o flag antes de entrar no loop

  while (!confirmado) {
    chaveseletora();
    // 1) lê joystick (centra em zero)
    float xVal = joystickX.read() - 0.5f;
    float yVal = joystickY.read() - 0.5f;

    // 2) define direção só se passar da deadzone
    int dirX = (fabs(xVal) > DEADZONE) ? (xVal > 0 ? +1 : -1) : 0;
    int dirY = (fabs(yVal) > DEADZONE) ? (yVal > 0 ? +1 : -1) : 0;

    // 3) trava os limites em zero (sensor de fim curso negativo)
    if (dirX < 0 && endstopX_neg.read() == 0)
      dirX = 0;
    if (dirY < 0 && endstopY_neg.read() == 0)
      dirY = 0;

    // 4) trava os limites no fim de curso positivo
    if (dirX > 0 && endstopX_pos.read() == 0)
      dirX = 0;
    if (dirY > 0 && endstopY_pos.read() == 0)
      dirY = 0;

    // 5) calcula a “velocidade” a usar:
    //    - se mexer nos dois eixos ao mesmo tempo, divide por 2
    float vel = (dirX && dirY) ? (velocidade_jog / 2.0f) : velocidade_jog;

    // 6) dispara os pulsos
    if (dirX)
      x(dirX, vel);
    if (dirY)
      y(dirY, vel);

    else if (botaoZmais == 1) {
      z(1, velocidade_jog);
    } else if (botaoZmenos == 1) {
      z(-1, velocidade_jog);
    }
  }

  Enable = 1; // desabilita o driver

  // exibe posição final no LCD
  char buf[32];
  sprintf(buf, "Final X:%d Y:%d", passos_X, passos_Y);
  printLCD(buf, 0);
}
