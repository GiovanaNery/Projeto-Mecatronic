#include "JOG.h"
#include "mbed.h"
#include "Referenciamento.h"
Ponto3D = {0, 0, 0};  // x = 0, y = 0, z = 0
struct Ponto3D {
    int x = 0;
    int y = 0;
    int z = 0;
};


int main() {
    Ponto3D
    // Inicializa o sistema
    referenciar_EixoZ();   // <- função que você criou com setup de posição e volume

    // Aqui você pode chamar a execução da pipetagem (se tiver feito essa função)
    // executarPipetagem();

    while (true) {
        // Aguarda ou roda tarefas repetitivas
    }
}
