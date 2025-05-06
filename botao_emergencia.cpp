#include "botao_emergencia.h"
#include "IHM.h"
#include "JOG.h"
#include "printLCD.h"
#include "mbed.h"

// Pino e flag de emergência
InterruptIn  botaoEmergencia(D2);
volatile bool pedidoEmergencia = false;

<<<<<<< HEAD
// === Pinos e variáveis ===
InterruptIn botaoEmergencia(D2);
DigitalOut buzzer(PB_2);
=======
// LEDs externos definidos em IHM.cpp
>>>>>>> a6a84bbed911adf9b699a22f0d40446b8cb42052
extern DigitalOut ledVermelho;
extern DigitalOut ledVerde;

// Buzzer e sinalização de confirmação do encoder
DigitalOut buzzer(PB_2);
extern bool   confirmado;

// ISR: sinaliza que o botão foi pressionado
void requisitarEmergencia() {
    pedidoEmergencia = true;
}

// Callback de blink, opcional
static void alternarLedVermelho() {
    ledVermelho = !ledVermelho;
}

// Função que implementa todo o protocolo de emergência
void modoEmergencia() {
    // 1) Estado de emergência ativo
    pararMotores();

    // 2) Aciona buzzer e LED vermelho fixo
    buzzer      = 1;
    ledVermelho = 1;

    // 3) Mensagens de alerta
    printLCD("   ATENCAO:        ", 0);
    printLCD("   Operacao        ", 1);
    printLCD("   Interrompida!   ", 2);
    wait(2);
    printLCD(" MODO DE EMERGENCIA", 0);

    // 4) Aguarda liberação do botão de emergência
    while (botaoEmergencia.read() == 1) {
        // nada aqui
    }

    // 5) Pergunta confirmação de saída
    printLCD("Confirmar Saída do", 0);
    printLCD("Modo de Emergencia?", 1);
    confirmado = false;
    while (!confirmado) { }
    wait_ms(300);

    // 6) Desliga buzzer e vermelho
    buzzer      = 0;
    ledVermelho = 0;

    // 7) Blink rápido com o LED verde
    ledVerde = 1;
    wait_ms(300);
    ledVerde = 0;

    // 8) Pergunta reinício do processo
    printLCD("Reiniciar Processo?", 0);
    confirmado = false;
    while (!confirmado) { }
    wait_ms(300);

    // 9) Saída do modo emergência
    printLCD("  Sistema Ativo   ", 0);
}
