#include "Acionapipeta.h"
#include "IHM.h"
#include "JOG.h"
#include "Referenciamento.h"
#include "Salvar_Posicoes_Volume.h"
#include "TextLCD.h"
#include "botao_emergencia.h"
#include "cmsis_nvic.h" // necessário para NVIC_SystemReset()
#include "mbed.h"
#include "printLCD.h"

extern DigitalOut ledVermelho;
extern DigitalOut ledAmarelo;
extern DigitalOut ledVerde;
extern DigitalIn botaoZmais; // Pressionado = HIGH (sem pull-down)
extern DigitalIn botaoZmenos;
extern DigitalIn botaoEmergencia;
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

  // configura encoder + botões
  setupEncoder();

  // configuracão botão de emergencia
  // if (botaoEmergencia = 1) {
  // modoEmergencia();
  //}

  // Mostra o nome do sistema por 0.5s
  if (botaoEmergencia == 0) {
    printLCD("     PIPETRONIX     ", 0);
    printLCD("                    ", 1);
    printLCD("                    ", 2);
    wait(2); // Espera 0.5 segundos
    // 1) perguntamos se quer iniciar o referenciamento
    printLCD("   Pressione para   ", 0);
    printLCD("     iniciar o      ", 1);
    printLCD("  referenciamento   ", 2);
    while (!confirmado) {
      led = !led;
      wait(0.5);
      // espera o encoder ser pressionado
    }
    wait_ms(300); // debounce
  }

  // 2) homing dos eixos X e Y
  if (botaoEmergencia == 0) {
    acenderLed('y'); // liga apenas o amarelo
    Enable = 0;
    referenciar_EixoZ();
    referenciar_EixoX();
    referenciar_EixoY();
    Enable = 1;
  }
  if (botaoEmergencia == 0) {
    // Ao terminar o homing, troca para o verde por 3 segundos
    acenderLed('g'); // liga apenas o verde
  }
  if (botaoEmergencia == 0) {
    wait(2.0); // mantém o verde por 3 segundos
    // Desliga todos os LEDs e continua o programa
    ledVermelho = ledVerde = ledAmarelo = 0;
  }

  if (botaoEmergencia == 0) {
    configurarSistema();
  }

  if (botaoEmergencia == 0) {
    printLCD("Executando...", 0);
    acenderLed('y'); // liga apenas o amarelo
    wait(1);
  }

  // 2) Deslocamento para cada tubo usando moverInterpoladoXY fixo
  if (botaoEmergencia == 0) {
    for (int i = 0; i < quantidadeTubos; ++i) {
      if (botaoEmergencia.read() == 1) {
        break; // Interrompe o laço imediatamente
      }
      char buf[21]; // LCD 16x2: 16 chars + '\0'
      // Linha 0: número do tubo
      sprintf(buf, "Executando tubo %d", i + 1);
      printLCD(buf, 0);
      printLCD("mL: 0", 1);
      for (int ml = 0; ml < tubos[i].volumeML; ++ml) {
        if (botaoEmergencia.read() == 1) {
          break; // Interrompe o laço imediatamente
        }
        if (botaoEmergencia == 0) {
          mover_Z(0);
        }
        if (botaoEmergencia == 0) {                     // zerando eixo Z
          moverInterpoladoXY(posBecker.x, posBecker.y); // movendo pro becker
        }
        if (botaoEmergencia == 0) {
          wait(0.1);
          mover_Z(posBecker.z); // move o Z
        }
        if (botaoEmergencia == 0) {
          wait(0.1);
          coleta_liberacao(); // coletando do becker
        }
        if (botaoEmergencia == 0) {
          wait(2);
          mover_Z(0); // zera o Z
        }
        if (botaoEmergencia == 0) {
          wait(0.1);
          moverInterpoladoXY(tubos[i].pos.x, tubos[i].pos.y);
        }
        if (botaoEmergencia == 0) {
          wait(0.1); // indo até o primeiro tubo
          mover_Z(tubos[i].pos.z);
        }
        if (botaoEmergencia == 0) {
          wait(0.1);
          coleta_liberacao(); // dispensando
        }
        if (botaoEmergencia == 0) {
          wait(2);
          mover_Z(0); // zera o Z
        }
        if (botaoEmergencia == 0) {
          sprintf(buf, "mL: %d", ml + 1);
          printLCD(buf, 1);
          wait(0.2);
        }
      }
      wait(0.5);
    }
  }

  if (botaoEmergencia == 0) {
    // Som e mensagem para sinalizar término
    acenderLed('g'); // liga apenas o verde
    printLCD(" Processo concluido ", 0);
    printLCD("    com sucesso!    ", 1);
    buzzer = 1;
    wait(2); // 2s de buzzer ligado
    buzzer = 0;
  }

  if (botaoEmergencia == 0) {
    confirmado = false;
    // 8) Pergunta reinício do processo
    printLCD(" Deseja reiniciar", 0);
    printLCD("    o processo?", 1);
    while (!confirmado) {
      // espera nova confirmação
      wait(0.5);
    }
    wait_ms(300);
    // 9) Reinicia sistema
    NVIC_SystemReset();
    ledVerde = 0;
  }

  // configuracão botão de emergencia
  if (botaoEmergencia == 1) {
    modoEmergencia();
  }
}
