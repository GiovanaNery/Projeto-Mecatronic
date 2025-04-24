#ifndef SalvarPosicoesVolume_H
#define SalvarPosicoesVolume_H

#include "mbed.h"
#include <string>
#include <cstring>
//#include "TextLCD.h"
#include "IHM.h"
// Definições de struct (podem ficar aqui ou num outro header próprio)
struct Ponto3D {
    int x, y, z;
};

struct Tubo {
    Ponto3D pos;
    int volumeML;
};

// Declarações extern
extern const int MAX_TUBOS;
extern Tubo tubos[];            // o tamanho é dado por MAX_TUBOS
extern int quantidadeTubos;
extern Ponto3D posBecker;
extern int volumeBeckerML;

// declaração de funções e variaveis
void configurarSistema();

#endif