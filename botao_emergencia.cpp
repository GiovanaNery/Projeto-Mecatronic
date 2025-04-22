#include "mbed.h"
#include "IHM.h"      // só pra referenciar ledVermelho
#include "JOG.h"
#include "printLCD.h"

// === Pinos e variáveis ===
DigitalIn  botaoEmergencia(D2);
DigitalOut buzzer(PB_5);
DigitalOut ledVermelho(PC_10);

extern bool confirmado;
bool emergenciaAtiva = false;

// Criamos um Ticker para o piscar não‑bloqueante
Ticker tickerPiscar;

// Callback do Ticker: alterna o LED vermelho
void alternarLedVermelho() {
    ledVermelho = !ledVermelho;
}

// Modo de emergência, compatível com Mbed 2
void modoEmergencia() {
    emergenciaAtiva = true;

    // 1) Para todos os eixos
    pararMotores();

    // 2) Aciona buzzer
    buzzer = 1;

    // 3) Inicia piscar do LED vermelho a cada 300 ms
    tickerPiscar.attach(&alternarLedVermelho, 0.3);

    // 4) Mensagens no LCD
    printLCD("ATENCAO: Operacão Interrompida!", 0);
    wait(2);
    printLCD("MODO DE EMERGENCIA", 0);

    // 5) Aguarda o botão de emergência ser liberado
    while (botaoEmergencia == 0) {
        // aqui o LED continua piscando via Ticker
    }

    // 6) Solicita confirmação de saída
    printLCD("Confirmar Saída do Modo de Emergencia?", 0);
    confirmado = false;
    while (!confirmado) {
        // LED ainda piscando
    }
    wait_ms(300);

    // 7) Desliga buzzer e piscar
    buzzer = 0;
    tickerPiscar.detach();
    ledVermelho = 0;

    // 8) Solicita confirmação de reinício
    printLCD("Reiniciar Processo?", 0);
    confirmado = false;
    while (!confirmado) {
        // sem piscar agora
    }
    wait_ms(300);

    // 9) Fim da emergência
    emergenciaAtiva = false;
    printLCD("Sistema Ativo", 0);
}
