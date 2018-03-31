//ROV Control 2018
//Onboard Arduino Code
//Tim Brewis



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
#define MINPULS 1000
#define MAXPULS 2000
#define pinFL_H 2
#define pinFR_H 3
#define pinBL_H 4
#define pinBR_H 5
#define pinFL_V 6
#define pinFR_V 7
#define pinBL_V 8
#define pinBR_V 9

//-------------------------------------------------------------Global Variables

	//ESC pulselengths
  uint16_t neutral = (MAXPULS + MINPULS) / 2; 


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

	escFL_H.attach(pinFL_H, MINPULS, MAXPULS);
  escFR_H.attach(pinFR_H, MINPULS, MAXPULS);
  escBL_H.attach(pinBL_H, MINPULS, MAXPULS);
  escBR_H.attach(pinBR_H, MINPULS, MAXPULS);
  escFL_V.attach(pinFL_V, MINPULS, MAXPULS);
  escFR_V.attach(pinFR_V, MINPULS, MAXPULS);
  escBL_V.attach(pinBL_V, MINPULS, MAXPULS);
  escBR_V.attach(pinBR_V, MINPULS, MAXPULS);

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



//-------------------------------------------------------------Motor Calculations


  int Lx = 0;
  int Ly = 0;
  int Rx = 0;
  int Ry = 0;
  int LT = 0;
  int RT = 0;
  int LB = 0;
  int RB = 0;

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
