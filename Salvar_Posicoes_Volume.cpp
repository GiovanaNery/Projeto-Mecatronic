#include "IHM.h"
#include "JOG.h"
#include "Referenciamento.h"
#include "mbed.h"
#include "printLCD.h"

extern DigitalIn botaoEmergencia;

// === ESTRUTURAS ===
struct Ponto3D {
  int x, y, z;
};

struct Tubo {
  Ponto3D pos;
  int volumeML;
};

const int MAX_TUBOS = 100;
Tubo tubos[MAX_TUBOS];
int quantidadeTubos = 0;

Ponto3D posBecker;
int volumeBeckerML = 0;

/// === FLUXO DE CONFIGURAÇÃO DO SISTEMA ===
void configurarSistema() {
    // Configura o encoder e seus interrupções
    setupEncoder();

    // 1. POSICIONAR BÉQUER
    // Exibe mensagem no LCD solicitando ao usuário mover a pipeta até o béquer
    printLCD("    >> Bequer <<", 0);   // Linha 0: título “>> Bequer <<”
    printLCD("   Mova a pipeta", 1);   // Linha 1: instrução para mover a pipeta
    printLCD("    para coleta", 2);    // Linha 2: instrução complementar

    // Entra em modo de posicionamento manual (joystick) até o usuário confirmar posição
    modoPosicionamentoManual();

    // Armazena as coordenadas atuais (número de passos X, Y, Z) na posição do béquer
    posBecker.x = passos_X;
    posBecker.y = passos_Y;
    posBecker.z = passos_Z;

    // 3. DEFINIR QUANTIDADE DE TUBOS
    // Exibe menu no LCD para selecionar a quantidade de tubos (1 a 100)
    quantidadeTubos =
        selecionarVolumeEncoder("Quantidade de tubos:", 1, 1, 100, 1);
    // Parâmetros:
    // - Mensagem: "Quantidade de tubos:"
    // - Valor inicial: 1
    // - Mínimo: 1
    // - Máximo: 100
    // - ind = 1 (faz exibir “X ” em vez de “Volume: X mL”)

    // 4. DEFINIR POSIÇÃO E VOLUME DE CADA TUBO
    // Para cada tubo, enquanto não houver emergência
    for (int i = 0; i < quantidadeTubos && botaoEmergencia == 0; i++) {
        char buf[21];
        // Prepara a string “>> Tubo N <<” para exibir no LCD
        sprintf(buf, "    >> Tubo %d <<", i + 1);
        printLCD(buf, 0);            // Linha 0: “>> Tubo N <<”
        printLCD("   Mova a pipeta", 1); // Linha 1: instrução para mover a pipeta
        printLCD("   para este tubo", 2); // Linha 2: instrução para este tubo específico

        // Aguarda o usuário posicionar a pipeta sobre o tubo via joystick
        modoPosicionamentoManual();

        // Armazena a posição atual (passos X, Y, Z) no vetor de tubos
        tubos[i].pos.x = passos_X;
        tubos[i].pos.y = passos_Y;
        tubos[i].pos.z = passos_Z;

        // Solicita ao usuário volume em mL para o tubo atual:
        // - Usa buf (que contém “>> Tubo N <<”) como mensagem
        // - Valor inicial: 1 mL
        // - Mínimo: 1 mL
        // - Máximo: 100 mL
        // - ind = 0 (exibe “Volume: X mL”)
        tubos[i].volumeML = selecionarVolumeEncoder(buf, 1, 1, 100, 0);
    }
}
