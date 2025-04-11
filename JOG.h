#ifndef JOG_H
#define JOG_H

#include "mbed.h"
#include <string>

// declaração de funções e variaveis
// funções direções dos eixos 
void z(float direcao);
void x(float direcao);
void y(float direcao);
// funções para desligar o motor
void desliga_motor_z();
void desliga_motor_x();
void desliga_motor_y();
//funções para mover motor por interpolação
void moverInterpoladoXY();
void modoPosicionamentoManual();

#endif
        