//Servo
#include <Servo.h>
Servo escFL_H;


//Working:  1, 2, 3, 4, 5, 6, 7, 8

//Macros
#define BAUDRATE      115200
#define SPEED_LIMIT   1.00
#define MINPULS       700
#define MAXPULS       2000
#define pinFL_H       3

//Globals
const uint16_t NEUTRAL = (MAXPULS + MINPULS) / 2;

//Functions
void serial_recv(uint16_t *varA, uint16_t *varB, uint16_t *varC, uint16_t *varD, uint16_t *varE, uint16_t *varF, uint16_t *varG, uint16_t *varH) {
  char serial_in[25] = "000000000000000000000000";
  if(Serial.available()) {
  Serial.readBytes(serial_in, 24);
  *varA = ((serial_in[0] - '0') * 100) + ((serial_in[1] - '0') * 10) + (serial_in[2] - '0') - 100;
  *varB = ((serial_in[3] - '0') * 100) + ((serial_in[4] - '0') * 10) + (serial_in[5] - '0') - 100;
  *varC = ((serial_in[6] - '0') * 100) + ((serial_in[7] - '0') * 10) + (serial_in[8] - '0') - 100;
  *varD = ((serial_in[9] - '0') * 100) + ((serial_in[10] - '0') * 10) + (serial_in[11] - '0') - 100;
  *varE = ((serial_in[12] - '0') * 100) + ((serial_in[13] - '0') * 10) + (serial_in[14] - '0') - 100;
  *varF = ((serial_in[15] - '0') * 100) + ((serial_in[16] - '0') * 10) + (serial_in[17] - '0') - 100;
  *varG = ((serial_in[18] - '0') * 100) + ((serial_in[19] - '0') * 10) + (serial_in[20] - '0') - 100;
  *varH = ((serial_in[21] - '0') * 100) + ((serial_in[22] - '0') * 10) + (serial_in[23] - '0') - 100;
  }
}


void setup() {
  Serial.begin(BAUDRATE);
  escFL_H.attach(3, MINPULS, MAXPULS);
  escFL_H.writeMicroseconds(NEUTRAL);
  //analogReadResolution(10);
  //SerialUSB.println("Begin");

}

void loop() {

  //Serial Recieve
  uint16_t Lx, Ly, Rx, Ry, LT, RT, LB, RB;
  //serial_recv(&Lx, &Ly, &Rx, &Ry, &LT, &RT, &LB, &RB);


  int throttle = analogRead(5);
  throttle = map(throttle, 0, 1023, 0, 179);
  Serial.println(throttle);
  Serial.println(throttle);
  escFL_H.write(throttle);
  delay(1);

  

  

}
