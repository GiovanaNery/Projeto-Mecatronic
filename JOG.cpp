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
AnalogIn joystickX(A1);
AnalogIn joystickY(A0);
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
    return; // Sem movimento se direção = 0

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
    wait_us(int(tempo * 1e6f)); // intervalo entre pulsos
    // Atualiza contador de passos
    passos_X += (direcao > 0) ? +1 : -1;
  }
}

// Função de acionamento do motor no eixo Y
void y(int direcao) {
  if (direcao == 0)
    return; // Sem movimento se direção = 0

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
    wait_us(int(tempo * 1e6f)); // intervalo entre pulsos
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
    Enable = 0;
    int passos = 150;
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
  Enable = 1;
}

// === POSICIONAMENTO MANUAL COM INTERPOLAÇÃO E JOYSTICK ===
extern volatile bool confirmado;
extern DigitalIn endstopX_pos; // ativo em 0 quando bate no limite direito
extern DigitalIn endstopY_pos; // ativo em 0 quando bate no limite traseiro

struct Ponto3D {
  int x, y, z;
};
float TEMPO_BASE = 0.005f; // intervalo base entre passos (s)
float DEADZONE = 0.2f;

// Modo de posicionamento manual
void modoPosicionamentoManual(Ponto3D &pos) {
  Enable = 0; // habilita o driver

  while (!confirmado) {
    // 1) lê joystick
    float xVal = joystickX.read() - 0.5f;
    float yVal = joystickY.read() - 0.5f;

    // 2) determina dirX/dirY
    int dirX = (fabs(xVal) > DEADZONE) ? (xVal > 0 ? +1 : -1) : 0;
    int dirY = (fabs(yVal) > DEADZONE) ? (yVal > 0 ? +1 : -1) : 0;

    // 3) bloqueia os limites ZERO
    if (dirX < 0 && pos.x <= 0)
      dirX = 0;
    if (dirY < 0 && pos.y <= 0)
      dirY = 0;

    // 4) bloqueia os limites MÁXIMOS (fim de curso pos)
    if (dirX > 0 && endstopX_pos.read() == 0)
      dirX = 0;
    if (dirY > 0 && endstopY_pos.read() == 0)
      dirY = 0;

    // 5) executa os movimentos válidos
    if (dirX) {
      x(dirX);
      pos.x += dirX;
    }
    if (dirY) {
      y(dirY);
      pos.y += dirY;
    }
  }
  // imprime no LCD a cada N passos, para não travar o loop
  // if (dirX || dirY) {
  // if (--passosDesdeUltimoPrint <= 0) {
  // char buf[32];
  // sprintf(buf, "Pos X=%d Y=%d", pos.x, pos.y);
  // printLCD(buf, 0);
  // passosDesdeUltimoPrint = passosEntrePrints;
  //}
  //}

  Enable = 1; // desabilita o driver

  // mostra posição final
  char buf[32];
  sprintf(buf, "Final X:%d Y:%d", pos.x, pos.y);
  printLCD(buf, 0);
}
