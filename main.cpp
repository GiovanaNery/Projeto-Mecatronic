#include "mbed.h"
#include "mbed2/299/drivers/DigitalIn.h"

DigitalOut myled(LED1);

DigitalIn But(PC_13);

int main() {
  while(1) {
    myled = 1;
    wait(0.2);
    myled = 0;
    wait(0.2);
  }
}
