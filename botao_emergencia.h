#ifndef BOTAO_EMERGENCIA_H
#define BOTAO_EMERGENCIA_H

#include "mbed.h"

// Declaração dos pinos e variáveis globais relacionados à emergência
extern DigitalIn botaoEmergencia;
extern DigitalOut buzzer;
extern bool confirmado;
extern bool emergenciaAtiva;
extern bool pararPiscar;

// Funções
void modoEmergencia();
void iniciarPiscarLedVermelho();

#endif
