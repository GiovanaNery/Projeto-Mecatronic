#include "botao_emergencia.h"
#include "IHM.h"
#include "JOG.h"
#include "cmsis_nvic.h" // necessário para NVIC_SystemReset()
#include "mbed.h"
#include "printLCD.h"

// Pino e flag de emergência
DigitalIn botaoEmergencia(D2);
volatile bool pedidoEmergencia = false;

// LEDs externos definidos em IHM.cpp
extern DigitalOut ledVermelho;
extern DigitalOut ledAmarelo;
extern DigitalOut ledVerde;

// Buzzer e sinalização de confirmação do encoder
DigitalOut buzzer(PB_2);
extern bool confirmado;

// ISR: sinaliza que o botão foi pressionado
// void requisitarEmergencia() { pedidoEmergencia = true; }

// Função que implementa todo o protocolo de emergência
void modoEmergencia() {

  // configura encoder + botões
  setupEncoder();

  // 1) Estado de emergência ativo
  pararMotores();

  // 2) Aciona buzzer e LED vermelho fixo
  ledAmarelo = 0;
  buzzer = 1;
  ledVermelho = 1;

  // 3) Mensagens de alerta
  printLCD(" MODO DE EMERGENCIA ", 0);

  // 4) Aguarda liberação do botão de emergência
  while (botaoEmergencia.read() == 1) {
    // nada aqui
  }
  // Desliga buzzer
  buzzer = 0;

  // 5) Pergunta confirmação de saída
  printLCD("Confirmar saida do", 0);
  printLCD("Modo de Emergencia?", 1);
  confirmado = false;
  while (!confirmado) {
    wait(0.5);
  }
  wait_ms(300);

  // 6) Desliga led vermelho e liga verde
  ledVermelho = 0;
  ledVerde = 1;

  confirmado = false;
  // 8) Pergunta reinício do processo
  printLCD(" Deseja reiniciar ", 0);
  printLCD("    o processo?   ", 1);
  while (!confirmado) {
    // espera nova confirmação
    wait(0.5);
  }
  wait_ms(300);

  // 9) Reinicia sistema
  NVIC_SystemReset();
  ledVerde = 0;
}
