#include "botao_emergencia.h"
#include "IHM.h"
#include "JOG.h"
#include "cmsis_nvic.h" // necessário para NVIC_SystemReset()
#include "mbed.h"
#include "printLCD.h"

// Pino de entrada para o botão de emergência (ativo-alto)
DigitalIn botaoEmergencia(D2);
// Flag volátil para sinalizar pedido de emergência (usada em ISR, não implementada aqui)
volatile bool pedidoEmergencia = false;

// LEDs externos definidos em IHM.cpp (declarações externas)
extern DigitalOut ledVermelho;
extern DigitalOut ledAmarelo;
extern DigitalOut ledVerde;

// Buzzer para sinalização sonora e flag de confirmação do encoder (declarada externamente)
DigitalOut buzzer(PB_2);
extern bool confirmado;

// ISR (rotina de interrupção) comentada: seria chamada ao apertar o botão de emergência
// void requisitarEmergencia() { pedidoEmergencia = true; }

// Função que implementa todo o protocolo de emergência
void modoEmergencia() {
    // Configura encoder e botões para leitura (função definida em outro lugar)
    setupEncoder();

    // 1) Estado de emergência ativo: para todos os motores imediatamente
    pararMotores();

    // 2) Aciona buzzer e LED vermelho (LED amarelo apagado, LED vermelho aceso)
    ledAmarelo = 0; // Garante que LED amarelo esteja desligado
    buzzer = 1;     // Liga o buzzer (sinal sonoro constante)
    ledVermelho = 1;// Liga o LED vermelho para indicar emergência

    // 3) Exibe mensagem de alerta no LCD na linha 0
    printLCD(" MODO DE EMERGENCIA ", 0);

    // 4) Aguarda até que o botão de emergência seja liberado (botaoEmergencia.read() == 0)
    while (botaoEmergencia.read() == 1) {
        // Fica em loop sem fazer nada até o usuário soltar o botão de emergência
    }
    // Uma vez liberado, desliga o buzzer
    buzzer = 0;

    // 5) Pergunta ao usuário se deseja sair do modo de emergência
    printLCD("   Pressione para   ", 0);     // Linha 0: instrução
    printLCD(" confirmar saida do ", 1);     // Linha 1: instrução
    printLCD(" Modo de Emergencia ", 2);     // Linha 2: especifica “Modo de Emergencia”
    confirmado = false;                      // Reseta flag de confirmação
    while (!confirmado) {
        // Aguarda até que o usuário confirme (por exemplo, apertando o encoder)
        wait(0.5); // Pequena pausa para não travar o processador
    }
    wait_ms(300); // Debounce: espera 300 ms antes de prosseguir

    // 6) Desliga o LED vermelho e acende o LED verde para indicar saída da emergência
    ledVermelho = 0; // Desliga LED vermelho
    ledVerde = 1;    // Liga LED verde

    confirmado = false; // Reseta novamente a flag de confirmação

    // 8) Pergunta se o usuário deseja reiniciar o processo
    printLCD("   Pressione para   ", 0); // Linha 0: instrução
    printLCD("      reiniciar     ", 1); // Linha 1: instrução
    printLCD("     o processo     ", 2); // Linha 2: especifica “o processo”
    while (!confirmado) {
        // Aguarda nova confirmação do usuário
        wait(0.5); // Pequena pausa em cada iteração
    }
    wait_ms(300); // Debounce: espera 300 ms antes de reiniciar

    // 9) Reinicia o sistema via NVIC (reset completo do microcontrolador)
    NVIC_SystemReset(); // Chama reset para retornar ao estado inicial
    ledVerde = 0;       // Desliga o LED verde (não será alcançado se o reset ocorrer imediatamente)
}