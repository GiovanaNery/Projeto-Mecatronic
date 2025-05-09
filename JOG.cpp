#include "IHM.h"
#include "Referenciamento.h"
#include "mbed.h"
#include "printLCD.h"
#include <math.h>
#define PI 3.14159265f
#define T_BASE_MS 2.0f
#define INITIAL_FACTOR   16     // começa em 8 ms (8 000 µs)
#define RAMP_DECR_US         1    // tira 0,2 ms (200 µs) a cada pulso
#define PULSE_SETUP_US     2    // setup da direção
#define PULSE_WIDTH_US     5    // largura mínima do STEP
#define PULSE_GAP_US       10    // gap entre X e Y
#define BASE_DELAY_US  55    // ~1 ms/16 ≈62 µs
#define RAMP_PERCENT     10    // percentual de passos para ramp-up/down
#define JITTER_US        3     // jitter máximo em µs para quebrar ressonância
float tempo = (0.0006/16.0);
float tempo_interpolado = (0.0006 / 32.0);
//float tempo_z = 0.001;             61    // tempo para o eixo Z

// Definindo os pinos do motor de passo de cada eixo (X, Y e Z)
DigitalOut Enable(D3); // Pino enable de liga e desliga
BusOut MOTOR_Z(D10, D11, D12, D13);
Serial pc(USBTX, USBRX, 9600);

// Pinos conectados ao driver do motor no eixo X
DigitalOut DIR_X(A3);  // Pino de direção (DIR)
DigitalOut STEP_X(D4); // Pino de passo (STEP)

// Pinos conectados ao driver do eixo Y
DigitalOut DIR_Y(PB_7);  // Direção
DigitalOut STEP_Y(D9); // Passo

// === ENTRADAS ===
AnalogIn joystickX(A1);
AnalogIn joystickY(PC_4);
extern DigitalIn botaoZmais; // Pressionado = HIGH (sem pull-down)
extern DigitalIn botaoZmenos;
extern DigitalIn seletor;

// Criando parametros
int Z_passo = 0;


// --- eixo X: recebe periodo_s em segundos ---
// Função X com ramp-up automático e reset ao parar
void x(int direcao, float periodo_s) {
    static float delayX_us = 0.0f;
    static bool  rampandoX = false;

    // Se parar ou período inválido, zera estado de rampa e delay
    if (direcao == 0 || periodo_s <= 0.0f) {
        rampandoX = false;
        delayX_us = 0.0f;
        return;
    }

    // Converte segundos → microsegundos
    int alvo_us = int(periodo_s * 1e6f);

    // Inicia arrancada com delay maior só na primeira chamada
    if (!rampandoX) {
        delayX_us  = alvo_us * INITIAL_FACTOR;
        rampandoX = true;
    }

    // Rampa de subida: decrementa até alcançar o alvo
    if (delayX_us > alvo_us) {
        delayX_us -= RAMP_DECR_US;
        if (delayX_us < alvo_us) delayX_us = alvo_us;
    }

    // Gera pulso no driver
    DIR_X  = (direcao > 0);
    wait_us(PULSE_SETUP_US);
    STEP_X = 1;
    wait_us(PULSE_WIDTH_US);
    STEP_X = 0;

    // Aguarda conforme rampa e atualiza contador
    wait_us(int(delayX_us));
    passos_X += direcao;
}

// --------------------------------------------------
// Função Y com ramp-up automático e reset ao parar
void y(int direcao, float periodo_s) {
    static float delayY_us = 0.0f;
    static bool  rampandoY = false;

    // Se parar ou período inválido, zera estado de rampa e delay
    if (direcao == 0 || periodo_s <= 0.0f) {
        rampandoY = false;
        delayY_us = 0.0f;
        return;
    }

    // Converte segundos → microsegundos
    int alvo_us = int(periodo_s * 1e6f);

    // Inicia arrancada com delay maior só na primeira chamada
    if (!rampandoY) {
        delayY_us  = alvo_us * INITIAL_FACTOR;
        rampandoY = true;
    }

    // Rampa de subida: decrementa até alcançar o alvo
    if (delayY_us > alvo_us) {
        delayY_us -= RAMP_DECR_US;
        if (delayY_us < alvo_us) delayY_us = alvo_us;
    }

    // Gera pulso no driver
    DIR_Y  = (direcao > 0);
    wait_us(PULSE_SETUP_US);
    STEP_Y = 1;
    wait_us(PULSE_WIDTH_US);
    STEP_Y = 0;

    // Aguarda conforme rampa e atualiza contador
    wait_us(int(delayY_us));
    passos_Y += direcao;
}
//------------------------------------------------------------------------------
// Função original de acionamento do eixo Z (mantida sem alterações)
void z(int direcao, float velocidade) {
    // Subir
    if (direcao < 0) {
        MOTOR_Z = 1 << Z_passo;
        Z_passo = (Z_passo + 1) % 4;
        passos_Z++;
        wait_us(int(velocidade * 1e6f));
    }
    // Descer
    else if (direcao > 0) {
        MOTOR_Z = 1 << Z_passo;
        Z_passo = (Z_passo - 1 + 4) % 4;
        passos_Z--;
        wait_us(int(velocidade * 1e6f));
    }
    MOTOR_Z = 0;
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


void xStep(int dir) {
    if (dir == 0) return;
    DIR_X = (dir > 0) ? 1 : 0;
    wait_us(PULSE_SETUP_US);
    STEP_X = 1;
    wait_us(PULSE_WIDTH_US);
    STEP_X = 0;
    passos_X += (dir > 0) ? +1 : -1;
}

void yStep(int dir) {
    if (dir == 0) return;
    DIR_Y = (dir > 0) ? 1 : 0;
    wait_us(PULSE_SETUP_US);
    STEP_Y = 1;
    wait_us(PULSE_WIDTH_US);
    STEP_Y = 0;
    passos_Y += (dir > 0) ? +1 : -1;
}

// Interpolação Bresenham com suavização de aceleração e jitter (Mbed 2)
void moverInterpoladoXY(int xDestino, int yDestino) {
    // inicializa jitter timer somente na primeira chamada
    static Timer jitterTimer;
    static bool jitterInit = false;
    if (!jitterInit) {
        jitterTimer.start();
        srand(jitterTimer.read_us());
        jitterInit = true;
    }

    // habilita driver (ativo-baixo)
    Enable = 0;

    int x0 = passos_X;
    int y0 = passos_Y;
    int dx = abs(xDestino - x0);
    int dy = abs(yDestino - y0);
    int sx = (x0 < xDestino) ? 1 : -1;
    int sy = (y0 < yDestino) ? 1 : -1;
    int err = dx - dy;

    int totalSteps = (dx > dy) ? dx : dy;
    int rampSteps  = totalSteps * RAMP_PERCENT / 100;
    if (rampSteps < 1) rampSteps = 1;

    for (int stepCount = 0; stepCount < totalSteps; ++stepCount) {
        bool doX = false, doY = false;
        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
            doX = true;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
            doY = true;
        }

        // perfil trapezoidal de velocidade
        int delayUs = BASE_DELAY_US;
        if (stepCount < rampSteps) {
            delayUs += (rampSteps - stepCount) * (BASE_DELAY_US / rampSteps);
        } else if (stepCount > totalSteps - rampSteps) {
            int idx = stepCount - (totalSteps - rampSteps);
            delayUs += idx * (BASE_DELAY_US / rampSteps);
        }
        // aplica jitter
        int jitter = (rand() % (2 * JITTER_US + 1)) - JITTER_US;
        delayUs += jitter;
        // garante mínimo de delay para não cortar pulso
        int minDelay = PULSE_SETUP_US + PULSE_WIDTH_US + 1;
        if (delayUs < minDelay) delayUs = minDelay;

        // executa pulsos com espaçamento
        if (doX && doY) {
            xStep(sx);
            wait_us(delayUs / 2);
            yStep(sy);
            wait_us(delayUs - delayUs / 2);
        } else if (doX) {
            xStep(sx);
            wait_us(delayUs);
        } else if (doY) {
            yStep(sy);
            wait_us(delayUs);
        }
    }

    // desabilita driver
    Enable = 1;
}


// === POSICIONAMENTO MANUAL COM INTERPOLAÇÃO E JOYSTICK ===
extern volatile bool confirmado;
extern DigitalIn endstopX_pos; // ativo em 0 quando bate no limite direito
extern DigitalIn endstopY_pos; // ativo em 0 quando bate no limite traseiro
extern DigitalIn endstopX_neg;
extern DigitalIn endstopY_neg;
extern DigitalIn endstopZ_neg;
extern DigitalIn endstopZ_pos;

struct Ponto3D {
  int x, y, z;
};
float DEADZONE = 0.2f;

// Modo de posicionamento manual 
float velocidade_jog = 0.02f;

// Modo de posicionamento manual
void modoPosicionamentoManual() {
    Enable = 0;
    confirmado = false;

    // Configure os botões de Z (uma única vez, idealmente no main)
    botaoZmais .mode(PullUp);
    botaoZmenos.mode(PullUp);

    while (!confirmado) {
        chaveseletora();
        float xVal = joystickX.read() - 0.5f;
        float yVal = joystickY.read() - 0.5f;

        int dirX = (fabs(xVal) > DEADZONE) ? (xVal > 0 ? +1 : -1) : 0;
        int dirY = (fabs(yVal) > DEADZONE) ? (yVal > 0 ? +1 : -1) : 0;

        // >>> Travamento dos limites X/Y (sensor ativo em LOW) <<<
        if (dirX < 0 && endstopX_neg.read() == 0) dirX = 0;
        if (dirX > 0 && endstopX_pos.read() == 0) dirX = 0;
        if (dirY < 0 && endstopY_neg.read() == 0) dirY = 0;
        if (dirY > 0 && endstopY_pos.read() == 0) dirY = 0;

        float vel = (dirX && dirY) ? (velocidade_jog / 2.0f) : velocidade_jog;

        // 1) Movimento X/Y
        x(dirX, vel);
        y(dirY, vel);

        // 2) Travamento e movimento do eixo Z (sensor ativo em LOW)    
        //    Só sobe se Z+ pressionado e sensor de base NÃO estiver acionado
        if (botaoZmais.read() == 0 && endstopZ_pos.read() != 0) {
            z(+1, velocidade_jog);
        }
        //    Só desce se Z- pressionado e sensor de topo NÃO estiver acionado
        else if (botaoZmenos.read() == 0 && endstopZ_neg.read() != 0) {
            z(-1, velocidade_jog);
        }
    }

    Enable = 1;
}



void mover_Z(float posZ){
    while(passos_Z != posZ){
        if (passos_Z >= posZ) {
            z(-1,tempo);
        }
        if (passos_Z <= posZ) {
            z(1,tempo);
        }

    }
}