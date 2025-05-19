#include "Acionapipeta.h"
#include "IHM.h"
#include "JOG.h"
#include "Referenciamento.h"
#include "Salvar_Posicoes_Volume.h"
#include "TextLCD.h"
#include "botao_emergencia.h"
#include "mbed.h"
#include "printLCD.h"

extern DigitalOut ledVermelho;
extern DigitalOut ledAmarelo;
extern DigitalOut ledVerde;
extern DigitalIn botaoZmais; // Pressionado = HIGH (sem pull-down)
extern DigitalIn botaoZmenos;
DigitalOut led(PA_5);
DigitalOut startstop(PB_1);

// 1) Instancia o barramento I2C em D14=SDA, D15=SCL
I2C i2c_lcd(D14, D15);
// 2) Cria o objeto LCD na linha I2C, endereço 0x4E, 20×4
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD20x4);

// Função para testar impressão no LCD
// Exemplo usando TextLCD do Mbed
void printLCD(const char *texto, int linha) {
  // só limpa o LCD quando for escrever na linha 0
  if (linha == 0) {
    lcd.cls(); // limpa tudo
  }
  lcd.locate(0, linha); // posiciona no início da coluna 0, linha especificada
  lcd.printf("%s", texto);
}

int main() {
  startstop = 0;
  // configuracão LCD
  lcd.setCursor(TextLCD::CurOff_BlkOff);
  lcd.setBacklight(TextLCD::LightOn);
  while (true) {
    // configura encoder + botões
    setupEncoder();

    // configuracão botão de emergencia
    // botaoEmergencia.mode(PullDown);
    // botaoEmergencia.fall(&modoEmergencia);

    // Mostra o nome do sistema por 0.5s
    printLCD("     PIPETRONIX     ", 0);
    printLCD("                    ", 1);
    printLCD("                    ", 2);
    wait(2); // Espera 0.5 segundos
    // 1) perguntamos se quer iniciar o referenciamento
    printLCD("   Pressione para   ", 0);
    printLCD("     iniciar o      ", 1);
    printLCD("  referenciamento   ", 2);
    confirmado = false;
    while (!confirmado) {
      led = !led;
      wait(0.5);
      // espera o encoder ser pressionado
    }
    wait_ms(300); // debounce

    // 2) homing dos eixos X e Y
    acenderLed('y'); // liga apenas o amarelo
    Enable = 0;
    referenciar_EixoZ();
    referenciar_EixoXY();
    Enable = 1;
    acenderLed('g');
    // Ao terminar o homing, troca para o verde por 3 segundos
    wait(2.0); // mantém o verde por 3 segundos
    // Desliga todos os LEDs e continua o programa
    ledVermelho = ledVerde = ledAmarelo = 0;
    acenderLed('y'); // piscar ele

    configurarSistema();
    printLCD("   Deseja iniciar   ", 0);
    printLCD("      o ciclo ?      ", 1);
    confirmado = false;
    while (!confirmado) {
      wait(0.1);
    }
    printLCD("Executando...", 0);
    acenderLed('y'); // liga apenas o amarelo
    wait(1);

    // 2) Deslocamento para cada tubo usando moverInterpoladoXY fixo
    for (int i = 0; i < quantidadeTubos; ++i) {
      char buf[21]; // LCD 16x2: 16 chars + '\0'
      // Linha 0: número do tubo
      sprintf(buf, "Executando tubo %d", i + 1);
      printLCD(buf, 0);
      printLCD("mL: 0", 1);
      for (int ml = 0; ml < tubos[i].volumeML; ++ml) {
        mover_Z(0);                                   // zerando eixo Z
        moverInterpoladoXY(posBecker.x, posBecker.y); // movendo pro becker
        wait(0.1);
        mover_Z(posBecker.z); // move o Z
        wait(0.1);
        coleta_liberacao(); // coletando do becker
        wait(2);
        mover_Z(0); // zera o Z
        wait(0.1);
        moverInterpoladoXY(tubos[i].pos.x, tubos[i].pos.y);
        wait(0.1); // indo até o primeiro tubo
        mover_Z(tubos[i].pos.z);
        wait(0.1);
        coleta_liberacao(); // dispensando
        wait(2);
        mover_Z(0); // zera o Z
        sprintf(buf, "mL: %d", ml + 1);
        printLCD(buf, 1);
        wait(0.2);
      }
      wait(0.5);
    }
    mover_Z(0);
    moverInterpoladoXY(10000, 231500);
    // Som e mensagem para sinalizar término
    acenderLed('g'); // liga apenas o verde
    printLCD(" Processo concluido ", 0);
    printLCD("    com sucesso!    ", 1);
    buzzer = 1;
    wait(2); // 2s de buzzer ligado
    buzzer = 0;
  }
}
