//ROV Control 2018
//Surface Arduino Code
//Tim Brewis




//USB Library
#include <XBOXUSB.h>
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif
USB Usb;
XBOXUSB Xbox(&Usb);


//Macros
#define BAUDRATE 115200 //DO NOT CHANGE
#define DEADZONE 6500
#define AXISMAX 32767


//Globals
const int controller_enum[] = {LeftHatX, LeftHatY, RightHatX, RightHatY, L2, R2, L1, R1};
int16_t controller_val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
float gain_val[6] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00};
uint16_t tick;
uint16_t _micros;
uint16_t pmicros;




//---------------------------------------------------------------Functions
void deadzone_check(int16_t *axis) {
  if(*axis < DEADZONE && *axis > -DEADZONE) {*axis = 0;}
}

void map_axis(int16_t *axis) {
  if(*axis > 0) {*axis = map(*axis, DEADZONE, AXISMAX, 0, 100);}
  if(*axis < 0) {*axis = map(*axis, -AXISMAX, -DEADZONE, -100, 0);}
}

String serial_make_str(uint16_t val) {
  String str;
  String zero = "0";
  if(val < 10 and val >= 0) {str = zero + zero + val;}
  if(val < 100 and val >= 10) {str = zero + val;}
  if(val < 1000 and val >= 100) {str = val;}
  if(val < 0 or val >= 1000) {str = "000";}
  return str;
}

void serial_send(uint16_t val1, uint16_t val2, uint16_t val3, uint16_t val4, uint16_t val5, uint16_t val6, uint16_t val7, uint16_t val8) {
  String buffer;
  char ser_out[25] = "000000000000000000000000";
  buffer = serial_make_str(val1) + serial_make_str(val2) + serial_make_str(val3) + serial_make_str(val4) + serial_make_str(val5) + serial_make_str(val6) + serial_make_str(val7) + serial_make_str(val8);
  buffer.toCharArray(ser_out, 25);
  Serial2.write(ser_out, 24);
  //Serial2.flush();
  //Serial.print(ser_out);
}




//---------------------------------------------------------------Setup
void setup() {

	//Serial Setup
	Serial.begin(BAUDRATE);
	Serial2.begin(BAUDRATE);
	Serial.setTimeout(500);

 	//Controller Setup
 	#if !defined(__MIPSEL__)
  	while (!Serial); 													//Wait for serial port to connect
  	#endif
  	if (Usb.Init() == -1) {
    	Serial.print(F("\r\nXBOX USB did not start"));
    	while(1); //Stop
  	}
  	Serial.print(F("\r\nXBOX USB Library Started"));
}




//---------------------------------------------------------------Loop
void loop() {
	pmicros = micros();
	for(uint8_t i = 0; i < 8; i++) {controller_val[i] = 0;}
	
//---------------------------------------------------------------Controller
	Usb.Task();
	if(Xbox.Xbox360Connected) {

    	//Fetch values
	    for(uint8_t i = 0; i < 4; i++) {
	    	controller_val[i] = Xbox.getAnalogHat(controller_enum[i]);
	      	controller_val[i + 4] = Xbox.getButtonPress(controller_enum[i + 4]);
	      	deadzone_check(&controller_val[i]);
	    }

	    //Axis mapping
	    for(uint8_t i = 0; i < 4; i++) {map_axis(&controller_val[i]);}
	    if(controller_val[4] != 0) {controller_val[4] = map(controller_val[4], 0, 255, 0, 100);}
	    if(controller_val[5] != 0) {controller_val[5] = map(controller_val[5], 0, 255, 0, 100);}

	   /*for(uint8_t i = 0; i < 8; i++) {
	    	Serial.print(controller_val[i]);
	    	Serial.print(" ");
	    }*/
	    //Serial.print("	");

	}
	else {
		for(uint8_t i = 0; i < 8; i++) {controller_val[i] = 0;}
	}


//---------------------------------------------------------------Serial To Onboard

  	uint16_t val1 = ceil(controller_val[0] * gain_val[0]) + 100;
  	uint16_t val2 = ceil(controller_val[1] * gain_val[1]) + 100;
  	uint16_t val3 = ceil(controller_val[2] * gain_val[2]) + 100;
  	uint16_t val4 = ceil(controller_val[3] * gain_val[3]) + 100;
  	uint16_t val5 = ceil(controller_val[4] * gain_val[4]) + 100;
  	uint16_t val6 = ceil(controller_val[5] * gain_val[5]) + 100;
  	uint16_t val7 = ceil(controller_val[6]) + 100;
  	uint16_t val8 = ceil(controller_val[7]) + 100;
  	
  	Usb.Task();

  	serial_send(val1, val2, val3, val4, val5, val6, val7, val8);	
    delay(4);	
	

	/*_micros = micros();
	tick = _micros - pmicros;
	Serial.print("	");
	Serial.println(tick);
	
		//Serial.println(tick);*/



}

