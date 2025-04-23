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
  Ponto3D posteste = {0, 0, 0};
  modoPosicionamentoManual(posteste); // função implementada em outro cpp
  while (true) {
    x(10);
    y(-10);
  }
}
