//Send Functions
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
  Serial.print("Out: ");
  Serial.println(ser_out);
  Serial2.write(ser_out, 24);
}

//Recieve Functions
void serial_recv(uint16_t *varA, uint16_t *varB, uint16_t *varC, uint16_t *varD, uint16_t *varE, uint16_t *varF, uint16_t *varG, uint16_t *varH) {
  char serial_in[25] = "000000000000000000000000";
  Serial3.readBytes(serial_in, 24);
  Serial.print("In:  ");
  Serial.println(serial_in);
  *varA = ((serial_in[0] - '0') * 100) + ((serial_in[1] - '0') * 10) + (serial_in[2] - '0');
  *varB = ((serial_in[3] - '0') * 100) + ((serial_in[4] - '0') * 10) + (serial_in[5] - '0');
  *varC = ((serial_in[6] - '0') * 100) + ((serial_in[7] - '0') * 10) + (serial_in[8] - '0');
  *varD = ((serial_in[9] - '0') * 100) + ((serial_in[10] - '0') * 10) + (serial_in[11] - '0');
  *varE = ((serial_in[12] - '0') * 100) + ((serial_in[13] - '0') * 10) + (serial_in[14] - '0');
  *varF = ((serial_in[15] - '0') * 100) + ((serial_in[16] - '0') * 10) + (serial_in[17] - '0');
  *varG = ((serial_in[18] - '0') * 100) + ((serial_in[19] - '0') * 10) + (serial_in[20] - '0');
  *varH = ((serial_in[21] - '0') * 100) + ((serial_in[22] - '0') * 10) + (serial_in[23] - '0');
}




void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);
}




void loop() {
  uint16_t val1 = 0;
  uint16_t val2 = 4;
  uint16_t val3 = 47;
  uint16_t val4 = 342;
  uint16_t val5 = 650;
  uint16_t val6 = 999;
  uint16_t val7 = 255;
  uint16_t val8 = 97;
  uint16_t varA, varB, varC, varD, varE, varF, varG, varH;
  
  serial_send(val1, val2, val3, val4, val5, val6, val7, val8);
  serial_recv(&varA, &varB, &varC, &varD, &varE, &varF, &varG, &varH);
  Serial.print("varA = ");
  Serial.println(varA);
  Serial.print("varB = ");
  Serial.println(varB);
  Serial.print("varC = ");
  Serial.println(varC);
  Serial.print("varD = ");
  Serial.println(varD);
  Serial.print("varE = ");
  Serial.println(varE);
  Serial.print("varF = ");
  Serial.println(varF);
  Serial.print("varG = ");
  Serial.println(varG);
  Serial.print("varH = ");
  Serial.println(varH);
  
  delay(1000);
  

  
}


/*
  for(int i = 0; i < 8; i++) {
    int buffer_pos = i * 3;
    if(controller_I2C[i] < 10) {
      I2Cbuffer[buffer_pos] = I2Cbuffer[buffer_pos + 1] = '0';
      I2Cbuffer[buffer_pos + 2] = controller_I2C[i] + '0';
    }
    if(controller_I2C[i] < 100 && controller_I2C[i] >= 10) {
      I2Cbuffer[buffer_pos] = '0';
      I2Cbuffer[buffer_pos + 1] = floor(controller_I2C[i] / 10) + '0';
      I2Cbuffer[buffer_pos + 2] = (controller_I2C[i] - (10 * floor(controller_I2C[i] / 10))) + '0';
    }
    if(controller_I2C[i] < 1000 && controller_I2C[i] >= 10) {
      I2Cbuffer[buffer_pos] = floor(controller_I2C[i] / 100) + '0';
      I2Cbuffer[buffer_pos + 1] = floor((controller_I2C[i] - (100 * floor(controller_I2C[i]/ 100))) / 10) + '0';
      I2Cbuffer[buffer_pos + 2] = (controller_I2C[i] - (100 * floor(controller_I2C[i] / 100)) - (10 * floor((controller_I2C[i] - (100 * floor(controller_I2C[i]/ 100))) / 10))) + '0';
    }
    else {
      I2Cbuffer[buffer_pos] = I2Cbuffer[buffer_pos + 1] = I2Cbuffer[buffer_pos + 2] = '0';
    }
  }*/




