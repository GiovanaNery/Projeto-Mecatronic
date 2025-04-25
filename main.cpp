#include "IHM.h"
#include "JOG.h"
#include "Referenciamento.h"
#include "TextLCD.h"
#include "mbed.h"
#include "printLCD.h"
#include "botao_emergencia.h"
#include "Salvar_Posicoes_Volume.h"

// 1) Instancia o barramento I2C em D14=SDA, D15=SCL
I2C i2c_lcd(D14, D15);
// 2) Cria o objeto LCD na linha I2C, endereço 0x4E, 20×4
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD20x4);

// Função para testar impressão no LCD
// Exemplo usando TextLCD do Mbed
void printLCD(const char* texto, int linha) {
    // só limpa o LCD quando for escrever na linha 0
    if (linha == 0) {
        lcd.cls();        // limpa tudo
    }
    lcd.locate(0, linha); // posiciona no início da coluna 0, linha especificada
    lcd.printf("%s", texto);
}


int main() {
  //configuracão LCD
  lcd.setCursor(TextLCD::CurOff_BlkOff);
  lcd.setBacklight(TextLCD::LightOn);

  //configuracão botão de emergencia
  botaoEmergencia.mode(PullDown);
  botaoEmergencia.fall(&modoEmergencia);  

  // 1) configura encoder + botões
  setupEncoder();

  // 2) homing dos eixos X e Y
  Enable = 0 ;
  printLCD("Refer. eixo X...", 0);
  referenciar_EixoX();
  printLCD("Refer. eixo Y...", 0);
  referenciar_EixoY();
  Enable = 1;

  configurarSistema();
  printLCD("Executando...",0);
  moverInterpoladoXY(posBecker.x, posBecker.y);


    // 2) Deslocamento para cada tubo usando moverInterpoladoXY fixo
    for (int i = 0; i < quantidadeTubos; ++i) {
        int alvoX = tubos[i].pos.x;
        int alvoY = tubos[i].pos.y;
        moverInterpoladoXY(alvoX, alvoY);
        wait(1);
        moverInterpoladoXY(posBecker.x, posBecker.y);
        // se tiver função de dispensar volume, chame aqui:
        // dispensarVolume(tubos[i].volumeML);

        wait(1);
    }
    printLCD(" Processo concluido ", 0);
    printLCD("    com sucesso!    ", 1);
}

