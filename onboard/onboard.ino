//ROV Control 2018
//Onboard Arduino Code
//Tim Brewis



#include <LiquidCrystal.h>
LiquidCrystal LCD(12,11,5,4,3,2);


//Servo
#include <Servo.h>
Servo escFL_H;    //FL-H means front, left, horizontal
Servo escFR_H;
Servo escFL_V;
Servo escFR_V;
Servo escBL_H;
Servo escBR_H;
Servo escBL_V;
Servo escBR_V;

//-------------------------------------------------------------Macros
#define minpuls 1000
#define maxpuls 2000

//-------------------------------------------------------------Global Variables
	//Servo
	int pinFL_H = 2;
	int pinFR_H = 3;
	int pinBL_H = 4;
	int pinBR_H = 5;
	int pinFL_V = 6;
	int pinFR_V = 7;
	int pinBL_V = 8;
	int pinBR_V = 9;  
	//ESC pulselengths
  int minpuls = 1000;      //these values should be tuned such that they are at the absolute limits of effective throttle
  int maxpuls = 2000;      //Sidenote: maxes should be at a level that does not exceed maximum permitted current draw. (2.5A/motor ABSOLUTE MAX)
  int neutral = (maxpuls + minpuls) / 2; //once tuned, neutral may not be perfectly in the middle. Hence the separate variable


	//Serial
	char serial_in[24];
	int serial_vals[8];

	//Motor speed calculations
  float straferate =  0.25;
  float speedFL_H, speedFR_H = 0;
  float speedFL_V, speedFR_V = 0;
  float speedBL_H, speedBR_H = 0;
  float speedBL_V, speedBR_V = 0;
  float pfwdbwd, pleftright, pvert, proll, ppitch = 0;
	


//-------------------------------------------------------------Setup
void setup() {

	Serial.begin(115200);
  LCD.begin(16,2);

	escFL_H.attach(pinFL_H, minpuls, maxpuls);
  escFR_H.attach(pinFR_H, minpuls, maxpuls);
  escBL_H.attach(pinBL_H, minpuls, maxpuls);
  escBR_H.attach(pinBR_H, minpuls, maxpuls);
  escFL_V.attach(pinFL_V, minpuls, maxpuls);
  escFR_V.attach(pinFR_V, minpuls, maxpuls);
  escBL_V.attach(pinBL_V, minpuls, maxpuls);
  escBR_V.attach(pinBR_V, minpuls, maxpuls);

  escFL_H.write(neutral);
  escFR_H.write(neutral);
  escBL_H.write(neutral);
  escBR_H.write(neutral);
  escFL_V.write(neutral);
  escFR_V.write(neutral);
  escBL_V.write(neutral);
  escBR_V.write(neutral);

}



//-------------------------------------------------------------Main Loop
void loop() {


//-------------------------------------------------------------Serial Comms
  for(int i = 0; i < 8; i++) {
    serial_vals[i] = (serial_in[3*i] - '0') * 100 + (serial_in[3*i + 1] - '0') * 10 + (serial_in[3*i + 2] - '0');
  }

//-------------------------------------------------------------Motor Calculations
  //Convert variables back to their actual value
  for(int i = 0; i < 8; i++) {
  	serial_vals[i] -= 200;
  }
  serial_vals[5] = serial_vals[5] * -1;

  int Lx = serial_vals[0];
  int Ly = serial_vals[1];
  int Rx = serial_vals[2];
  int Ry = serial_vals[3];
  int LT = serial_vals[4];
  int RT = serial_vals[5];
  int LB = serial_vals[6];
  int RB = serial_vals[7];

  //Get current stick values, convert to float between -1 and 1
  float fwdbwd = (float)Ly / 100;;
  float leftright = (float)Lx / 100;
  float vert = ((float)RT / 100) + ((float)LT / 100);
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
  if(LB) {
    speedFL_H -= straferate;
    speedFR_H += straferate;
    speedBL_H -= straferate;
    speedBR_H += straferate;
  }
  if(RB) {
    speedFL_H += straferate;
    speedFR_H -= straferate;
    speedBL_H += straferate;
    speedBR_H -= straferate;
  }

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
  if(fwdbwd == 0 && leftright == 0) {
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
  escFL_H.write(percentToServo(speedFL_H));
  escFR_H.write(percentToServo(speedFR_H));
  escFL_V.write(percentToServo(speedFL_V));
  escFR_V.write(percentToServo(speedFR_V));
  escBL_H.write(percentToServo(speedBL_H));
  escBR_H.write(percentToServo(speedBR_H));
  escBL_V.write(percentToServo(speedBL_V));
  escBR_V.write(percentToServo(speedBR_V));


//-------------------------------------------------------------Loop Cleanup
  delay(1);

  LCD.setCursor(0,0);
  for(int i = 0; i < 12; i++) {
    LCD.setCursor(0,i);
    LCD.write(char(serial_in[i]));
  }
  delay(100);
  LCD.clear();
  
//END MAIN LOOP
}



//-------------------------------------------------------------Functions
int percentToServo(float percent) {

  int negmax = 0;
  int neut = 90;
  int posmax = 179;
  int serVal = 0;

  if(percent > 0) {
    serVal = map(percent, 0, 100, neut, posmax);
    return serVal;
  }
  if(percent < 0) {
    serVal = map(percent, -100, 0, negmax, neut);
    return serVal;
  }
  else {
    serVal = neut;
    return serVal;
  }
}
