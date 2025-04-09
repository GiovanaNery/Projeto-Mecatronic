#include "AcionamentoMotor.h"
int tempo_pega = 2;

DigitalOut Acionar(PC_9);

// função para acionar a pipeta
void acionamento()
{
    Acionar = 1;
    wait_ms(2000);
    Acionar = 0;
    wait(tempo_pega);
}
            