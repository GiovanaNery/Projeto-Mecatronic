#ifndef IHM_H
#define IHM_H

#include "mbed.h"
//#include "TextLCD.h"
#include <string>
#include <cstring>

// Declaração do barramento I2C (caso também esteja em outro .cpp)
//extern I2C i2c_lcd;

// Declaração externa do LCD
extern float velocidade_jog;
extern bool confirmado_5seg;

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
void verificarPressionamentoLongo();
#endif