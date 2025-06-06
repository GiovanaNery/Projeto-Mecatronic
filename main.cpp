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

extern DigitalOut ledVermelho;       // LED vermelho (externo, declarado em outro módulo)
extern DigitalOut ledAmarelo;        // LED amarelo (externo, declarado em outro módulo)
extern DigitalOut ledVerde;          // LED verde (externo, declarado em outro módulo)
extern DigitalIn botaoZmais;         // Botão Z+ (input digital; HIGH quando pressionado, sem pull-down)
extern DigitalIn botaoZmenos;        // Botão Z– (input digital; HIGH quando pressionado)
extern DigitalIn botaoEmergencia;    // Botão de emergência (externo, declarado em outro módulo)

DigitalOut led(PA_5);    // LED de status genérico conectado ao pino PA_5
DigitalOut startstop(PB_1); // Pino de controle start/stop (externo) no pino PB_1

// 1) Instancia o barramento I2C em D14=SDA, D15=SCL
I2C i2c_lcd(D14, D15);
// 2) Cria o objeto LCD na linha I2C, endereço 0x4E, 20×4 caracteres
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD20x4);

// Função para imprimir texto no LCD
// Parâmetros:
// - texto: string a ser exibida
// - linha: número da linha (0 a 3) em que o texto deve aparecer
void printLCD(const char *texto, int linha) {
    // Se for escrever na linha 0, limpa todo o display antes
    if (linha == 0) {
        lcd.cls();          // Limpa o conteúdo do LCD
    }
    // Posiciona o cursor na coluna 0 da linha especificada
    lcd.locate(0, linha);
    // Escreve a string no LCD, a partir da posição definida
    lcd.printf("%s", texto);
}

int main() {
    // --- Configuração inicial do LCD ---
    lcd.setCursor(TextLCD::CurOff_BlkOff);  // Desliga o cursor e o bloco de cursor
    lcd.setBacklight(TextLCD::LightOn);     // Liga a luz de fundo do LCD

    // Configura o encoder (rotativo) e seus botões
    setupEncoder();

    // Exibe texto de teste no LCD (linha 0)
    printLCD(" aa", 0);

    // Se o botão de emergência não estiver pressionado (botaoEmergencia == 0), garante startstop em LOW
    if (botaoEmergencia == 0) {
        startstop = 0;  // Define pino startstop em 0 (ativo-baixo ou reset dependendo da lógica)
    }

    // --- Comentário: seções duplicadas foram comentadas, então usamos apenas as linhas acima ---

    // Exibe nome do sistema no LCD e aguarda interação
    if (botaoEmergencia == 0) {
        printLCD("     PIPETRONIX     ", 0); // Linha 0: exibe “PIPETRONIX” centralizado
        wait(2);                           // Espera 2 segundos (2000 ms)

        // 1) Pergunta ao usuário se quer iniciar o referenciamento dos eixos
        printLCD("   Pressione para   ", 0);     // Linha 0: instrução “Pressione para”
        printLCD("     iniciar o      ", 1);     // Linha 1: instrução “iniciar o”
        printLCD("  referenciamento   ", 2);     // Linha 2: instrução “referenciamento”

        confirmado = false;                     // Reseta flag de confirmação do encoder
        // Aguarda até que o encoder seja pressionado (aoConfirmar seta confirmado=true)
        while (!confirmado) {
            led = !led;    // Pisca o LED para indicar espera (inverte estado a cada 0.5 s)
            wait(0.5);     // Delay de 0.5 segundos
        }
        wait_ms(300);      // Debounce: espera 300 ms antes de prosseguir

        // 2) Executa homing/referenciamento dos eixos X, Y e Z se não houver emergência
        if (botaoEmergencia == 0) {
            acenderLed('y');   // Acende apenas o LED amarelo para indicar “em processo”
            Enable = 0;        // Habilita o driver de motores (nível ativo-baixo)

            referenciar_EixoZ();   // Executa rotina de referenciamento do eixo Z
            referenciar_EixoXY();  // Executa rotina de referenciamento dos eixos X e Y

            Enable = 1;        // Desabilita o driver após referenciamento
        }

        // Se não houve emergência, sinaliza término do homing com LED verde
        if (botaoEmergencia == 0) {
            acenderLed('g');   // Acende apenas o LED verde
        }
        if (botaoEmergencia == 0) {
            wait(2.0);         // Mantém o LED verde aceso por 2 segundos
            // Desliga todos os LEDs após o período de sinalização
            ledVermelho = ledVerde = ledAmarelo = 0;
        }

        // 3) Chama rotina de configuração da pipetadora (configurarSistema) se não houver emergência
        if (botaoEmergencia == 0) {
            configurarSistema();
        }

        // 4) Pergunta ao usuário se quer iniciar o processo de pipetagem
        if (botaoEmergencia == 0) {
            confirmado = false;
            printLCD("   Pressione para   ", 0);    // Linha 0: instrução “Pressione para”
            printLCD(" iniciar o processo", 1);    // Linha 1: instrução “iniciar o processo”
            // Aguarda confirmação do encoder novamente
            while (!confirmado) {
                wait(0.5);    // Delay para não travar o loop
            }
        }

        // 5) Inicia processo de pipetagem
        if (botaoEmergencia == 0) {
            printLCD("Executando...", 0);   // Exibe “Executando...” no LCD
            acenderLed('y');               // Acende LED amarelo para indicar que está em execução
            wait(1);                       // Aguarda 1 segundo antes de continuar
        }

        // 6) Laço principal de deslocamento e coleta para cada tubo definido
        if (botaoEmergencia == 0) {
            for (int i = 0; i < quantidadeTubos; ++i) {
                // Se a qualquer momento o botão de emergência for pressionado, interrompe o loop
                if (botaoEmergencia.read() == 1) {
                    break; // Sai do for imediatamente
                }
                char buf[21]; // Buffer para exibir mensagens no LCD (20 chars + '\0')

                // Exibe número do tubo atual na linha 0 do LCD
                sprintf(buf, "Executando tubo %d", i + 1);
                printLCD(buf, 0);

                // Inicializa a linha 1 com “mL: 0”
                printLCD("mL: 0", 1);

                // Para cada mL de volume definido para o tubo, executa sequência de movimento e dispensação
                for (int ml = 0; ml < tubos[i].volumeML; ++ml) {
                    // Verifica emergência antes de cada etapa
                    if (botaoEmergencia.read() == 1) {
                        break; // Sai do laço interno se emergência for acionada
                    }
                    // 6.1) Zera o eixo Z (mover até 0) para iniciar cada ciclo de coleta
                    if (botaoEmergencia == 0) {
                        mover_Z(0);
                    }
                    // 6.2) Move a pipeta para a posição do béquer
                    if (botaoEmergencia == 0) {
                        moverInterpoladoXY(posBecker.x, posBecker.y);
                    }
                    // 6.3) Desce Z até a altura do béquer (posBecker.z)
                    if (botaoEmergencia == 0) {
                        wait(0.1);
                        mover_Z(posBecker.z);
                    }
                    // 6.4) Executa coleta e liberação da pipeta no béquer
                    if (botaoEmergencia == 0) {
                        wait(0.1);
                        coleta_liberacao(); // Função que realiza a sucção/travamento de líquido
                    }
                    // 6.5) Aguarda 2.5 segundos para completar coleta, depois zera Z
                    if (botaoEmergencia == 0) {
                        wait(2.5);
                        mover_Z(0);
                    }
                    // 6.6) Move interpolado até a posição do tubo atual
                    if (botaoEmergencia == 0) {
                        wait(0.1);
                        moverInterpoladoXY(tubos[i].pos.x, tubos[i].pos.y);
                    }
                    // 6.7) Desce Z até a altura do tubo (tubos[i].pos.z)
                    if (botaoEmergencia == 0) {
                        wait(0.1);
                        mover_Z(tubos[i].pos.z);
                    }
                    // 6.8) Dispensa o volume para o tubo usando coleta_liberacao()
                    if (botaoEmergencia == 0) {
                        wait(0.1);
                        coleta_liberacao();
                    }
                    // 6.9) Aguarda 2.5 segundos para completar dispensação, depois zera Z
                    if (botaoEmergencia == 0) {
                        wait(2.5);
                        mover_Z(0);
                    }
                    // 6.10) Atualiza no LCD o volume atual dispensado (“mL: X”)
                    if (botaoEmergencia == 0) {
                        sprintf(buf, "mL: %d", ml + 1);
                        printLCD(buf, 1);  // Linha 1: exibe o contador de mL
                        wait(0.2);         // Aguarda 200 ms antes do próximo passo
                    }
                }
                wait(0.5); // Pequena pausa após terminar o tubo antes de ir para o próximo
            }
        }

        // 7) Finaliza o processo após todos os tubos ou emergência
        if (botaoEmergencia == 0) {
            mover_Z(0);                        // Garante que Z fique na posição zero
            moverInterpoladoXY(10000, 10000); // Move XY para uma posição “home” segura

            // Sinaliza término com LED verde e mensagem no LCD
            acenderLed('g'); // Liga apenas o LED verde
            printLCD(" Processo concluido ", 0);  // Linha 0: “Processo concluido”
            printLCD("    com sucesso!    ", 1);  // Linha 1: “com sucesso!”
            buzzer = 1;         // Liga o buzzer para sinal sonoro
            wait(2);            // Mantém buzzer ligado por 2 segundos
            buzzer = 0;         // Desliga o buzzer
        }

        // 8) Pergunta se usuário quer reiniciar o processo
        if (botaoEmergencia == 0) {
            confirmado = false;
            printLCD("   Pressione para   ", 0);       // Linha 0: instrução “Pressione para”
            printLCD("reiniciar o processo", 1);       // Linha 1: instrução “reiniciar o processo”
            // Aguarda confirmação do encoder
            while (!confirmado) {
                wait(0.5);
            }
            wait_ms(300);     // Debounce
            // 9) Executa reset do sistema
            NVIC_SystemReset(); // Reinicia todo o microcontrolador
            ledVerde = 0;       // Desliga LED verde (não será alcançado se reset ocorrer imediatamente)
        }
    }

    // Configuração do modo de emergência: se o botão de emergência estiver pressionado
    if (botaoEmergencia == 1) {
        modoEmergencia(); // Chama rotina de emergência
    }
}