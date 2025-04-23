#include "IHM.h"
#include "JOG.h"
#include "Referenciamento.h"
#include "TextLCD.h"
#include "mbed.h"
#include "printLCD.h"

// Struct local (caso você ainda não tenha separado)
struct Ponto3D {
  int x;
  int y;
  int z;
};

// 1) Instancia o barramento I2C em D14=SDA, D15=SCL
I2C i2c_lcd(D14, D15);
// 2) Cria o objeto LCD na linha I2C, endereço 0x4E, 20×4
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD20x4);

// Função para testar impressão no LCD
void printLCD(const char *texto, int linha) {
  lcd.locate(0, linha);
  for (int i = 0; i < lcd.columns(); i++)
    lcd.putc(' ');
  lcd.locate(0, linha);
  lcd.printf("%s", texto);
}

int main() {
  lcd.setCursor(TextLCD::CurOff_BlkOff);
  lcd.setBacklight(TextLCD::LightOn);
  // 1) configura encoder + botões
  setupEncoder();

  // 2) homing dos eixos X e Y
  Enable = 0 ;
  printLCD("Refer. eixo X...", 0);
  referenciar_EixoX();
  printLCD("Refer. eixo Y...", 0);
  referenciar_EixoY();
  Enable = 1;

  // laço principal
  while (true) {
    // 3) seleciona volume via encoder
    int volume = selecionarVolumeEncoder("Volume (mL):", 5, 0, 100, 0);

    // 4) posiciona em X/Y com o joystick
    Ponto3D pos = {0, 0, 0};
    printLCD("Use joystick...", 0);
    printLCD("", 1);
    Enable = 0; // habilita driver
    modoPosicionamentoManual(pos);
    Enable = 1; // desabilita driver

    // 5) exibe confirmação final
    {
      char buf[32];
      sprintf(buf, "Pronto! %d mL", volume);
      printLCD(buf, 0);
    }
    {
      char buf2[32];
      sprintf(buf2, "X=%d Y=%d", pos.x, pos.y);
      printLCD(buf2, 1);
    }
    wait_ms(2000);
  }
}

