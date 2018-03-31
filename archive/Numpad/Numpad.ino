/* @file CustomKeypad.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates changing the keypad size and key values.
|| #
*/
#include <Keypad.h>

#define ROWS  4
#define COLS  3

char hexa_keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'D','0','E'}
};

byte row_pins[ROWS] = {34, 32, 30, 28};
byte col_pins[COLS] = {26, 24, 22};

Keypad Numpad = Keypad(makeKeymap(hexa_keys), row_pins, col_pins, ROWS, COLS); 

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char key_in = Numpad.getKey();
  
  if (key_in){
    Serial.println(key_in);
  }
}
