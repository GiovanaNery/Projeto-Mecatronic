#include "mbed.h"
#include "Acionapipeta.h"

int tempo_pega = 2;

// Definindo os pinos 
DigitalOut Acionar(PC_9);

// função para acionar a pipeta
void coleta_liberacao()
{
    Acionar = 1; //botao apertado
    wait_ms(2000);
    Acionar = 0; 
    wait(tempo_pega);
    
}