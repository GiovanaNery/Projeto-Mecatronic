#include "mbed.h"
#include "IHM.h"      
#include "JOG.h"
#include "printLCD.h"

// === Pinos e variáveis ===
DigitalIn  botaoEmergencia(D2);
DigitalOut buzzer(PB_5);
DigitalOut ledVermelho(PC_10);

extern bool confirmado;
bool emergenciaAtiva = false;

// Criamos um Ticker para o piscar não-bloqueante
Ticker tickerPiscar;

// Callback do Ticker: alterna o LED vermelho
void alternarLedVermelho() {
    ledVermelho = !ledVermelho;
}

// Modo de emergência
void modoEmergencia() {
    emergenciaAtiva = true;          // entra em modo de emergência
    pararMotores();                  // 1) para todos os motores
    buzzer = 1;                      // 2) liga o buzzer
    tickerPiscar.attach(&alternarLedVermelho, 0.3);  // 3) começa a piscar o LED vermelho

    // 4) mostra mensagem de interrupção
    printLCD("ATENCAO: Operacão Interrompida!", 0);
    wait(2);
    printLCD("MODO DE EMERGENCIA", 0);

    // 5) aguarda liberar o botão de emergência
    while (botaoEmergencia == 0) {
        // LED continua piscando via Ticker
    }
    // LOGO QUE O BOTÃO É LIBERADO:
    tickerPiscar.detach();   // para o piscar
    ledVermelho = 0;         // garante o LED desligado

    // 6) solicita confirmação de saída do modo emergência (buzzer ainda ativo)
    printLCD("Confirmar Saída do Modo de Emergencia?", 0);
    confirmado = false;
    while (!confirmado) {
        // aqui o LED já está apagado, buzzer continua som
    }
    wait_ms(300);  // debounce

    // 7) agora sim desliga o buzzer
    buzzer = 0;

    // 8) solicita confirmação de reinício
    printLCD("Reiniciar Processo?", 0);
    confirmado = false;
    while (!confirmado) {
        // permanece tudo desligado
    }
    wait_ms(300);

    // 9) sai do modo emergência
    emergenciaAtiva = false;
    printLCD("Sistema Ativo", 0);
}

//1. Botão apertado → buzzer + LED piscando.
//2. Botão liberado → LED apaga imediatamente; buzzer continua.
//3. Usuário confirma saída → buzzer para.
//4. Usuário confirma reinício → sistema volta ao normal.