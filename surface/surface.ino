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

//TFT Screen Library
#include <Adafruit_GFX.h>                                              //Core graphics library
#include <Adafruit_TFTLCD.h>                                           //Hardware-specific library
#define LCD_CS    A3                                                   //Chip Select goes to Analog 3
#define LCD_CD    A2                                                   //Command/Data goes to Analog 2
#define LCD_WR    A1                                                   //LCD Write goes to Analog 1
#define LCD_RD    A0                                                   //LCD Read goes to Analog 0
#define LCD_RESET A4                                                   //Can alternately just connect to Arduino's reset pin
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

//Rotary Encoder Setup
#include <Encoder.h>                                                    //Interrupt driven encoder library
Encoder myEnc(18, 19);

//EEPROM
#include <EEPROM.h>




//Macros
#define BAUDRATE 115200 //DO NOT CHANGE
#define DEADZONE 6500
#define AXISMAX 32767
#define LCD_BLACK 0xFFFF
#define LCD_WHITE 0x0000
#define LCD_RED   0x07FF


//Globals
const int controller_enum[] = {LeftHatX, LeftHatY, RightHatX, RightHatY, L2, R2, L1, R1};
int16_t controller_val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int16_t controller_pval[8] = {-199, -199, -199, -199, -199, -199, -199, -199};
float gain_val[6] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00};

uint8_t main_pos = 1;
uint8_t sub_pos = 0;
uint8_t main_menu = 1;
uint8_t sub_menu = 0;
uint8_t last_main_pos = 1;
uint8_t last_seconds = 0;
uint8_t last_minutes = 0;
uint8_t last_time =  0;
uint8_t loop_time = 0;
uint16_t width;
uint16_t height;
const char *gain_str[] = {"F/B  ", "L/R  ", "Vert ", "Yaw  ", "Roll ", "Pitch"};
const char *menu_str[] = {"1: Debug", "2: Controller", "3: Gain Settings"};
const char *controller_str[] = {"Lx", "Ly", "Rx", "Ry", "LT", "RT", "LB", "RB"};

uint16_t tick;
uint16_t _micros;
uint16_t pmicros;

long encoder_pos = 0;
long p_encoder_pos = -999;




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
  Serial1.write(ser_out, 24);
  //Serial2.flush();
}




//---------------------------------------------------------------Setup
void setup() {

	//Serial Setup
	Serial.begin(BAUDRATE);
	Serial1.begin(BAUDRATE);
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


  //TFT Screen Setup
  tft.reset();

  uint16_t identifier = tft.readID();
  if(identifier==0x0101)
      identifier=0x9341;
   identifier=0x9481;

  tft.begin(identifier);
  tft.setRotation(1);
  tft.fillScreen(LCD_BLACK);
  tft.setTextSize(2);
  width = tft.width();
  height = tft.height();

  //Initial/permanent TFT screen prints 
  tft.setCursor(10, 8);
  tft.setTextColor(LCD_WHITE);
  tft.println("ROV Control");
  tft.drawFastHLine(0, 28, tft.width(), LCD_WHITE);
  tft.drawFastVLine(tft.width()/2, 0, tft.height(), LCD_WHITE);
  tft.drawRect(384, 4, 92,20, LCD_WHITE);

  for(uint8_t i = 1; i < 4; i++) {
    if(i == main_pos) {
      tft.setTextColor(LCD_RED);
    }
    else {
      tft.setTextColor(LCD_WHITE);
    }
    tft.setCursor(15, 15 + (i * 25));
    tft.println(menu_str[i - 1]);
  }




}//End setup




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


  	   //Menu Navigation 
      if(main_menu) {
        if(Xbox.getButtonClick(UP)) {
          last_main_pos = main_pos;
          main_pos -= 1;
        }
        if(Xbox.getButtonClick(DOWN)) {
          last_main_pos = main_pos;
          main_pos += 1;
        }
        if(main_pos > 3) {
          //last_main_pos = main_pos;
          main_pos = 1;
        }
        if(main_pos < 1) {
          //last_main_pos = main_pos;
          main_pos = 3;
        }
      }

      if(sub_menu) {
        if(Xbox.getButtonClick(UP)) {
          sub_pos -= 1;
        }
        if(Xbox.getButtonClick(DOWN)) {
          sub_pos += 1;
        }
        if(sub_pos > 6) {
          sub_pos = 0;
        }
        if(sub_pos < 0) {
          sub_pos = 6;
        }
      }

      if(Xbox.getButtonClick(A)) {
        main_menu = 0;
        sub_menu = 1;
      }
      if(Xbox.getButtonClick(B)) {
        main_menu = 1;
        sub_menu = 0;
      }

  }
  else {
    //Catch for Xbox360Connected being false
    for(uint8_t i = 0; i < 8; i++) {
      controller_val[i] = 0;
    }
  }


//-------------------------------------------------------------Menu Selection
if(main_pos != last_main_pos) {
  for(uint8_t i = 1; i < 4; i++) {
    if(i == main_pos) {
      tft.setTextColor(LCD_RED);
    }
    else {
      tft.setTextColor(LCD_WHITE);
    }
    tft.setCursor(15, 15 + (i * 25));
    tft.println(menu_str[i - 1]);
  }

}

//-------------------------------------------------------------Debug




if(main_pos != last_main_pos) {
//-----------------------------------------------------------Controller Readings
  if(main_pos == 2) {

    tft.setTextColor(LCD_WHITE);

    for(int i = 0; i < 8; i++) {
      tft.setCursor(280, 65 + (i * 25));
      tft.print(controller_str[i]);
      tft.print(" = ");

      if(controller_val[i] != controller_pval[i]) {
        tft.setTextColor(LCD_BLACK);
        tft.setCursor(340, 65 + (i * 25));
        tft.print(controller_pval[i]);
        tft.setTextColor(LCD_WHITE);
        tft.setCursor(340, 65 + (i * 25));
        tft.print(controller_val[i]);
      }
    }

  }
  

//-----------------------------------------------------------Gain Settings

  //Encoder readings
  int change;
  encoder_pos = myEnc.read();
  if(encoder_pos != p_encoder_pos) {
    change = encoder_pos - p_encoder_pos;
    p_encoder_pos = encoder_pos;
  }
  else {
    change = 0;
  }

  if(main_pos == 3) {
    uint8_t lspace = 43;

    for(uint8_t i = 0; i < 7; i++) {

      if(sub_menu == 1 && i == sub_pos && i != 6) {
        tft.setTextColor(LCD_BLACK);
        tft.setCursor(380, 65 + (i * 25));
        tft.print(gain_val[i]);
        gain_val[i] += float(change) / 200;
        if(gain_val[i] > 1.00) {gain_val[i] = 1.00;}
        if(gain_val[i] < 0.00) {gain_val[i] = 0.00;}
        tft.setTextColor(LCD_RED);
      }
      else {
        tft.setTextColor(LCD_WHITE);
      }

      if(i != 6) {
        tft.setCursor(280, 65 + (i * 25));
        tft.print(gain_str[i]);
        tft.print(" = ");
        tft.setCursor(380, 65 + (i * 25));
        tft.print(gain_val[i]);
      }
      else {
        tft.setCursor(280, 65 + 25 + (i * 25));
        if(sub_pos == 6) {
          tft.setTextColor(LCD_RED);
          tft.print("SAVE TO EEPROM");
          if(Xbox.getButtonPress(A)) {
            for(int i = 0; i < 6; i++) {
              EEPROM.write(i, int(gain_val[i] * 100));
            }
            for(uint8_t i = 0; i < 2; i++) {
              tft.setTextColor(LCD_BLACK);
              tft.setCursor(280, 65 + 25 + (6 * 25));
              tft.print("SAVE TO EEPROM");
              delay(500);
              tft.setTextColor(LCD_RED);
              tft.setCursor(280, 65 + 25 + (6 * 25));
              tft.print("SAVE TO EEPROM");
              delay(500);
            }
          }
        }
        else {
          tft.setTextColor(LCD_WHITE);
          tft.print("SAVE TO EEPROM");
        }
      }



    }

  }


//-------------------------------------------------------------Cleanup Menu
  tft.setTextColor(LCD_BLACK);
  
  //Controller
  if(last_main_pos == 2) {
    for(uint8_t i = 0; i < 8; i++) {
      tft.setCursor(280, 65 + (i * 25));
      tft.print(controller_str[i]);
      tft.print(" = ");
      tft.print(controller_pval[i]);
    }
    last_main_pos = 0;  //Null value so this code is skipped next loop
  }

  //Gain Settings
  if(last_main_pos == 3) {
    for(uint8_t i = 0; i < 6; i++) {
      tft.setCursor(280, 65 + (i * 25));
      tft.print(gain_str[i]);
      tft.print(" = ");
      tft.setCursor(380, 65 + (i * 25));
      tft.print(gain_val[i]);
    }
    tft.setCursor(280, 65 + (7 * 25));
    tft.print("SAVE TO EEPROM");
    last_main_pos = 0;
    sub_pos = 0;
  }


//---------------------------------------------------------------Controller
  for(uint8_t i = 0; i < 8; i++) {
    if(main_pos == 2){
      controller_pval[i] = controller_val[i];
    }
    else {
      controller_pval[i] = -199;
    }
  }
}//End change check

//---------------------------------------------------------------Runtime
  //Runtime calculation
  long time_ms = millis();
  int time = int(time_ms / 1000);
  uint8_t seconds;
  uint8_t minutes;

  if(time < 60) {
     seconds = time;
     minutes = 0;
  }
  if(time > 60) {
     minutes = floor(time / 60);
     seconds = time - minutes * 60;
  }

  //Status bar
  if(minutes < 15) {
    tft.drawFastVLine(385 + (time / 10), 5, 18, LCD_RED);
  }

  //Timer
  if(minutes >= 13) {
     tft.setTextColor(LCD_RED);
  }
  else {
     tft.setTextColor(LCD_WHITE);
  }

  //Print time every second
  if(seconds == last_seconds + 1 || (last_seconds == 59 && seconds == 0)) {
    tft.setTextColor(LCD_BLACK);
    tft.setCursor(480/2 + 10, 8);
    tft.print(last_minutes);
    tft.print("min ");
    tft.print(last_seconds);
    tft.print("s");
    tft.setTextColor(LCD_WHITE);
    tft.setCursor(width/2 + 10, 8);
    tft.print(minutes);
    tft.print("min ");
    tft.print(seconds);
    tft.print("s");
    last_minutes = minutes;
    last_seconds = seconds;
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
    delay(10);	
	

	/*_micros = micros();
	tick = _micros - pmicros;
	Serial.print("	");
	Serial.println(tick);
	
		//Serial.println(tick);*/



}
