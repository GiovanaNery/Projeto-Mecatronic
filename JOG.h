#ifndef JOG_H
#define JOG_H

#include "mbed.h"
#include <string>

// declaração de funções e variaveis
// funções direções dos eixos 
void z_menos();
void z_mais();
void x_menos();
void x_mais();
void y_menos();
void y_mais();
// funções para desligar o motor
void z_desliga();
void x_desliga();
void y_desliga();

#endif
        