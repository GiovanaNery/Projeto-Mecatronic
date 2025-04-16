#ifndef IHM_H
#define IHM_H

#include "mbed.h"
#include <string>
#include <cstring>

// declaração de funções e variaveis
void encoderGiro();
void aoConfirmar(); 
void setupEncoder();
void setupEncoder();
int selecionarVolumeEncoder(const char *mensagem, int valorInicial, int minValor, int maxValor);
void chaveseletora(DigitalIn &seletor, float &tempo);
void acenderLed(char cor); 
void piscarLed(char cor, bool& parar);

#endif