#include "mbed.h"
#include "Acionapipeta.h"

int tempo_pega = 2;

// Definindo os pinos 
DigitalOut Acionar(PC_9);

// função para acionar a pipeta
void coleta_liberacao()
{
    Acionar = 0; //botao apertado
    wait(tempo_pega);
    Acionar = 1; 
    wait_ms(2000);
    
}