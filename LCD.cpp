#include "mbed.h"
#include "TextLCD.h"
TextLCD lcd(D8, D9, D4, D5, D6, D7); //rs,e,d0,d1,d2,d3
int main()
{
lcd.printf("Hello World!");
}
