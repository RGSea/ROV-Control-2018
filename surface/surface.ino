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


//Rotary Encoder
#include <Encoder.h>                                                    //Interrupt driven encoder library
Encoder myEnc(18, 19);  //CHANGE THESE PINS AS USED BY TX RX FOR SERIAL 1


//EEPROM
#include <EEPROM.h>


//Numpad
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




//Macros
#define BAUDRATE 115200
#define DEADZONE 6500
#define AXISMAX 32767
#define LCD_BLACK 0xFFFF
#define LCD_WHITE 0x0000
#define LCD_RED   0x07FF
#define SPEED_LIMIT 80




//Globals

  //Controller
    const int controller_enum[] = {LeftHatX, LeftHatY, RightHatX, RightHatY, L2, R2, L1, R1};
    int16_t controller_val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int16_t controller_pval[8] = {-199, -199, -199, -199, -199, -199, -199, -199};
    float gain_val[6] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00};

  //Screen / Menu
    uint8_t screen_enable = 0;
    uint8_t main_pos = 1;
    uint8_t sub_pos = 0;
    uint8_t last_menu = 1;
    uint8_t main_menu = 1;
    uint8_t sub_menu = 0;
    uint8_t last_main_pos = 2;
    uint8_t last_seconds = 0;
    uint8_t last_minutes = 0;
    uint8_t last_time =  0;
    uint8_t loop_time = 0;
    uint16_t width;
    uint16_t height;

  //Menu Strings
    const char *gain_str[] = {"F/B  ", "L/R  ", "Vert ", "Yaw  ", "Roll ", "Pitch"};
    const char *menu_str[] = {"Flight Calculations", "Gain Settings", "Controller Readings"};
    const char *controller_str[] = {"Lx", "Ly", "Rx", "Ry", "LT", "RT", "LB", "RB"};

  //Time
    uint16_t tick;
    uint16_t _micros;
    uint16_t pmicros;

  //Encoder
    long encoder_pos = 0;
    long p_encoder_pos = -999;
    
  //Flight Calculations
    
    uint8_t del = 0;
    uint8_t ent = 0;
    float key_in = -1;
    uint8_t data_counter_1 = 0;
    uint8_t data_counter_2 = 0;
    float heading = 0;
    float ascent_airspeed = 0;
    float ascent_rate = 0;
    float engine_fail_time = 0;
    float descent_airspeed = 0;
    float descent_rate = 0;
    float wind_speed = 0;
    float wind_heading = 0;
    float last_val = 27;
    float ascent_vector = 0;
    float descent_vector = 0;
    float altitude = 0;
    float descent_time = 0;
    float wind_vector = 0;




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

String serial_make_str_alt(uint16_t val) {
  String str;
  str = val;
  return str;
}

void serial_send(uint16_t val1, uint16_t val2, uint16_t val3, uint16_t val4, uint16_t val5, uint16_t val6, uint16_t val7, uint16_t val8) {
  String buffer;
  char ser_out[25] = "000000000000000000000000";
  String error_str;
  error_str += 'i';
  String speed_str;
  speed_str += '0';
  speed_str += SPEED_LIMIT;
  buffer = error_str + serial_make_str(val1) + serial_make_str(val2) + serial_make_str(val3) + serial_make_str(val4) + serial_make_str(val5) + serial_make_str(val6) + serial_make_str_alt(val7) + serial_make_str_alt(val8) + speed_str;
  buffer.toCharArray(ser_out, 25);
  Serial1.write(ser_out, 24);
  //Serial.println(ser_out);
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



}//End setup




//---------------------------------------------------------------Loop
void loop() {


  if(screen_enable == 1) {
    char key = Numpad.getKey();

    if(key != 'D' && key != 'E') {
      key_in = key - '0';
      del = 0;
      ent = 0;
    }
    else{
      if(key == 'D') {del = 1; key_in = -1;}
      if(key == 'E') {ent = 1; key_in = -1;} 
      else {key_in = -1;}
    }
  }

 
	
	
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
      if(Xbox.getButtonClick(Y)) {
        if(screen_enable == 1) {
          screen_enable = 0;
        }
        else {
          screen_enable = 1;
        }
      }

  }
  else {
    for(uint8_t i = 0; i < 8; i++) {controller_val[i] = 0;}
  }


//--------------------------------------------------------------------------Menu
if(screen_enable == 1) {
  if(main_menu == 1) {
    
    if(last_menu != 0) {
      tft.fillScreen(LCD_BLACK);
      tft.drawFastHLine(0, 30, width, LCD_WHITE);
      tft.setTextColor(LCD_WHITE);
      tft.setCursor(10, 8);
      tft.println("Main Menu");
      last_menu = 0;
    }

    if(main_pos != last_main_pos) {
  
      for(uint8_t i = 1; i < 4; i++) {
        if(i == main_pos) {
        tft.setTextColor(LCD_RED);
        }
        else {
          tft.setTextColor(LCD_WHITE);  
        }
        tft.setCursor(15, 15 + (i * 25));
        tft.print(i);
        tft.print(": ");
        tft.println(menu_str[i - 1]);
      }
      
    }
    
  }

  if(sub_menu == 1) {

    if(last_menu != 1) {
      tft.fillScreen(LCD_BLACK);
      tft.setTextColor(LCD_WHITE);  
      tft.drawFastHLine(0, 30, width, LCD_WHITE);
      tft.setCursor(10, 8);
      tft.println(menu_str[main_pos - 1]);
      last_menu = 1;
      data_counter_1 = 0;
      data_counter_2 = 0;
      last_val = -1;
    }



//-----------------------------------------------------------Flight Calculations
    if(main_pos == 1) { 

      if(Xbox.getButtonClick(X)) {
        data_counter_1 -= 1;
        data_counter_2 = 0;
        if(data_counter_1 == 255) {
          data_counter_1 = 0;
        }
      }
      
      //-----------------------------------------------Heading
      if(data_counter_1 == 0) {
        tft.setCursor(30, 65);
        tft.print("Heading:");            //Input as 3 figure bearing

        tft.fillCircle(15, 99, 4, LCD_BLACK);
        tft.fillCircle(15, 74, 4, LCD_RED);

        if(del == 1) {
          heading = 0; 
          data_counter_2 = 0;
        }
  
        if(key_in >= 0) {
          switch(data_counter_2) {
            case 0:
              heading = (key_in * 100);
              data_counter_2 += 1;
              break;
            case 1:
              heading += (key_in * 10);
              data_counter_2 += 1;
              break;
            case 2:
              heading += key_in;
              data_counter_2 += 1;
              break;
            case 3:
              heading += (key_in / 10);
              data_counter_2 += 1;
              break;
            case 4:
              heading += (key_in / 100);
              data_counter_2 = 0;
              break;
          }
        }

       if(heading != last_val) {
          tft.setCursor(300, 65);
          tft.setTextColor(LCD_BLACK);
          tft.print(last_val);
          tft.print(" deg");
          tft.setCursor(300, 65);
          tft.setTextColor(LCD_WHITE);
          tft.print(heading);
          tft.print(" deg");
          last_val = heading;
        }
  
        if(ent == 1) {
          if(heading < 360 && heading >= 0) {
            data_counter_1 = 1;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
          }
          else {
            heading = 0;
            data_counter_2 = 0;
          }
        }


        
      }//End of heading


      //--------------------------------------------Airspeed on Ascent
      if(data_counter_1 == 1) {
        
        tft.setCursor(30, 90);
        tft.print("Ascent airspeed:"); 

        tft.fillCircle(15, 74, 4, LCD_BLACK);
        tft.fillCircle(15, 124, 4, LCD_BLACK);
        tft.fillCircle(15, 99, 4, LCD_RED);

        if(del == 1) {
          ascent_airspeed = 0; 
          data_counter_2 = 0;
        }

        if(key_in >= 0) {
          switch(data_counter_2) {
            case 0:
              ascent_airspeed = (key_in * 100);
              data_counter_2 += 1;
              break;
            case 1:
              ascent_airspeed += (key_in * 10);
              data_counter_2 += 1;
              break;
            case 2:
              ascent_airspeed += key_in;
              data_counter_2 += 1;
              break;
            case 3:
              ascent_airspeed += (key_in / 10);
              data_counter_2 += 1;
              break;
            case 4:
              ascent_airspeed += (key_in / 100);
              data_counter_2 = 0;
              break;
          }
        }

       if(ascent_airspeed != last_val) {
          tft.setCursor(300, 90);
          tft.setTextColor(LCD_BLACK);
          tft.print(last_val);
          tft.print(" m/s");
          tft.setCursor(300, 90);
          tft.setTextColor(LCD_WHITE);
          tft.print(ascent_airspeed);
          tft.print(" m/s");
          last_val = ascent_airspeed;
        }

        if(ent == 1) {
          if(ascent_airspeed < 1000 && ascent_airspeed >= 0) {
            data_counter_1 = 2;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
          }
          else {
            ascent_airspeed = 0;
            data_counter_2 = 0;
          }
        }
    
      } //End of ascent airspeed


      //------------------------------------------------------------Ascent Rate
      if(data_counter_1 == 2) {
        
        tft.setCursor(30, 115);
        tft.print("Ascent rate:"); 

        tft.fillCircle(15, 99, 4, LCD_BLACK);
        tft.fillCircle(15, 149, 4, LCD_BLACK);
        tft.fillCircle(15, 124, 4, LCD_RED);

        if(del == 1) {
          ascent_rate = 0; 
          data_counter_2 = 0;
        }

        if(key_in >= 0) {
          switch(data_counter_2) {
            case 0:
              ascent_rate = (key_in * 100);
              data_counter_2 += 1;
              break;
            case 1:
              ascent_rate += (key_in * 10);
              data_counter_2 += 1;
              break;
            case 2:
              ascent_rate += key_in;
              data_counter_2 += 1;
              break;
            case 3:
              ascent_rate += (key_in / 10);
              data_counter_2 += 1;
              break;
            case 4:
              ascent_rate += (key_in / 100);
              data_counter_2 = 0;
              break;
          }
        }

       if(ascent_rate != last_val) {
          tft.setCursor(300, 115);
          tft.setTextColor(LCD_BLACK);
          tft.print(last_val);
          tft.print(" m/s");
          tft.setCursor(300, 115);
          tft.setTextColor(LCD_WHITE);
          tft.print(ascent_rate);
          tft.print(" m/s");
          last_val = ascent_rate;
        }


        if(ent == 1) {
          if(ascent_rate < 1000 && ascent_rate >= 0) {
            data_counter_1 = 3;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
          }
          else {
            ascent_rate = 0;
            data_counter_2 = 0;
          }
        }

    
      } //End of ascent rate


      //------------------------------------------------------------Engine Failure Time
      if(data_counter_1 == 3) {
        
        tft.setCursor(30, 140);
        tft.print("Engine failure time:"); 

        tft.fillCircle(15, 124, 4, LCD_BLACK);
        tft.fillCircle(15, 174, 4, LCD_BLACK);
        tft.fillCircle(15, 149, 4, LCD_RED);

        if(del == 1) {
          engine_fail_time = 0; 
          data_counter_2 = 0;
        }

        if(key_in >= 0) {
          switch(data_counter_2) {
            case 0:
              engine_fail_time = (key_in * 100);
              data_counter_2 += 1;
              break;
            case 1:
              engine_fail_time += (key_in * 10);
              data_counter_2 += 1;
              break;
            case 2:
              engine_fail_time += key_in;
              data_counter_2 += 1;
              break;
            case 3:
              engine_fail_time += (key_in / 10);
              data_counter_2 += 1;
              break;
            case 4:
              engine_fail_time += (key_in / 100);
              data_counter_2 = 0;
              break;
          }
        }

        if(engine_fail_time != last_val) {
          tft.setCursor(300, 140);
          tft.setTextColor(LCD_BLACK);
          tft.print(last_val);
          tft.print(" s");
          tft.setCursor(300, 140);
          tft.setTextColor(LCD_WHITE);
          tft.print(engine_fail_time);
          tft.print(" s");
          last_val = engine_fail_time;
        }

        if(ent == 1) {
          if(engine_fail_time < 1000 && engine_fail_time >= 0) {
            data_counter_1 = 4;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
          }
          else {
            engine_fail_time = 0;
            data_counter_2 = 0;
          }
        }

      } //End of engine fail time



      //------------------------------------------------------------Descent Airspeed
      if(data_counter_1 == 4) {
        
        tft.setCursor(30, 165);
        tft.print("Descent airspeed:"); 

        tft.fillCircle(15, 149, 4, LCD_BLACK);
        tft.fillCircle(15, 199, 4, LCD_BLACK);
        tft.fillCircle(15, 174, 4, LCD_RED);

        if(del == 1) {
          descent_airspeed = 0; 
          data_counter_2 = 0;
        }

        if(key_in >= 0) {
          switch(data_counter_2) {
            case 0:
              descent_airspeed = (key_in * 100);
              data_counter_2 += 1;
              break;
            case 1:
              descent_airspeed += (key_in * 10);
              data_counter_2 += 1;
              break;
            case 2:
              descent_airspeed += key_in;
              data_counter_2 += 1;
              break;
            case 3:
              descent_airspeed += (key_in / 10);
              data_counter_2 += 1;
              break;
            case 4:
              descent_airspeed += (key_in / 100);
              data_counter_2 = 0;
              break;
          }
        }

        if(descent_airspeed != last_val) {
          tft.setCursor(300, 165);
          tft.setTextColor(LCD_BLACK);
          tft.print(last_val);
          tft.print(" m/s");
          tft.setCursor(300, 165);
          tft.setTextColor(LCD_WHITE);
          tft.print(descent_airspeed);
          tft.print(" m/s");
          last_val = descent_airspeed;
        }

        if(ent == 1) {
          if(descent_airspeed < 1000 && descent_airspeed >= 0) {
            data_counter_1 = 5;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
          }
          else {
            descent_airspeed = 0;
            data_counter_2 = 0;
          }
        }

      } //End of descent airspeed



      //------------------------------------------------------------Descent Rate
      if(data_counter_1 == 5) {
        
        tft.setCursor(30, 190);
        tft.print("Descent rate:"); 

        tft.fillCircle(15, 174, 4, LCD_BLACK);
        tft.fillCircle(15, 224, 4, LCD_BLACK);
        tft.fillCircle(15, 199, 4, LCD_RED);

        if(del == 1) {
          descent_rate = 0; 
          data_counter_2 = 0;
        }

        if(key_in >= 0) {
          switch(data_counter_2) {
            case 0:
              descent_rate = (key_in * 100);
              data_counter_2 += 1;
              break;
            case 1:
              descent_rate += (key_in * 10);
              data_counter_2 += 1;
              break;
            case 2:
              descent_rate += key_in;
              data_counter_2 += 1;
              break;
            case 3:
              descent_rate += (key_in / 10);
              data_counter_2 += 1;
              break;
            case 4:
              descent_rate += (key_in / 100);
              data_counter_2 = 0;
              break;
          }
        }

        if(descent_rate != last_val) {
          tft.setCursor(300, 190);
          tft.setTextColor(LCD_BLACK);
          tft.print(last_val);
          tft.print(" m/s");
          tft.setCursor(300, 190);
          tft.setTextColor(LCD_WHITE);
          tft.print(descent_rate);
          tft.print(" m/s");
          last_val = descent_rate;
        }

        if(ent == 1) {
          if(descent_rate < 1000 && descent_rate >= 0) {
            data_counter_1 = 6;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
          }
          else {
            descent_rate = 0;
            data_counter_2 = 0;
          }
        }

      } //End of descent rate


      //------------------------------------------------------------Wind Heading
      if(data_counter_1 == 6) {
        
        tft.setCursor(30, 215);
        tft.print("Wind heading:"); 

        tft.fillCircle(15, 199, 4, LCD_BLACK);
        tft.fillCircle(15, 249, 4, LCD_BLACK);
        tft.fillCircle(15, 224, 4, LCD_RED);

        if(del == 1) {
          wind_heading = 0; 
          data_counter_2 = 0;
        }

        if(key_in >= 0) {
          switch(data_counter_2) {
            case 0:
              wind_heading = (key_in * 100);
              data_counter_2 += 1;
              break;
            case 1:
              wind_heading += (key_in * 10);
              data_counter_2 += 1;
              break;
            case 2:
              wind_heading += key_in;
              data_counter_2 += 1;
              break;
            case 3:
              wind_heading += (key_in / 10);
              data_counter_2 += 1;
              break;
            case 4:
              wind_heading += (key_in / 100);
              data_counter_2 = 0;
              break;
          }
        }

        if(wind_heading != last_val) {
          tft.setCursor(300, 215);
          tft.setTextColor(LCD_BLACK);
          tft.print(last_val);
          tft.print(" deg");
          tft.setCursor(300, 215);
          tft.setTextColor(LCD_WHITE);
          tft.print(wind_heading);
          tft.print(" deg");
          last_val = wind_heading;
        }

        if(ent == 1) {
          if(wind_heading < 360 && descent_rate >= 0) {
            data_counter_1 = 7;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
          }
          else {
            wind_heading = 0;
            data_counter_2 = 0;
          }
        }

      } //End of wind heading


      //------------------------------------------------------------Wind Speed
      if(data_counter_1 == 7) {
        
        tft.setCursor(30, 240);
        tft.print("Wind speed:"); 

        tft.fillCircle(15, 224, 4, LCD_BLACK);
        tft.fillCircle(15, 289, 4, LCD_BLACK);
        tft.fillCircle(15, 249, 4, LCD_RED);

        if(del == 1) {
          wind_speed = 0; 
          data_counter_2 = 0;
        }

        if(key_in >= 0) {
          switch(data_counter_2) {
            case 0:
              wind_speed = (key_in * 100);
              data_counter_2 += 1;
              break;
            case 1:
              wind_speed += (key_in * 10);
              data_counter_2 += 1;
              break;
            case 2:
              wind_speed += key_in;
              data_counter_2 += 1;
              break;
            case 3:
              wind_speed += (key_in / 10);
              data_counter_2 += 1;
              break;
            case 4:
              wind_speed += (key_in / 100);
              data_counter_2 = 0;
              break;
          }
        }

        if(wind_speed != last_val) {
          tft.setCursor(300, 240);
          tft.setTextColor(LCD_BLACK);
          tft.print(last_val);
          tft.print(" m/s");
          tft.setCursor(300, 240);
          tft.setTextColor(LCD_WHITE);
          tft.print(wind_speed);
          tft.print(" m/s");
          last_val = wind_speed;
        }

        if(ent == 1) {
          if(wind_speed < 1000 && wind_speed >= 0) {
            data_counter_1 = 8;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
          }
          else {
            wind_speed = 0;
            data_counter_2 = 0;
          }
        }

      } //End of wind speed


      //----------------------------------------------------Compute
      if(data_counter_1 == 8) {
        tft.setCursor(195, 280);
        tft.print("COMPUTE");  

        tft.fillCircle(15, 249, 4, LCD_BLACK);
        tft.fillCircle(15, 289, 4, LCD_RED);

        if(ent == 1) {
            data_counter_1 = 9;
            data_counter_2 = 0;
            last_val = -1;
            ent = 0;
        }
            
      }

      if(data_counter_1 == 9) {
        
        if(data_counter_2 == 0) {
          tft.fillScreen(LCD_BLACK);
          data_counter_2 = 1;
          tft.drawFastHLine(0, 30, width, LCD_WHITE);
          tft.setCursor(10, 8);
          tft.println("Results of Flight Calculations");

          //Calculate
          ascent_vector = ascent_airspeed * engine_fail_time;
          altitude = ascent_rate * engine_fail_time;
          descent_time = altitude / descent_rate;
          descent_vector = descent_time * descent_airspeed;
          wind_vector = (engine_fail_time + descent_time) * wind_speed;

          //Display
          tft.setCursor(10, 40);
          tft.print("Ascent   = ");
          tft.print(ascent_vector);
          tft.print("m at ");
          tft.print(heading);
          tft.print(" deg");

          tft.setCursor(10, 65);
          tft.print("Descent  = ");
          tft.print(descent_vector);
          tft.print("m at ");
          tft.print(heading);
          tft.print(" deg");

          tft.setCursor(10, 90);
          tft.print("Wind     = ");
          tft.print(wind_vector);
          tft.print("m at ");
          tft.print(wind_heading);
          tft.print(" deg");

          tft.setCursor(10, 115);
          tft.print("Altitude = ");
          tft.print(wind_vector);
          tft.print("m");
         
        }
      
      } //End of compute
      
      
    } //End of flight calculations


    //-----------------------------------------------------------------------------Gain Settings
    if(main_pos == 2) {
      
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

      //Gain
  
      for(uint8_t i = 0; i < 7; i++) {
  
        if(sub_menu == 1 && i == sub_pos && i != 6) {
          tft.setTextColor(LCD_BLACK);
          tft.setCursor(140, 65 + (i * 25));
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
          tft.setCursor(30, 65 + (i * 25));
          tft.print(gain_str[i]);
          tft.print(" = ");
          tft.setCursor(140, 65 + (i * 25));
          tft.print(gain_val[i]);
        }
        else {
          tft.setCursor(30, 65 + 25 + (i * 25));
          if(sub_pos == 6) {
            tft.setTextColor(LCD_RED);
            tft.print("SAVE TO EEPROM");
            if(Xbox.getButtonPress(A)) {
              for(int i = 0; i < 6; i++) {
                EEPROM.write(i, int(gain_val[i] * 100));
              }
              for(uint8_t i = 0; i < 2; i++) {
                tft.setTextColor(LCD_BLACK);
                tft.setCursor(30, 65 + 25 + (6 * 25));
                tft.print("SAVE TO EEPROM");
                delay(500);
                tft.setTextColor(LCD_RED);
                tft.setCursor(30, 65 + 25 + (6 * 25));
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
  
  
        
    } //End of gain settings



    //-----------------------------------------------------------------------------Controller Readings
    if(main_pos == 3) {
      tft.setTextColor(LCD_WHITE);
  
      for(int i = 0; i < 8; i++) {
        tft.setCursor(30, 65 + (i * 25));
        tft.print(controller_str[i]);
        tft.print(" = ");
  
        if(controller_val[i] != controller_pval[i]) {
          tft.setTextColor(LCD_BLACK);
          tft.setCursor(90, 65 + (i * 25));
          tft.print(controller_pval[i]);
          tft.setTextColor(LCD_WHITE);
          tft.setCursor(90, 65 + (i * 25));
          tft.print(controller_val[i]);
        }
      } 

      for(uint8_t i = 0; i < 8; i++) {
        controller_pval[i] = controller_val[i];  
      }
    }//End of controller readings
    
  } //End of sub menus

} //End of screen enable
else {
  uint8_t x = 0;
  if(x == 0) {
    tft.fillScreen(LCD_BLACK);
    x = 1;
    last_menu = 2;
  }
}


//---------------------------------------------------------------Serial To Onboard

  	uint16_t val1 = ceil(controller_val[0] * gain_val[0]) + 100;
  	uint16_t val2 = ceil(controller_val[1] * gain_val[1]) + 100;
  	uint16_t val3 = ceil(controller_val[2] * gain_val[2]) + 100;
  	uint16_t val4 = ceil(controller_val[3] * gain_val[3]) + 100;
  	uint16_t val5 = ceil(controller_val[4] * gain_val[4]) + 100;
  	uint16_t val6 = ceil(controller_val[5] * gain_val[5]) + 100;
  	uint16_t val7 = ceil(controller_val[6]);
  	uint16_t val8 = ceil(controller_val[7]);

  	serial_send(val1, val2, val3, val4, val5, val6, val7, val8);	
    delay(10);	


}
