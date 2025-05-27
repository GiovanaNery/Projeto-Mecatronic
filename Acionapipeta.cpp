#include "mbed.h"
#include "Acionapipeta.h"

// Definindo os pinos 
DigitalOut Acionar(PC_6);

// função para acionar a pipeta
void coleta_liberacao()
{
    Acionar = 0; //botao apertado
    wait(1);
    Acionar = 1; 
    wait(1);
    
}


