//RGSea 2018 Onboard Control Code
//Tim Brewis




//----------------------------------------------------------------------Libraries
  
  //Servo
  #include <Servo.h>
  Servo escFL_H;		      Servo escFL_V;
  Servo escFR_H;		      Servo escFR_V;
  Servo escBL_H;		      Servo escBL_V;
  Servo escBR_H;		      Servo escBR_V;

//----------------------------------------------------------------------Macros

  #define BAUDRATE      	115200
  #define MINPULS       	1064
  #define MAXPULS       	1864
  #define NEUTRAL         1464
  #define pinFL_H			    6 			        //connected to ESC 8 
  #define pinFR_H		    	7 			        //connected to ESC 7
  #define pinBL_H			    8			          //connected to ESC 6
  #define pinBR_H			    9			          //connected to ESC 5
  #define pinFL_V			    10              //connected to ESC 4
  #define pinFR_V			    11              //connected to ESC 3
  #define pinBL_V			    12 			        //connected to ESC 2
  #define pinBR_V			    13 			        //connected to ESC 1

//----------------------------------------------------------------------Global Variables

  //Motor speeds
  float speedFL_H = 0;    float speedFR_H = 0;
  float speedFL_V = 0;    float speedFR_V = 0;
  float speedBL_H = 0;    float speedBR_H = 0;
  float speedBL_V = 0;    float speedBR_V = 0;

  //Control input
  int16_t Lx = 0;         int16_t Ly = 0;
  int16_t Rx = 0;         int16_t Ry = 0;
  int16_t LT = 0;         int16_t RT = 0;
  int16_t LB = 0;         int16_t RB = 0;

  //Control algorithm
  float pfwdbwd = 0;      float pleftright = 0;
  float pvert = 0;        float proll = 0;
  float ppitch = 0;       float pstrafe = 0;
  float straferate =  0.25;

  //Motor management
  uint8_t horizontal = 0;
  uint8_t vertical = 0;
  char priority = 'n';
  float SPEED_LIMIT = 0.50;




//----------------------------------------------------------------------Functions

  void serial_recv(int16_t *varA, int16_t *varB, int16_t *varC, int16_t *varD, int16_t *varE, int16_t *varF, int16_t *varG, int16_t *varH) {
  
    //Create input buffer
    char serial_in[25] = "000000000000000000000000";
  
    //Read data
    if(Serial5.available()) {
      
      Serial5.readBytes(serial_in, 24);
  
      //Error detection
      uint8_t start_pos = 0;
      uint8_t read_pos[24];
  
      for(uint8_t i = 0; i < 25; i++) {
        if(serial_in[i] == 'i') {start_pos = i + 1;}
      }
  
      for(uint8_t i = 0; i < 25; i++) {
        if((start_pos + i) < 24) {
          read_pos[i] = start_pos + i;
        }
        else {
          read_pos[i] = start_pos + i - 25;
        }
      }
  
      //Split buffer into variables
      *varA = ((serial_in[read_pos[0]] - '0') * 100) + ((serial_in[read_pos[1]] - '0') * 10) + (serial_in[read_pos[2]] - '0') - 100;
      *varB = ((serial_in[read_pos[3]] - '0') * 100) + ((serial_in[read_pos[4]] - '0') * 10) + (serial_in[read_pos[5]] - '0') - 100;
      *varC = ((serial_in[read_pos[6]] - '0') * 100) + ((serial_in[read_pos[7]] - '0') * 10) + (serial_in[read_pos[8]] - '0') - 100;
      *varD = ((serial_in[read_pos[9]] - '0') * 100) + ((serial_in[read_pos[10]] - '0') * 10) + (serial_in[read_pos[11]] - '0') - 100;
      *varE = ((serial_in[read_pos[12]] - '0') * 100) + ((serial_in[read_pos[13]] - '0') * 10) + (serial_in[read_pos[14]] - '0') - 100;
      *varF = ((serial_in[read_pos[15]] - '0') * 100) + ((serial_in[read_pos[16]] - '0') * 10) + (serial_in[read_pos[17]] - '0') - 100;
      *varG = ((serial_in[read_pos[18]] - '0'));
      *varH = ((serial_in[read_pos[19]] - '0'));
      SPEED_LIMIT = ((float((serial_in[read_pos[20]] - '0') * 100)) + float(((serial_in[read_pos[21]] - '0') * 10)) + float(((serial_in[read_pos[22]] - '0')))) / 100;
    
    }
    else {
      *varA = *varB = *varC = *varD = *varF = *varG = *varH = 0;
    }
  
  }

  uint16_t percent_to_servo(float percent) {
  
    uint16_t servo_val = 0;
  
    if(percent > 0) {
      servo_val = map(percent * 100, 0, 100, NEUTRAL, NEUTRAL + 400);
      return servo_val;
    }
    if(percent < 0) {
      servo_val = map(percent * 100, -100, 0, NEUTRAL - 400, NEUTRAL);
      return servo_val;
    }
    else {
      servo_val = neutral;
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
  
  escFL_H.writeMicroseconds(NEUTRAL);
  escFR_H.writeMicroseconds(NEUTRAL);
  escBL_H.writeMicroseconds(NEUTRAL);
  escBR_H.writeMicroseconds(NEUTRAL);
  escFL_V.writeMicroseconds(NEUTRAL);
  escFR_V.writeMicroseconds(NEUTRAL);
  escBL_V.writeMicroseconds(NEUTRAL);
  escBR_V.writeMicroseconds(NEUTRAL);
  
}




//--------------------------------------------------------------------------------Main Loop
  void loop() {
  
  
  //----------------------------------------------------------------------Receive Serial Data
    
    serial_recv(&Lx, &Ly, &Rx, &Ry, &LT, &RT, &LB, &RB);
  
  
  //----------------------------------------------------------------------Control Algorithm
    
    //Float conversion
    float fwdbwd = (float)Ly / 100;;
    float leftright = (float)Lx / 100;
    float vert = ((float)RT / 100) + ((float)LT / -100);
    float pitch = (float)Ry / 100;
    float roll = (float)Rx / 100;
  
    //Forwards, backwards, left & right
    speedFL_H -= (fwdbwd - pfwdbwd);
    speedFL_H -= (leftright - pleftright);
    speedFR_H -= (fwdbwd - pfwdbwd);
    speedFR_H += (leftright - pleftright);
    speedBL_H += (fwdbwd - pfwdbwd);
    speedBL_H -= (leftright - pleftright);
    speedBR_H += (fwdbwd - pfwdbwd);
    speedBR_H += (leftright - pleftright);
  
    //Yaw
       //-----------------------------------------!!
  
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
    if(speedFL_H > 1)  {speedFL_H = 1;}
    if(speedFR_H > 1)  {speedFR_H = 1;}
    if(speedBL_H > 1)  {speedBL_H = 1;}
    if(speedBR_H > 1)  {speedBR_H = 1;}
    if(speedFL_V > 1)  {speedFL_V = 1;}
    if(speedFR_V > 1)  {speedFR_V = 1;}
    if(speedBL_V > 1)  {speedBL_V = 1;}
    if(speedBR_V > 1)  {speedBR_V = 1;}
    if(speedFL_H < -1) {speedFL_H = -1;}
    if(speedFR_H < -1) {speedFR_H = -1;}
    if(speedBL_H < -1) {speedBL_H = -1;}
    if(speedBR_H < -1) {speedBR_H = -1;}
    if(speedFL_V < -1) {speedFL_V = -1;}
    if(speedFR_V < -1) {speedFR_V = -1;}
    if(speedBL_V < -1) {speedBL_V = -1;}
    if(speedBR_V < -1) {speedBR_V = -1;}
  
    //Set past variables for next loop
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
  
    //----------------------------------------------------------------------Motor Management
    
    if(speedFL_H != 0 || speedFR_H != 0 || speedBL_H != 0 || speedBR_H != 0) {horizontal = 1;}
    else {horizontal = 0;}
    if(speedFL_V != 0 || speedFR_V != 0 || speedBL_V != 0 || speedBR_V != 0) {vertical = 1;}
    else {vertical = 0;}
    if(horizontal == 0 && vertical == 0) {priority == 'n';}
    if(priority = 'n') {
      if(horizontal == 1) {priority = 'h';}
      if(vertical == 1) {priority = 'v';}
      if(horizontal == 1 && vertical == 1) {priority == 'h';}
    }
  
    if(priority == 'h') {speedFL_V = speedFR_V = speedBL_V = speedBR_V = 0;}
    if(priority == 'v') {speedFL_H = speedFR_H = speedBL_H = speedBR_H = 0;}
  
  
  //----------------------------------------------------------------------ESC Write
  
    escFL_H.writeMicroseconds(percent_to_servo(speedFL_H * SPEED_LIMIT, NEUTRAL));
    escFR_H.writeMicroseconds(percent_to_servo(speedFR_H * SPEED_LIMIT, NEUTRAL));
    escBL_H.writeMicroseconds(percent_to_servo(speedBL_H * SPEED_LIMIT, NEUTRAL));
    escBR_H.writeMicroseconds(percent_to_servo(speedBR_H * SPEED_LIMIT, NEUTRAL));
    escFL_V.writeMicroseconds(percent_to_servo(speedFL_V * SPEED_LIMIT, NEUTRAL));
    escFR_V.writeMicroseconds(percent_to_servo(speedFR_V * SPEED_LIMIT, NEUTRAL));
    escBL_V.writeMicroseconds(percent_to_servo(speedBL_V * SPEED_LIMIT, NEUTRAL));
    escBR_V.writeMicroseconds(percent_to_servo(speedBR_V * SPEED_LIMIT, NEUTRAL));
   
  
  //----------------------------------------------------------------------Debug Print
    
    /*
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
    */
  
    /*
    SerialUSB.println(Lx);  SerialUSB.println(Ly);
    SerialUSB.println(Rx);  SerialUSB.println(Ry);
    SerialUSB.println(LT);  SerialUSB.println(RT);
    SerialUSB.println(LB);  SerialUSB.println(RB);
    SerialUSB.println();
    */
    

  } //End main loop
