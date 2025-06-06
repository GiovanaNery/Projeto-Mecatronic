#include "Salvar_Posicoes_Volume.h"
#include "mbed.h"
#include "printLCD.h"

// Definição dos pinos para os LEDs de status
DigitalOut ledVermelho(A2);  // LED vermelho conectado ao pino A2
DigitalOut ledAmarelo(A4);   // LED amarelo conectado ao pino A4
DigitalOut ledVerde(A5);     // LED verde conectado ao pino A5

// Pino da chave seletora (usada para ajustar a velocidade no modo manual)
DigitalIn seletor(PC_4);      // Chave seletora conectada ao pino PC_4

// Definição dos pinos dos botões para controle do eixo Z
DigitalIn botaoZmais(PC_2);   // Botão que movimenta Z no sentido positivo (conectado a PC_2)
DigitalIn botaoZmenos(PC_3);  // Botão que movimenta Z no sentido negativo (conectado a PC_3)

// Definição dos pinos do driver do motor do eixo Z
DigitalOut passoZ(D6);        // Pino STEP do motor Z conectado ao D6
DigitalOut direcaoZ(D5);      // Pino DIR do motor Z conectado ao D5

// Definição dos pinos do encoder rotativo
InterruptIn encoderCLK(PB_12);   // Sinal A (CLK) do encoder conectado ao PB_12
DigitalIn encoderDT(PB_11);      // Sinal B (DT) do encoder conectado ao PB_11
InterruptIn encoderBotao(PA_11); // Botão integrado do encoder (leitura manual via interrupção) no pino PA_11

// Declaração externa do botão de emergência (definido em outro módulo)
extern DigitalIn botaoEmergencia;

// === VARIÁVEIS GLOBAIS ===
volatile int encoderValor = 1;     // Valor atual selecionado pelo encoder (começa em 1)
volatile int contadorCliques = 0;  // Contador intermediário para detectar passo do encoder
volatile bool confirmado = false;  // Flag que indica se o usuário confirmou a seleção
int valorAnterior = encoderValor;  // Armazena valor anterior para controle de atualização de display
bool ultimoEstadoBotao = 1;        // Estado anterior do botão do encoder (1 = não pressionado)

// === Rotina de Interrupção para leitura do giro do encoder ===
void encoderGiro() {
    // Quando o encoder gira, o sinal CLK muda. Comparamos CLK e DT para saber a direção.
    // Se CLK ≠ DT, assumimos que girou no sentido horário e incrementamos contadorCliques.
    if (encoderCLK.read() != encoderDT.read()) {
        contadorCliques++;
    } else {
        // Caso CLK == DT, girou no sentido anti-horário, decrementamos contadorCliques.
        contadorCliques--;
    }

    // Quando contadorCliques acumula +2, incrementamos encoderValor (um “passo” completo)
    if (contadorCliques >= 2) {
        encoderValor++;       // Ajusta valor para cima
        contadorCliques = 0;  // Reseta contador para começar novo ciclo
    }
    // Quando contadorCliques chega a -2, decrementamos encoderValor
    else if (contadorCliques <= -2) {
        encoderValor--;       // Ajusta valor para baixo
        contadorCliques = 0;  // Reseta contador
    }
}

// === Função de Interrupção para botão de confirmação do encoder ===
void aoConfirmar() {
    confirmado = true; // Seta flag de confirmado quando o botão do encoder é pressionado
}

// === Função principal que exibe menu de seleção de volume usando o encoder no LCD ===
// Parâmetros:
// - mensagem: texto a ser exibido na linha 0 do LCD para instruir o usuário
// - valorInicial: valor a iniciar no encoder
// - minValor: valor mínimo permitido
// - maxValor: valor máximo permitido
// - ind: indicador para decidir se exibe “Volume: X mL” (ind == 0) ou apenas valor numérico (ind != 0)
int selecionarVolumeEncoder(const char *mensagem, int valorInicial,
                            int minValor, int maxValor, int ind) {
    encoderValor = valorInicial;  // Inicia encoderValor com o valorInicial
    contadorCliques = 0;          // Reseta contador de cliques
    confirmado = false;           // Reseta flag de confirmação
    int valorAnterior = -1;       // Força atualização imediata no LCD (diferente de encoderValor)

    printLCD(mensagem, 0);        // Exibe mensagem na linha 0 do LCD

    // Loop até usuário confirmar (confirmado == true) ou botão de emergência ser pressionado
    while (!confirmado && botaoEmergencia == 0) {
        // Garante que encoderValor fique dentro dos limites mínimo e máximo
        if (encoderValor < minValor)
            encoderValor = minValor;
        else if (encoderValor > maxValor)
            encoderValor = maxValor;

        // Se o valor mudou em relação à última iteração, atualiza display
        if (encoderValor != valorAnterior) {
            char buffer[20];
            if (ind == 0) {
                // Formato com “Volume: X mL ”
                sprintf(buffer, "Volume: %d mL ", encoderValor);
            } else {
                // Formato apenas valor numérico
                sprintf(buffer, "%d ", encoderValor);
            }
            printLCD(buffer, 1);    // Exibe o buffer na linha 1 do LCD
            valorAnterior = encoderValor; // Armazena valor atual como anterior
        }

        wait_ms(10); // Pequeno atraso para debouncing do encoder
    }

    confirmado = false; // Reseta a flag para próxima chamada
    return encoderValor; // Retorna o valor selecionado pelo usuário
}

// === Função de configuração inicial do encoder ===
void setupEncoder() {
    encoderDT.mode(PullUp);             // Configura entrada DT com resistor PullUp
    encoderBotao.mode(PullUp);          // Configura botão do encoder com PullUp
    encoderCLK.fall(&encoderGiro);      // Dispara encoderGiro() na borda de descida de CLK
    encoderBotao.fall(&aoConfirmar);    // Dispara aoConfirmar() na borda de descida do botão
}

// Variável externa para controlar a velocidade no modo manual (definida em outro módulo)
extern float velocidade_jog;

// === Função que lê a chave seletora para ajustar velocidade de jog (modo manual) ===
void chaveseletora() {
    if (seletor == 1) {
        // Se seletor estiver em 1, velocidade menor (devagar)
        velocidade_jog = (0.001f / 16.0f);
    } else {
        // Caso contrário, velocidade maior (rápido)
        velocidade_jog = (0.0004f / 16.0f);
    }
}

// === Função para acender somente o LED correspondente à cor passada ===
// Parâmetro:
// - cor: caractere indicando LED (‘r’ = vermelho, ‘g’ = verde, ‘y’ = amarelo)
void acenderLed(char cor) {
    // Apaga todos os LEDs primeiro
    ledVermelho = 0;
    ledVerde = 0;
    ledAmarelo = 0;

    // Acende apenas o LED correspondente à inicial fornecida
    if (cor == 'r') {
        ledVermelho = 1; // Acende LED vermelho
    } else if (cor == 'g') {
        ledVerde = 1;    // Acende LED verde
    } else if (cor == 'y') {
        ledAmarelo = 1;  // Acende LED amarelo
    }
}