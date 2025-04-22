
// Simulação de funções de motores (você pode conectar a motores reais)
// === Definição dos pinos ===
DigitalIn botaoEmergencia(D2);
DigitalIn botaoConfirma(D3);

DigitalOut ledVermelho(PC_10);
DigitalOut buzzer(PB_5);

TextLCD lcd(PA_0, PA_1, PA_4, PB_0, PC_1, PC_0); // Ajuste os pinos conforme seu circuito

DigitalOut STEP_X(D4), STEP_Y(D5), STEP_Z(D6);

bool emergenciaAtiva = false;

// === Parar motores ===
void pararMotores() {
    STEP_X = 0;
    STEP_Y = 0;
    STEP_Z = 0;
}

// === Função de emergência NR-12 ===
void modoEmergencia() {
    emergenciaAtiva = true;
    pararMotores();

    buzzer = 1;
    lcd.cls();
    lcd.printf("ATENCAO:\nOperacao Interrompida!");
    wait(2);
    lcd.cls();
    lcd.printf("MODO DE\nEMERGENCIA");

    while (botaoEmergencia == 0);  // Espera botão ser liberado

    lcd.cls();
    lcd.printf("Confirmar\nSaida Emerg.?");
    while (botaoConfirma == 1);
    wait_ms(300);  // debounce

    buzzer = 0;
    ledVermelho = 0;

    lcd.cls();
    lcd.printf("Reiniciar\nProcesso?");
    while (botaoConfirma == 1);
    wait_ms(300);

    emergenciaAtiva = false;
    lcd.cls();
    lcd.printf("Sistema Ativo");
}
