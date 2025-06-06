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
float tempo = (0.0006/16.0);
float tempo_z = 0.002;
float tempo_interpolado = (0.0006/ 32.0);
//float tempo_z = 0.001;             61    // tempo para o eixo Z
// Definindo os pinos do motor de passo de cada eixo (X, Y e Z)
DigitalOut Enable(D3); // Pino enable de liga e desliga
BusOut MOTOR_Z(D10, D11, D12, D13);
Serial pc(USBTX, USBRX, 9600);
extern DigitalIn endstopX_pos; // ativo em 0 quando bate no limite direito
extern DigitalIn endstopY_pos; // ativo em 0 quando bate no limite traseiro
extern DigitalIn endstopX_neg;
extern DigitalIn endstopY_neg;
extern DigitalIn endstopZ_neg;
extern DigitalIn endstopZ_pos;

// Pinos conectados ao driver do motor no eixo X
DigitalOut DIR_X(A3);  // Pino de direção (DIR)
DigitalOut STEP_X(D4); // Pino de passo (STEP)

// Pinos conectados ao driver do eixo Y
DigitalOut DIR_Y(D8);  // Direção
DigitalOut STEP_Y(D9); // Passo

// === ENTRADAS ===
AnalogIn joystickX(A0);
AnalogIn joystickY(A1);
extern DigitalIn botaoZmais; // Pressionado = HIGH (sem pull-down)
extern DigitalIn botaoZmenos;
extern DigitalIn seletor;
extern DigitalIn botaoEmergencia;

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

    // === LIMITE DIGITAL DE PASSOS ===
    if ((direcao > 0 && passos_X >= 381759) ||
        (direcao < 0 && passos_X <= 10000)) {
        return; // bloqueia movimento
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

    // === LIMITE DIGITAL DE PASSOS ===
    if ((direcao > 0 && passos_Y >= 453582) ||
        (direcao < 0 && passos_Y <= 10000)) {
        return; // bloqueia movimento
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
void z(int direcao) {
    // Subir
    if (direcao < 0 && passos_Z > 100) {
        MOTOR_Z = 1 << Z_passo;
        Z_passo = (Z_passo + 1) % 4;
        passos_Z--;
        wait_us((0.0025 * 1e6f));
    }
    // Descer
    else if (direcao > 0 && passos_Z < 3950) {
        MOTOR_Z = 1 << Z_passo;
        Z_passo = (Z_passo - 1 + 4) % 4;
        passos_Z++;
        wait_us((0.0025 * 1e6f));
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
    // === Limite digital de passos para o eixo X ===
    // Se o contador de passos_X for menor ou igual a 10000, não faz nada e retorna.
    // Isso previne movimento quando atingido limite mínimo de passos.
    if (passos_X <= 10000) {
        return; // bloqueia movimento
    }
    // Verifica os endstops dos limites físico do eixo X:
    // - Se dir > 0 (movendo para a posição positiva) e o endstop positivo estiver acionado (leitura 0), bloqueia.
    // - Se dir < 0 (movendo para a posição negativa) e o endstop negativo estiver acionado (leitura 0), bloqueia.
    if (dir > 0 && endstopX_pos.read() == 0 || dir < 0 && endstopX_neg.read() == 0) {
        return; // bloqueia movimento por sensor de limite
    }
    // Se dir for 0, não há direção, então retorna sem gerar pulso.
    if (dir == 0) return;
    // Define o sinal do pino DIR_X para controlar a direção do driver de passo:
    // - Se dir > 0, DIR_X = 1 (movimento em sentido "positivo" do motor).
    // - Se dir < 0, DIR_X = 0 (movimento em sentido "negativo").
    DIR_X = (dir > 0) ? 1 : 0;
    // Aguarda o tempo de setup do pulso antes de acionar o pino STEP
    wait_us(PULSE_SETUP_US);
    // Sinaliza borda de subida no pino STEP_X (inicia o pulso)
    STEP_X = 1;
    // Mantém o pulso ativo pelo tempo de largura especificado
    wait_us(PULSE_WIDTH_US);
    // Desativa o pulso no pino STEP_X (finaliza o pulso)
    STEP_X = 0;
    // Atualiza o contador de passos_X:
    // - Se movendo em direção positiva, incrementa.
    // - Se movendo em direção negativa, decrementa.
    passos_X += (dir > 0) ? +1 : -1;
}

void yStep(int dir) {
    // === Limite digital de passos para o eixo Y ===
    // Se o contador de passos_Y for menor ou igual a 10000, não faz nada e retorna.
    if (passos_Y <= 10000) {
        return; // bloqueia movimento
    }
    // Verifica os endstops dos limites físico do eixo Y:
    // - Se dir > 0 (movendo para a posição positiva) e o endstop positivo estiver acionado, bloqueia.
    // - Se dir < 0 (movendo para a posição negativa) e o endstop negativo estiver acionado, bloqueia.
    if (dir > 0 && endstopY_pos.read() == 0 || dir < 0 && endstopY_neg.read() == 0) {
        return; // bloqueia movimento por sensor de limite
    }
    // Se dir for 0, não há nada a fazer, então retorna.
    if (dir == 0) return;
    // Define o sinal do pino DIR_Y para controlar a direção do driver de passo:
    DIR_Y = (dir > 0) ? 1 : 0;
    // Aguarda o tempo de setup para garantir estabilidade antes de gerar pulso
    wait_us(PULSE_SETUP_US);
    // Ativa o pino STEP_Y para iniciar pulso de passo
    STEP_Y = 1;
    // Mantém o pulso ativo pelo tempo de largura definido
    wait_us(PULSE_WIDTH_US);
    // Desativa o pino STEP_Y para finalizar o pulso
    STEP_Y = 0;
    // Atualiza o contador de passos_Y:
    passos_Y += (dir > 0) ? +1 : -1;
}

#define PULSE_GAP_US       10    // Gap mínimo (em microssegundos) entre pulsos de X e Y
#define BASE_DELAY_US      55    // Delay base (~1 ms/16 ≈ 62 µs) para interpolação de passos
#define RAMP_PERCENT       10    // Percentual de passos usados para rampa de aceleração/desaceleração
#define JITTER_US          3     // Máximo jitter (em µs) para quebrar ressonância mecânica

// Interpolação Bresenham com suavização de aceleração e jitter (Mbed 2)
void moverInterpoladoXY(int xDestino, int yDestino) {
    // --- Inicialização do timer de jitter na primeira chamada ---
    static Timer jitterTimer;       // Timer para gerar valor de semente aleatória
    static bool jitterInit = false; // Flag para checar se já inicializamos o jitter
    if (!jitterInit) {
        jitterTimer.start();                             // Inicia o timer
        srand(jitterTimer.read_us());                    // Usa tempo atual em microssegundos como semente randômica
        jitterInit = true;                               // Marca como inicializado
    }

    // Habilita o driver de motores (nível ativo-baixo no pino Enable)
    Enable = 0;

    // Pega a posição inicial atual de passos nos eixos X e Y
    int x0 = passos_X;
    int y0 = passos_Y;
    // Calcula as diferenças absolutas de passos entre posição atual e destino
    int dx = abs(xDestino - x0);
    int dy = abs(yDestino - y0);
    // Determina as direções (+1 ou -1) para X e Y com base na posição destino
    int sx = (x0 < xDestino) ? 1 : -1;
    int sy = (y0 < yDestino) ? 1 : -1;
    // Erro inicial para o algoritmo de Bresenham
    int err = dx - dy;

    // Calcula o número total de passos que serão executados
    int totalSteps = (dx > dy) ? dx : dy;
    // Calcula quantos passos serão usados na fase de rampa (10% do total)
    int rampSteps  = totalSteps * RAMP_PERCENT / 100;
    // Garante pelo menos 1 passo de rampa para evitar divisão por zero
    if (rampSteps < 1) rampSteps = 1;

    // Loop principal para gerar todos os passos até o ponto final
    for (int stepCount = 0; stepCount < totalSteps; ++stepCount) {
        bool doX = false, doY = false;    // Flags para indicar se faremos pulso em X ou Y
        int e2 = err * 2;                 // Dobra o erro para decisão de eixo

        // Decisão de passo para X
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
            doX = true;    // Indica que deveremos gerar pulso em X neste ciclo
        }
        // Decisão de passo para Y
        if (e2 < dx) {
            err += dx;
            y0 += sy;
            doY = true;    // Indica que deveremos gerar pulso em Y neste ciclo
        }

        // --- Perfil trapezoidal de velocidade no interpolador ---
        int delayUs = BASE_DELAY_US;  // Inicia com o delay base
        if (stepCount < rampSteps) {
            // Fase de aceleração: quanto menor stepCount, maior acréscimo de delay
            delayUs += (rampSteps - stepCount) * (BASE_DELAY_US / rampSteps);
        } else if (stepCount > totalSteps - rampSteps) {
            // Fase de desaceleração: após totalSteps - rampSteps, aplica rampa decrescente
            int idx = stepCount - (totalSteps - rampSteps);
            delayUs += idx * (BASE_DELAY_US / rampSteps);
        }

        // --- Aplica jitter aleatório para evitar ressonância ---
        int jitter = (rand() % (2 * JITTER_US + 1)) - JITTER_US; // Valor entre -JITTER_US e +JITTER_US
        delayUs += jitter;

        // Garante que o delay seja ao menos o mínimo necessário para não cortar o pulso
        int minDelay = PULSE_SETUP_US + PULSE_WIDTH_US + 1;
        if (delayUs < minDelay) delayUs = minDelay;

        // --- Executa os pulsos de passo para X e Y, espaçando conforme delay calculado ---
        if (doX && doY) {
            // Se ambos X e Y devem ser pulsados simultaneamente:
            xStep(sx);                                  // Gera pulso em X
            wait_us(delayUs / 2);                       // Metade do delay
            yStep(sy);                                  // Gera pulso em Y
            wait_us(delayUs - delayUs / 2);             // Resto do delay
        } else if (doX) {
            // Somente passo em X
            xStep(sx);
            wait_us(delayUs);
        } else if (doY) {
            // Somente passo em Y
            yStep(sy);
            wait_us(delayUs);
        }
    }

    // Desabilita o driver de motores ao fim do movimento (Enable ativo-alto desativa)
    Enable = 1;
}


// === POSICIONAMENTO MANUAL COM INTERPOLAÇÃO E JOYSTICK ===
extern volatile bool confirmado; // Variável externa que indica confirmação para sair do modo manual

struct Ponto3D {
    int x, y, z; // Estrutura para armazenar coordenadas de um ponto 3D (não usada diretamente aqui)
};
float DEADZONE = 0.1f;           // Zona morta para leitura do joystick (abs(x) < DEADZONE considera sem movimento)

// Velocidade base no modo posicionamento manual (valor de passo por iteração)
float velocidade_jog = 0.02f;

// Modo de posicionamento manual usando joystick para os eixos X, Y e botões para Z
void modoPosicionamentoManual() {
    Enable = 0;           // Habilita driver de motores (ativo-baixo)
    confirmado = false;   // Reseta sinal de confirmação
    static int lastDirX = 0, lastDirY = 0; // Direções anteriores para detecção de inversão

    // Continua em loop enquanto não houver confirmação e o botão de emergência não for pressionado
    while (!confirmado && botaoEmergencia == 0) {
        chaveseletora(); // Função para leitura de chaves seletoras (não detalhada aqui)

        // Leitura de joystick: valores de 0 a 1, centralizado em 0.5
        float xVal = joystickX.read() - 0.5f; 
        float yVal = joystickY.read() - 0.5f;

        // Determina direção de movimento: 
        // - Se o valor absoluto do joystick ultrapassar o DEADZONE, define direção (-1 ou +1).
        // - Caso contrário, dir = 0 (sem movimento).
        int dirX = (fabs(xVal) > DEADZONE) ? (xVal > 0 ? -1 : +1) : 0;
        int dirY = (fabs(yVal) > DEADZONE) ? (yVal > 0 ? -1 : +1) : 0;

        // Travamento dos limites físicos de X/Y (sensores ativos em LOW):
        if (dirX < 0 && endstopX_neg.read() == 0) dirX = 0; // Não descer se sensor de limite inferior ativo
        if (dirX > 0 && endstopX_pos.read() == 0) dirX = 0; // Não subir se sensor de limite superior ativo
        if (dirY < 0 && endstopY_neg.read() == 0) dirY = 0; // Não descer se sensor de limite inferior ativo
        if (dirY > 0 && endstopY_pos.read() == 0) dirY = 0; // Não subir se sensor de limite superior ativo

        // Se estiver movendo em X e Y simultaneamente, reduz velocidade pela metade
        float vel = (dirX && dirY) ? (velocidade_jog / 2.0f) : velocidade_jog;

        // --- Detecção de inversão de sinal para resetar rampa ---
        if (dirX * lastDirX < 0) {
            // Se houve inversão em X, manda sinal de parada (rampa = 0) para X e Y
            x(0, vel); // Função x() zera movimento em X e reseta rampa
            y(0, vel); // Função y() zera movimento em Y e reseta rampa
        }
        if (dirY * lastDirY < 0) {
            // Se houve inversão em Y, manda sinal de parada para Y e X
            y(0, vel); // Zera rampa em Y
            x(0, vel); // Zera rampa em X
        }
        lastDirX = dirX; // Atualiza última direção X
        lastDirY = dirY; // Atualiza última direção Y

        // 1) Movimento X/Y: chama funções x() e y() com a direção e velocidade
        x(dirX, vel); 
        y(dirY, vel);

        // 2) Controle do eixo Z via botões e endstops (sensores ativos em LOW):
        // - Se botão Z+ (botaoZmais) pressionado e sensor de base Z não acionado, sobe Z
        if (botaoZmais.read() == 0 && endstopZ_pos.read() != 0) {
            z(-1); // Chama função z() para subir
        }
        // - Se botão Z- (botaoZmenos) pressionado e sensor de topo Z não acionado, desce Z
        if (botaoZmenos.read() == 0 && endstopZ_neg.read() != 0) {
            z(+1); // Chama função z() para descer
        }
    }
    // Sai do loop e desabilita driver de motores
    Enable = 1;
}

// Função para mover o eixo Z até uma posição específica (posZ em número de passos)
void mover_Z(float posZ) {
    // Se posZ for 0, define posição padrão de 110 passos
    if (posZ == 0) {
        posZ = 110;
    }
    // Enquanto a posição atual (passos_Z) não alcançar posZ e não houver emergência
    while (passos_Z != posZ && botaoEmergencia == 0) {
        // Se passos_Z for maior que posZ e o endstop superior não estiver acionado, sobe Z
        if (passos_Z >= posZ && endstopZ_pos.read() != 0) {
            z(-1);
        }
        // Se passos_Z for menor que posZ e o endstop inferior não estiver acionado, desce Z
        else if (passos_Z <= posZ && endstopZ_neg.read() != 0) {
            z(1);
        }
    }
}