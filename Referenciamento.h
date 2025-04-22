#ifndef Referenciamento_H
#define Referenciamento_H

#include "mbed.h"
#include <string>
#include "TextLCD.h"

// declaração de funções e variaveis
void referenciar_EixoZ();
void referenciar_EixoX();
void referenciar_EixoY();

extern int passos_X;
extern int passos_Y;
extern int passos_Z;
#endif
        