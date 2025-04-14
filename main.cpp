#include "mbed.h"
#include "Referenciamento.h"

int main() {
    // Inicializa o sistema
    referenciar_EixoZ();   // <- função que você criou com setup de posição e volume

    // Aqui você pode chamar a execução da pipetagem (se tiver feito essa função)
    // executarPipetagem();

    while (true) {
        // Aguarda ou roda tarefas repetitivas
    }
}
