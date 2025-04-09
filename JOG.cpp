#include "JOG.h"

float tempo;  //tempo para os eixos X e Y
float tempo_eixoz;  //tempo para o eixo Z

// Definindo os pinos do motor de passo de cada eixo (X, Y e Z)
BusOut MOTOR_Y(PC_5, PC_6, PC_7, PC_8);
BusOut MOTOR_X(PB_1, PB_13, PB_14, PB_15);
BusOut MOTOR_Z(PA_8, PA_9, PA_10, PA_11);

// Subir (Z menos--> zero do eixo Z encima)
void z_menos()
{
    for (int i = 3; i > -1; i--) {
        MOTOR_Z = 1 << i;
        wait(tempo_z);
    }
}

// Descer (Z mais--> zero do eixo Z encima)
void z_mais()
{
    for (int i = 0; i < 4; i++) {
        MOTOR_Z = 1 << i;
        wait(tempo_z);
    }
}

// Esquerda (X menos)
void x_menos()
{
    for (int i = 0; i < 4; i++) {
        MOTOR_X = 1 << i;
        wait(tempo);
    }
}

// Direita (X mais)
void x_mais()
{
    for (int i = 3; i > -1; i--) {
        MOTOR_X = 1 << i;
        wait(tempo);
    }
}

// Frente (Y menos)
void y_menos()
{
    for (int i = 0; i < 4; i++) {
        MOTOR_Y = 1 << i;
        wait(tempo);
    }
}

// Trás (Y mais)
void y_mais()
{
    for (int i = 3; i > -1; i--) {
        MOTOR_Y = 1 << i;
        wait(tempo);
    }
}
