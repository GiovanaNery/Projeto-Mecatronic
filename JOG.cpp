//#include "TextLCD.h"
#include "mbed.h"
#include "printLCD.h"

float tempo = 0.0005;   // tempo para os eixos X e Y
float tempo_z = 0.001; // tempo para o eixo Z

// Definindo os pinos do motor de passo de cada eixo (X, Y e Z)
BusOut MOTOR_Z(D10, D11, D12, A5);
Serial pc(USBTX, USBRX, 9600);

// Pinos conectados ao driver do motor no eixo X
DigitalOut DIR_X(D2);   // Pino de direção (DIR)
DigitalOut STEP_X(D4);  // Pino de passo (STEP)

// Pinos conectados ao driver do eixo Y
DigitalOut DIR_Y(D6);   // Direção
DigitalOut STEP_Y(D7);  // Passo

// === ENTRADAS ===
AnalogIn joystickX(A5);
AnalogIn joystickY(A4);
DigitalIn botaoZmais(D6); // Pressionado = HIGH (sem pull-down)
DigitalIn botaoZmenos(D7);

// Criando parametros
int j;
int Y_passo = 0;
int X_passo = 0;
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

// ACIONAR MOTOR EIXO X - DRIVER
void x(int direcao) {
    if (direcao == 0) return;  // Sem movimento se passos = 0
    // Define a direção:
    // - Direita se passos > 0  → DIR_X = 1 (horario)
    // - Esquerda se passos < 0 → DIR_X = 0 (anti-horario)
    DIR_X = (direcao > 0) ? 1 : 0;

    // Garante tempo para o driver "ler" a direção
    wait_us(100);

    // Executa a quantidade absoluta de passos
    int totalPassos = abs(direcao);
    for (int i = 0; i < totalPassos; i++) {
        STEP_X = 1;
        wait_us(100);      // Pulso HIGH
        STEP_X = 0;
        wait_us(tempo * 1e6);  // Tempo entre passos
    }
}

// ACIONAR MOTOR EIXO Y - DRIVER
void y(int direcao) {
    if (direcao == 0) return;

    // Define a direção com base no sinal
    // Frente (negativo) = 0 --> anti-horario
    // Trás (positivo) = 1 --> horario
    DIR_Y = (direcao > 0) ? 1 : 0;
    wait_us(100);  // Delay para estabilizar o sinal DIR

    // Executa os passos
    int totalPassos = abs(direcao);
    for (int i = 0; i < totalPassos; i++) {
        STEP_Y = 1;
        wait_us(100);  // Pulso HIGH
        STEP_Y = 0;
        wait_us(tempo * 1e6);  // Tempo entre passos
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
  }
}

// === POSICIONAMENTO MANUAL COM INTERPOLAÇÃO E JOYSTICK ===
// === POSICIONAMENTO MANUAL COM INTERPOLAÇÃO E JOYSTICK CORRIGIDO ===
extern volatile bool confirmado;

struct Ponto3D {
    int x, y, z;
};

void modoPosicionamentoManual(Ponto3D &pos) {
    const int passosInterpol = 50;
    const int passosEntrePrints = 10;  // a cada 10 passos, atualiza o LCD
    int xDestino = pos.x;
    int yDestino = pos.y;

    int passosDesdeUltimoPrint = 15;

    while (!confirmado) {
        float xVal = joystickX.read();
        float yVal = joystickY.read();
        bool movimentou = false;

        // === EIXO X ===
        if (xVal > 0.6f) {
            xDestino++;
            moverInterpoladoXY(pos.x, pos.y, xDestino, pos.y, passosInterpol);
            pos.x = xDestino;
            movimentou = true;
        } else if (xVal < 0.4f) {
            xDestino--;
            moverInterpoladoXY(pos.x, pos.y, xDestino, pos.y, passosInterpol);
            pos.x = xDestino;
            movimentou = true;
        }

        // === EIXO Y ===
        if (yVal > 0.6f) {
            yDestino++;
            moverInterpoladoXY(pos.x, pos.y, pos.x, yDestino, passosInterpol);
            pos.y = yDestino;
            movimentou = true;
        } else if (yVal < 0.4f) {
            yDestino--;
            moverInterpoladoXY(pos.x, pos.y, pos.x, yDestino, passosInterpol);
            pos.y = yDestino;
            movimentou = true;
        }

        // === Atualiza LCD a cada X passos ===
        if (movimentou) {
            passosDesdeUltimoPrint++;
            if (passosDesdeUltimoPrint >= passosEntrePrints) {
                char buf[32];
                sprintf(buf, "X:%d Y:%d", pos.x, pos.y);
                printLCD(buf, 0);
                passosDesdeUltimoPrint = 0;
            }
        }

        wait_ms(5);  // controle leve de loop, sem travar
    }

    // Atualiza o LCD ao final, posição final
    char buf[32];
    sprintf(buf, "Final X:%d Y:%d", pos.x, pos.y);
    printLCD(buf, 0);
}

