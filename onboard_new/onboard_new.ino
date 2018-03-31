//Servo
#include <Servo.h>
Servo escFL_H;		Servo escFL_V;
Servo escFR_H;		Servo escFR_V;
Servo escBL_H;		Servo escBL_V;
Servo escBR_H;		Servo escBR_V;

//Macros
#define BAUDRATE      	115200
#define SPEED_LIMIT   	1.0//factor to limit motor speed
#define MINPULS       	1064
#define MAXPULS       	1864
#define pinFL_H			6 			//connected to ESC 8 
#define pinFR_H			7 			//connected to ESC 7
#define pinBL_H			8			//connected to ESC 6
#define pinBR_H			9			//connected to ESC 5
#define pinFL_V			10          //connected to ESC 4
#define pinFR_V			11          //connected to ESC 3
#define pinBL_V			12 			//connected to ESC 2
#define pinBR_V			13 			//connected to ESC 1

#define NEUTRAL_FL_H    1464 //117
#define NEUTRAL_FR_H    1464
#define NEUTRAL_BL_H    1464//111
#define NEUTRAL_BR_H    1464
#define NEUTRAL_FL_V    1464
#define NEUTRAL_FR_V    1464   //107
#define NEUTRAL_BL_V    1464
#define NEUTRAL_BR_V    1464

//Globals
const uint16_t NEUTRAL = (MAXPULS + MINPULS) / 2;

  float straferate =  0.25;
  float speedFL_H, speedFR_H = 0;
  float speedFL_V, speedFR_V = 0;
  float speedBL_H, speedBR_H = 0;
  float speedBL_V, speedBR_V = 0;
  float pfwdbwd, pleftright, pvert, proll, ppitch, pstrafe = 0;
  int16_t Lx = 0;
  int16_t Ly = 0;
  int16_t Rx = 0;
  int16_t Ry = 0;
  int16_t LT = 0;
  int16_t RT = 0;
  int16_t LB = 0;
  int16_t RB = 0;;


//Functions
void serial_recv(int16_t *varA, int16_t *varB, int16_t *varC, int16_t *varD, int16_t *varE, int16_t *varF, int16_t *varG, int16_t *varH) {
  char serial_in[25] = "000000000000000000000000";
  if(Serial5.available()) {
  Serial5.readBytes(serial_in, 24);
  *varA = ((serial_in[0] - '0') * 100) + ((serial_in[1] - '0') * 10) + (serial_in[2] - '0') - 100;
  *varB = ((serial_in[3] - '0') * 100) + ((serial_in[4] - '0') * 10) + (serial_in[5] - '0') - 100;
  *varC = ((serial_in[6] - '0') * 100) + ((serial_in[7] - '0') * 10) + (serial_in[8] - '0') - 100;
  *varD = ((serial_in[9] - '0') * 100) + ((serial_in[10] - '0') * 10) + (serial_in[11] - '0') - 100;
  *varE = ((serial_in[12] - '0') * 100) + ((serial_in[13] - '0') * 10) + (serial_in[14] - '0') - 100;
  *varF = ((serial_in[15] - '0') * 100) + ((serial_in[16] - '0') * 10) + (serial_in[17] - '0') - 100;
  *varG = ((serial_in[18] - '0') * 100) + ((serial_in[19] - '0') * 10) + (serial_in[20] - '0') - 100;
  *varH = ((serial_in[21] - '0') * 100) + ((serial_in[22] - '0') * 10) + (serial_in[23] - '0') - 100;
  //SerialUSB.println(serial_in);
  }
}

uint16_t percent_to_servo(float percent, uint16_t neutral) {

  uint16_t minimum = 1064;
  uint16_t maximum = 1864;
  uint16_t servo_val = 0;

  if(percent > 0) {
    servo_val = map(percent * 100, 0, 100, neutral, neutral + 400);
    //SerialUSB.println(servo_val);
    return servo_val;
  }
  if(percent < 0) {
    servo_val = map(percent * 100, -100, 0, neutral - 400, neutral);
    //SerialUSB.println(servo_val);
    return servo_val;
  }
  else {
    servo_val = neutral;
    //SerialUSB.println(servo_val);
    return servo_val;
  }
}




//--------------------------------------------------------------------------------Setup
void setup() {
  Serial5.begin(BAUDRATE);
  SerialUSB.begin(BAUDRATE);
  escFL_H.attach(pinFL_H, MINPULS, MAXPULS);
  escFR_H.attach(pinFR_H, MINPULS, MAXPULS);
  escBL_H.attach(pinBL_H, MINPULS, MAXPULS);
  escBR_H.attach(pinBR_H, MINPULS, MAXPULS);
  escFL_V.attach(pinFL_V, MINPULS, MAXPULS);
  escFR_V.attach(pinFR_V, MINPULS, MAXPULS);
  escBL_V.attach(pinBL_V, MINPULS, MAXPULS);
  escBR_V.attach(pinBR_V, MINPULS, MAXPULS);
  escFL_H.writeMicroseconds(NEUTRAL_FL_H);
  escFR_H.writeMicroseconds(NEUTRAL_FR_H);
  escBL_H.writeMicroseconds(NEUTRAL_BL_H);
  escBR_H.writeMicroseconds(NEUTRAL_BR_H);
  escFL_V.writeMicroseconds(NEUTRAL_FL_V);
  escFR_V.writeMicroseconds(NEUTRAL_FR_V);
  escBL_V.writeMicroseconds(NEUTRAL_BL_V);
  escBR_V.writeMicroseconds(NEUTRAL_BR_V);
}





//--------------------------------------------------------------------------------Loop
void loop() {

  //Serial Recieve
  
  serial_recv(&Ry, &LT, &RT, &LB, &RB, &Lx, &Ly, &Rx);
  //serial_recv(&Lx, &Ly, &Rx, &Ry, &LT, &RT, &LB, &RB);
  /*SerialUSB.println(Lx);  SerialUSB.println(Ly);
  SerialUSB.println(Rx);  SerialUSB.println(Ry);
  SerialUSB.println(LT);  SerialUSB.println(RT);
  SerialUSB.println(LB);  SerialUSB.println(RB);
  SerialUSB.println();*/

//Calculate motor percentage speeds
//Float conversion
  float fwdbwd = (float)Ly / 100;;
  float leftright = (float)Lx / 100;
  float vert = ((float)RT / 100) + ((float)LT / -100);
  float pitch = (float)Ry / 100;
  float roll = (float)Rx / 100;


  //Horizontal forwards, backwards, left & right
  speedFL_H -= (fwdbwd - pfwdbwd);
  speedFL_H -= (leftright - pleftright);
  speedFR_H -= (fwdbwd - pfwdbwd);
  speedFR_H += (leftright - pleftright);
  speedBL_H += (fwdbwd - pfwdbwd);
  speedBL_H -= (leftright - pleftright);
  speedBR_H += (fwdbwd - pfwdbwd);
  speedBR_H += (leftright - pleftright);

  //Yaw


  //Altitude
  speedFL_V += (vert - pvert);
  speedFR_V += (vert - pvert);
  speedBL_V += (vert - pvert);
  speedBR_V += (vert - pvert);

  //Pitch
  speedFL_V -= (pitch - ppitch);
  speedFR_V -= (pitch - ppitch);
  speedBL_V += (pitch - ppitch);
  speedBR_V += (pitch - ppitch);

  //Roll
  speedFL_V += (roll - proll);
  speedFR_V -= (roll - proll);
  speedBL_V += (roll - proll);
  speedBR_V -= (roll - proll);

  //Max check
  if(speedFL_H > 1) {speedFL_H = 1;}
  if(speedFR_H > 1) {speedFR_H = 1;}
  if(speedBL_H > 1) {speedBL_H = 1;}
  if(speedBR_H > 1) {speedBR_H = 1;}
  if(speedFL_H < -1) {speedFL_H = -1;}
  if(speedFR_H < -1) {speedFR_H = -1;}
  if(speedBL_H < -1) {speedBL_H = -1;}
  if(speedBR_H < -1) {speedBR_H = -1;}
  if(speedFL_V > 1) {speedFL_V = 1;}
  if(speedFR_V > 1) {speedFR_V = 1;}
  if(speedBL_V > 1) {speedBL_V = 1;}
  if(speedBR_V > 1) {speedBR_V = 1;}
  if(speedFL_V < -1) {speedFL_V = -1;}
  if(speedFR_V < -1) {speedFR_V = -1;}
  if(speedBL_V < -1) {speedBL_V = -1;}
  if(speedBR_V < -1) {speedBR_V = -1;}

  //Setting past variables for next loop
  pfwdbwd = fwdbwd;
  pleftright = leftright;
  pvert = vert;
  proll = roll;
  ppitch = pitch;

  //Make sure everything returns to zero
  if(fwdbwd == 0 && leftright == 0 && LB == 0 && RB == 0) {
    speedFL_H = 0;
    speedFR_H = 0;
    speedBL_H = 0;
    speedBR_H = 0;
    pfwdbwd = 0;
    pleftright = 0;
  }
  if(vert == 0 && roll == 0 && pitch == 0) {
    speedFL_V = 0;
    speedFR_V = 0;
    speedBL_V = 0;
    speedBR_V = 0;
    pvert = 0;
    proll = 0;
    ppitch = 0;
  }


//-------------------------------------------------------------ESC Write
  escFL_H.writeMicroseconds(percent_to_servo(speedFL_H * SPEED_LIMIT, NEUTRAL_FL_H));
  escFR_H.writeMicroseconds(percent_to_servo(speedFR_H * SPEED_LIMIT, NEUTRAL_FR_H));
  escBL_H.writeMicroseconds(percent_to_servo(speedBL_H * SPEED_LIMIT, NEUTRAL_BL_H));
  escBR_H.writeMicroseconds(percent_to_servo(speedBR_H * SPEED_LIMIT, NEUTRAL_BR_H));

  escFL_V.writeMicroseconds(percent_to_servo(speedFL_V * SPEED_LIMIT, NEUTRAL_FL_V));
  escFR_V.writeMicroseconds(percent_to_servo(speedFR_V * SPEED_LIMIT, NEUTRAL_FR_V));
  escBL_V.writeMicroseconds(percent_to_servo(speedBL_V * SPEED_LIMIT, NEUTRAL_BL_V));
  escBR_V.writeMicroseconds(percent_to_servo(speedBR_V * SPEED_LIMIT, NEUTRAL_BR_V));
 


//---------------------------------------------------------------Debug
  
  
  
  SerialUSB.print("\n\n    ");
  SerialUSB.print(percent_to_servo(speedFL_H, NEUTRAL_FL_H));
  SerialUSB.print("   ");
  SerialUSB.println(percent_to_servo(speedFR_H, NEUTRAL_FR_H));
  SerialUSB.print(percent_to_servo(speedFL_V, NEUTRAL_FL_V));
  SerialUSB.print("            ");
  SerialUSB.println(percent_to_servo(speedFR_V, NEUTRAL_FR_V));
  SerialUSB.println("\n\n\n");
  SerialUSB.print(percent_to_servo(speedBL_V, NEUTRAL_BL_V));
  SerialUSB.print("            ");
  SerialUSB.println(percent_to_servo(speedBR_V, NEUTRAL_BR_V));
  SerialUSB.print("    ");
  SerialUSB.print(percent_to_servo(speedBL_H, NEUTRAL_BL_H));
  SerialUSB.print("   ");
  SerialUSB.print(percent_to_servo(speedBR_H, NEUTRAL_BR_H));
//End main loop
}
