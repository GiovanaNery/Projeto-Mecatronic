#ifndef JOG_H
#define JOG_H

#include "mbed.h"
//#include "TextLCD.h"
#include <string>
#include <cstring> 
extern DigitalOut Enable;
extern float tempo;
extern float tempo_interpolado;
// declaração de funções e variaveis
// funções direções dos eixos 
void z(int direcao, float velocidade);
void x(int direcao , float velocidade);
void y(int direcao , float velocidade);
// funções para desligar o motor
void desliga_motor_z();
void desliga_motor_x();
void desliga_motor_y();
void pararMotores();
//funções para mover motor por interpolação
void moverInterpoladoXY(int x1, int y1);
void modoPosicionamentoManual();

#endif
        