#include "mbed.h"
#include "IHM.h"
#include "JOG.h"
#include "printLCD.h" 

// === Pinos e objetos ===
DigitalIn botaoEmergencia(D2);
DigitalOut buzzer(PB_5);

extern bool confirmado;               // Sinal de confirmação (controlado por aoConfirmar)
bool emergenciaAtiva = false;
bool pararPiscar = false;             // Controle para piscar LED


//Função principal do MODO DE EMERGÊNCIA (NR-12) 
void modoEmergencia() {
    emergenciaAtiva = true;
    pararMotores();      // Para os 3 eixos
    buzzer = 1;          // Liga alarme sonoro
    pararPiscar = false;
    piscarLed('r', pararPiscar);

    printLCD("ATENCAO: Operacão Interrompida!",0);
    wait(2);
    printLCD("MODO DE EMERGENCIA", 0);

    // Aguarda o botão de emergência ser liberado
    while (botaoEmergencia == 0);

    // Confirma saída do modo de emergência
    printLCD("Confirmar Saída do Modo de Emergencia?",0);
    confirmado = false;
    while (!confirmado);
    wait_ms(300);  // debounce

    // Para o alarme e o LED
    buzzer = 0; //desligar buzzer (som)
    pararPiscar = true;  // Encerra thread de piscar

    // Confirma reinício do processo
    printLCD("Reiniciar Processo?",0);
    confirmado = false;
    while (!confirmado);
    wait_ms(300);

    emergenciaAtiva = false;
    printLCD("Sistema Ativo",0);
}
