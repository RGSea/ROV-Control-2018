//ROV Control 2018
//Surface Arduino Code
//Tim Brewis


//TFT Screen Setup
#include <Adafruit_GFX.h>                                              //Core graphics library
#include <Adafruit_TFTLCD.h>                                           //Hardware-specific library
#define LCD_CS    A3                                                   //Chip Select goes to Analog 3
#define LCD_CD    A2                                                   //Command/Data goes to Analog 2
#define LCD_WR    A1                                                   //LCD Write goes to Analog 1
#define LCD_RD    A0                                                   //LCD Read goes to Analog 0
#define LCD_RESET A4                                                   //Can alternately just connect to Arduino's reset pin
#define LCD_BLACK 0xFFFF                                               //Hex colour macros
#define LCD_WHITE 0x0000
#define LCD_RED   0x07FF
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


//USB Library Setup
#include <XBOXUSB.h>
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif
USB Usb;
XBOXUSB Xbox(&Usb);


//Rotary Encoder Setup
#include <Encoder.h>		                                                //Interrupt driven encoder library
Encoder myEnc(18, 19);


//EEPROM
#include <EEPROM.h>

//-------------------------------------------------------------Macros
#define DEADZONE 6500

//-------------------------------------------------------------Global Variables
  //Screen
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

  //Controller (arrays used for more efficient printing)
  const char *controller_str[] = {"Lx", "Ly", "Rx", "Ry", "LT", "RT", "LB", "RB"};
  const int controller_enum[] = {LeftHatX, LeftHatY, RightHatX, RightHatY, L2, R2, L1, R1};
  int16_t controller_val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int16_t controller_pval[8] = {-199, -199, -199, -199, -199, -199, -199, -199};
  uint8_t controller_serial[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  char serial_buffer[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  //Gain
  const char *gain_str[] = {"F/B  ", "L/R  ", "Vert ", "Yaw  ", "Roll ", "Pitch"};
  float gain_val[6] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00};

  //Rotary Encoder
  long encoder_pos = 0;
  long p_encoder_pos = -999;

  //Menus
  const char *menu_str[] = {"1: Debug", "2: Controller", "3: Gain Settings"};





//-------------------------------------------------------------Setup
void setup() {

  Serial.begin(115200);
  Serial.setTimeout(500);

  //Controller Setup
  #if !defined(__MIPSEL__)
  while (!Serial); //Wait for serial port to connect
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
  print_header();

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

  //EEPROM Read
  for(uint8_t i = 0; i < 6; i++) {
    //gain_val[i] = float(EEPROM.read(i)) / 100;  //Disable until testing time
    ;
  }

  //Serial Comms
  Serial2.begin(115200);

}


//-------------------------------------------------------------Main Loop
void loop(void) {

//-------------------------------------------------------------Local Variables
  //there are none?



//-------------------------------------------------------------Controller

  Usb.Task(); //Poll controller
  if(Xbox.Xbox360Connected) {

    //Fetch values
    for(uint8_t i = 0; i < 4; i++) {
      controller_val[i] = Xbox.getAnalogHat(controller_enum[i]);
      controller_val[i + 4] = Xbox.getButtonPress(controller_enum[i + 4]);
      deadZoneCheck(&controller_val[i]);
    }

    //Axis mapping
    for(uint8_t i = 0; i < 4; i++) {
      mapAxis(&controller_val[i]);
    }
    controller_val[4] = map(controller_val[4], 0, 255, 0, 100);
    controller_val[5] = map(controller_val[5], 0, 255, 0, 100);

    //I2C
    for(uint8_t i = 0; i < 8; i++) {
      controller_serial[i] = controller_val[i] + 200;  //No -ve num or < 10 down I2C
    }

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





//-----------------------------------------------------------Controller Readings
  tft.setCursor(15, 65);
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


//---------------------------------------------------------------Controller pcontroller_serial
  for(uint8_t i = 0; i < 8; i++) {
    if(main_pos == 2){
      controller_pval[i] = controller_val[i];
    }
    else {
      controller_pval[i] = -199;
    }
  }


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



//---------------------------------------------------------------Serial Comms
  for(uint8_t i = 0; i < 8; i++) {
  uint8_t buffer_pos = i * 3;
  if(controller_serial[i] < 10) {
    serial_buffer[buffer_pos] = serial_buffer[buffer_pos + 1] = '0';
    serial_buffer[buffer_pos + 2] = (controller_serial[i]) + '0';
  }
  if(controller_serial[i] < 100 && controller_serial[i] >= 10) {
    serial_buffer[buffer_pos] = '0';
    serial_buffer[buffer_pos + 1] = floor(controller_serial[i] / 10) + '0';
    serial_buffer[buffer_pos + 2] = (controller_serial[i] - (10 * floor(controller_serial[i] / 10))) + '0';
  }
  if(controller_serial[i] < 1000 && controller_serial[i] >= 10) {
    serial_buffer[buffer_pos] = floor(controller_serial[i] / 100) + '0';
    serial_buffer[buffer_pos + 1] = floor((controller_serial[i] - (100 * floor(controller_serial[i]/ 100))) / 10) + '0';
    serial_buffer[buffer_pos + 2] = (controller_serial[i] - (100 * floor(controller_serial[i] / 100)) - (10 * floor((controller_serial[i] - (100 * floor(controller_serial[i]/ 100))) / 10))) + '0';
  }
  else {
    serial_buffer[buffer_pos] = serial_buffer[buffer_pos + 1] = serial_buffer[buffer_pos + 2] = '0';
  }
  
}


  delay(1);

  Serial.print("Outgoing Buffer: ");
  for(int i = 0; i < 24; i++) {
    Serial.print(serial_buffer[i]);
  }
  Serial.println();
  Serial2.write(serial_buffer, 24);







//END MAIN LOOP
}




//-----------------------------------------------------------Functions

void deadZoneCheck(int16_t *axis) {
  if(*axis < DEADZONE && *axis > -DEADZONE) {
    *axis = 0;
  }
}

void mapAxis(int16_t *axis) {
  if(*axis > 0) {
    *axis = map(*axis, DEADZONE, 32767, 0, 100);
  }
  if(*axis < 0) {
    *axis = map(*axis, -32767, -DEADZONE, -100, 0);
  }
}

void print_header() {
  tft.setCursor(10, 8);
  tft.setTextColor(LCD_WHITE);
  tft.println("ROV Control");
  tft.drawFastHLine(0, 28, tft.width(), LCD_WHITE);
  tft.drawFastVLine(tft.width()/2, 0, tft.height(), LCD_WHITE);
  tft.drawRect(384, 4, 92,20, LCD_WHITE);
}



//Only need to print over main menu when selection changes
//Menu cleanup on change looks bad because of order
//Apply gain to controller readings
