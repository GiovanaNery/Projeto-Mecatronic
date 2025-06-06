#ifndef IHM_H
#define IHM_H

#include "mbed.h"
//#include "TextLCD.h"
#include <string>
#include <cstring>

extern float velocidade_jog;

// declaração de funções e variaveis
void encoderGiro();
void aoConfirmar(); 
void setupEncoder();
void setupEncoder();
int selecionarVolumeEncoder(const char *mensagem, int valorInicial, int minValor, int maxValor, int ind);
void printLCD(const char* texto, int linha);
void chaveseletora();
void acenderLed(char cor); 
void controlarEixoZ();
#endif