#include "Referenciamento.h"
#include "JOG.h"
#include "mbed.h"
#include "printLCD.h"

// Definindo pinos dos sensores de fim de curso dos eixos X, Y e Z
DigitalIn endstopX_neg(PB_13); // Endstop X negativo (lado esquerdo, posição zero)
DigitalIn endstopX_pos(PB_15); // Endstop X positivo (lado direito, posição máxima)
DigitalIn endstopY_neg(D6);    // Endstop Y negativo (frente, posição zero)
DigitalIn endstopY_pos(D7);    // Endstop Y positivo (trás, posição máxima)
DigitalIn endstopZ_neg(PC_12); // Endstop Z negativo (topo, define posição zero)
DigitalIn endstopZ_pos(PC_10); // Endstop Z positivo (base, define limite inferior)
extern DigitalIn botaoEmergencia; // Declaração externa do botão de emergência

// Variáveis globais para armazenar o curso total (em número de passos)
int curso_total_x = 0; // Curso total estimado para o eixo X (inicial 0)
int curso_total_y = 0; // Curso total estimado para o eixo Y (inicial 0)
int curso_total_z = 0; // Curso total estimado para o eixo Z (inicial 0)

// Inicialização dos contadores de passos em um valor alto (para permitir movimento inicial)
int passos_X = 1000000; // Passos atuais no eixo X (inicialmente muito alto)
int passos_Y = 1000000; // Passos atuais no eixo Y (inicialmente muito alto)
int passos_Z = 1000000; // Passos atuais no eixo Z (inicialmente muito alto)

bool reset; // Flag auxiliar usada no referenciamento dos eixos X e Y

// === Referenciamento do Eixo Z ===
// O objetivo é levar o eixo Z ao ponto zero (topo) e depois ajustar a posição de referência.
void referenciar_EixoZ() {
    // Configura o endstopZ_pos com resistor PullUp para leitura correta (ativo-baixo)
    endstopZ_pos.mode(PullUp);

    // Se não houver emergência ativa, exibe mensagem inicial de referenciamento no LCD
    if (botaoEmergencia == 0) {
        printLCD("   Referenciamento   ", 0); // Linha 0: “Referenciamento”
        printLCD("         do         ", 1);  // Linha 1: “do”
        printLCD("       Eixo Z       ", 2);  // Linha 2: “Eixo Z”
    }

    // 1. Sobe até o topo (posição zero) enquanto o endstop Z positivo não estiver acionado (leitura 1) e não houver emergência
    while (endstopZ_pos.read() == 1 && botaoEmergencia == 0) {
        z(-1); // Chama função z() com -1 para mover no sentido “subir” (assume que -1 equivale a subir)
    }
    // Após atingir o topo, definimos passos_Z num valor arbitrário (3000) que representa uma posição acima do zero
    passos_Z = 3000;
    // Em seguida, movemos Z até uma posição ligeiramente abaixo (3150) para tirar “folga” do endstop
    mover_Z(3150);
    // Depois, posicionamos o eixo Z em 150 passos acima do zero de referência
    passos_Z = 150;

    // Se não houver emergência, exibe mensagem de fim de referenciamento no LCD
    if (botaoEmergencia == 0) {
        printLCD("Fim do Referenciamento", 0); // Linha 0: “Fim do Referenciamento”
    }
}

// === Referenciando os Eixos X e Y ===
// Objetivo: levar ambos eixos X e Y à posição zero (endstops negativos) e depois posicionar num “curso virtual” seguro.
void referenciar_EixoXY() {
    // Se não houver emergência, exibe mensagem inicial de referenciamento no LCD
    if (botaoEmergencia == 0) {
        printLCD("   Referenciamento   ", 0); // Linha 0: “Referenciamento”
        printLCD("         dos         ", 1); // Linha 1: “dos”
        printLCD("     Eixos X e Y     ", 2); // Linha 2: “Eixos X e Y”
        // Zera as rampas de aceleração nos eixos X e Y, chamando x(0, tempo_interpolado) e y(0, tempo_interpolado)
        // Isso garante que, ao iniciar o movimento de referenciamento, não haja velocidade residual
        x(0, tempo_interpolado);
        y(0, tempo_interpolado);
        reset = true; // Sinaliza que acabamos de resetar as rampas
    }
    // 1. Movimenta X e Y simultaneamente até atingir ambos endstops negativos (leitura 1 significa não acionado)
    while ((endstopX_neg.read() == 1 && botaoEmergencia == 0) ||
           (endstopY_neg.read() == 1 && botaoEmergencia == 0)) {
        // Verifica se ainda precisa mover em X ou em Y (endstop ainda não acionado)
        bool moverX = (endstopX_neg.read() == 1);
        bool moverY = (endstopY_neg.read() == 1);

        if (moverX && moverY) {
            // Se ambos precisam se mover, faz movimento interpolado (mesmo tempo para X e Y)
            x(-1, tempo_interpolado); // Move X no sentido negativo (0 para 1) com tempo_interpolado
            y(-1, tempo_interpolado); // Move Y no sentido negativo com o mesmo tempo_interpolado
        } else {
            // Se apenas um eixo precisa ser referenciado ou se estamos fazendo a primeira iteração após reset
            if (reset) {
                // Zera rampas novamente para evitar velocidade residual num dos eixos
                x(0, 0);
                y(0, 0);
                reset = false; // Desativa flag de reset
            }
            if (moverX) {
                // Se apenas X precisa andar para o endstop negativo, move X com tempo “tempo” (mais lento ou mais rápido)
                x(-1, tempo);
            }
            if (moverY) {
                // Se apenas Y precisa andar para o endstop negativo, move Y com tempo “tempo”
                y(-1, tempo);
            }
        }
    }

    // Ao sair do loop, significa que ambos eixos atingiram o endstop negativo (zero)
    if (botaoEmergencia == 0) {
        // Define passos_X e passos_Y para um valor alto temporariamente (100000) para permitir movimento posterior até fim de curso virtual
        passos_X = 100000;
        passos_Y = 100000;
        wait(0.5); // Pequena pausa para estabilização

        // Move os eixos X e Y até uma posição bem além do zero (112000 passos) usando interpolação Bresenham
        // Esse “fim de curso virtual” deixa folga antes de posicionar a referência final
        moverInterpoladoXY(112000, 112000); 

        // Define agora os passos_X e passos_Y para 12000, que passam a ser a referência real (ponto zero efetivo do sistema)
        passos_X = 12000;
        passos_Y = 12000;

        // Exibe mensagem de “Fim do Referenciamento” no LCD
        printLCD("Fim do Refenciamento", 0); // Linha 0: “Fim do Referenciamento”
    }
}