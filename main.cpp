#include "IHM.h"
#include "JOG.h"
#include "Referenciamento.h"
#include "Salvar_Posicoes_Volume.h"
#include "TextLCD.h"
#include "botao_emergencia.h"
#include "mbed.h"
#include "printLCD.h"
#include "Acionapipeta.h"

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
  // configuracão LCD
  lcd.setCursor(TextLCD::CurOff_BlkOff);
  lcd.setBacklight(TextLCD::LightOn);

  // configuracão botão de emergencia
  botaoEmergencia.mode(PullDown);
  botaoEmergencia.fall(&modoEmergencia);

  // 1) configura encoder + botões
  setupEncoder();

  // 2) homing dos eixos X e Y
  Enable = 0;
  printLCD("Refer. eixo X...", 0);
  referenciar_EixoX();
  printLCD("Refer. eixo Y...", 0);
  referenciar_EixoY();
  Enable = 1;

  configurarSistema();
  printLCD("Executando...", 0);
  wait(1);


  // 2) Deslocamento para cada tubo usando moverInterpoladoXY fixo
  for (int i = 0; i < quantidadeTubos; ++i) {
    int alvoX = tubos[i].pos.x;
    int alvoY = tubos[i].pos.y;
    char buf[21];  // LCD 16x2: 16 chars + '\0'
    // Linha 0: número do tubo
    sprintf(buf, "Executando tubo %d", i + 1);
    printLCD(buf, 0);
    printLCD("Ml: 0", 1);
    for (int ml = 0 ; ml < tubos[i].volumeML; ++ml){
        moverInterpoladoXY(posBecker.x, posBecker.y);
        wait(0.5);
        moverInterpoladoXY(alvoX, alvoY);
        wait(0.2);
        coleta_liberacao();
        sprintf(buf, "Ml: %d", ml + 1);
        printLCD(buf, 1);
        wait(0.2);
    }
    wait(0.5);
  }
  // Som e mensagem para sinalizar término
  printLCD(" Processo concluido ", 0);
  printLCD("    com sucesso!    ", 1);
  buzzer = 1;
  wait_ms(100); // 100 ms de buzzer ligado
  buzzer = 0;
}
