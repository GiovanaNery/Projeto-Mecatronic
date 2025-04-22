#include "Salvar_Posicoes_Volume.h"
#include "TextLCD.h"
#include "mbed.h"
#include "printLCD.h"

// Pinos para os LEDs
DigitalOut ledVermelho(D2);
DigitalOut ledVerde(D3);
DigitalOut ledAmarelo(D4);

// Pino fixo da chave seletora
DigitalIn seletor(D5);
// Pinos dos botões
DigitalIn botaoZmais(D2); //z no sentido positivo
DigitalIn botaoZmenos(D3); //z no sentido negativo
// Pinos do motor - drivers 
DigitalOut passoZ(D4);
DigitalOut direcaoZ(D5);


// === PINOS DO ENCODER ===
InterruptIn encoderCLK(D6);     // sinal A
DigitalIn encoderDT(D7);        // sinal B
InterruptIn encoderBotao(D8);   // botão (leitura manual)

// === VARIÁVEIS ===
volatile int encoderValor = 1;
volatile int contadorCliques = 0;
volatile bool confirmado = false;
int valorAnterior = encoderValor;
bool ultimoEstadoBotao = 1;  // botão não pressionado

// === Função de leitura de giro ===
void encoderGiro() {
    if (encoderCLK.read() == encoderDT.read()) {
        contadorCliques++;
    } else {
        contadorCliques--;
    }

    if (contadorCliques >= 5) {
        encoderValor++;
        contadorCliques = 0;
    } else if (contadorCliques <= -5) {
        encoderValor--;
        contadorCliques = 0;
    }
}

// === Função de confirmação via botão ===
void aoConfirmar() {
    confirmado = true;
}

// === Função principal de seleção com LCD ===
int selecionarVolumeEncoder(const char *mensagem, int valorInicial, int minValor, int maxValor) {
    encoderValor = valorInicial;
    contadorCliques = 0;
    confirmado = false;
    int valorAnterior = encoderValor;

    printLCD(mensagem, 0); // mostra mensagem na linha 0

    while (!confirmado) {
        if (encoderValor < 0) {
            encoderValor = 0;
        }

        if (encoderValor != valorAnterior) {
            char buffer[20];
            sprintf(buffer, "Volume: %d mL", encoderValor);
            printLCD(buffer, 1); // mostra valor na linha 1
            valorAnterior = encoderValor;
        }

        wait_ms(10); // Anti-repique leve
    }

    confirmado = false;
    return encoderValor;
}

// === Setup inicial ===
void setupEncoder() {
    encoderDT.mode(PullUp);
    encoderBotao.mode(PullUp);
    encoderCLK.fall(&encoderGiro);      // gira
    encoderBotao.fall(&aoConfirmar);    // botão confirma
}


// chave seletora - para definir a velocidade que quero usar 
void chaveseletora(float &tempo) {
    if (seletor == 0) {
        tempo = 0.01;  // Devagar
    } else {
        tempo = 0.002; // Rápido
    }
}

// acender LEDS por cor 
void acenderLed(char cor) {  //acender apenas indicando apenas a incial correspondente a cor
    // Apaga todos
    ledVermelho = 0;
    ledVerde = 0;
    ledAmarelo = 0;

    // Acende o LED escolhido
    if (cor == 'r') {
        ledVermelho = 1;
    } else if (cor == 'g') {
        ledVerde = 1;
    } else if (cor == 'y') {
        ledAmarelo = 1;
    }
}

// piscar LEDS por cor
void piscarLed(char cor, bool& parar) {
    DigitalOut* led = NULL;

    if (cor == 'r') led = &ledVermelho;
    else if (cor == 'g') led = &ledVerde;
    else if (cor == 'y') led = &ledAmarelo;

    if (led != NULL) { //led liga e desliga de 0.3 em 0.3
        while (!parar) {
            *led = 1;  //*led acende o valor apontado, ou seja, o LED que foi escolhido com base no char cor.
            wait(0.3);       // LED ligado 
            *led = 0;
            wait(0.3);       // LED desligado
        }
        *led = 0; // Garante que o LED fique apagado no final
    }
}
//COMO USAR A FUNCAO:
//pararPiscar = false;
//piscarLed('r', pararPiscar); // Enquanto pisca led vermelho, outra lógica pode rodar em paralelo
// Quando quiser parar:
//pararPiscar = true;


// Botão de movimentacão do eixo Z 
void controlarEixoZ() {
    // Configura pull-up interno (botão fechado para GND)
    botaoZmais.mode(PullUp);
    botaoZmenos.mode(PullUp);

    while (true) {
        if (botaoZmais == 0) {
            direcaoZ = 1;        // Sentido positivo
            passoZ = 1;
            wait_us(500);
            passoZ = 0;
            wait_us(500);
        } 
        else if (botaoZmenos == 0) {
            direcaoZ = 0;        // Sentido negativo
            passoZ = 1;
            wait_us(500);
            passoZ = 0;
            wait_us(500);
        } 
        else {
            // Nenhum botão pressionado → motor parado
            passoZ = 0;
        }
    }
}
