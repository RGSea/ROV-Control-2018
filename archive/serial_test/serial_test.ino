
//Macros
#define BAUDRATE 115200

//Functions
void serial_recv(uint16_t *varA, uint16_t *varB, uint16_t *varC, uint16_t *varD, uint16_t *varE, uint16_t *varF, uint16_t *varG, uint16_t *varH) {
  char serial_in[25] = "000000000000000000000000";
  Serial.readBytes(serial_in, 24);
  //Serial.print("In:  ");
  //Serial.println(serial_in);
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
  Serial.begin(BAUDRATE);

}

void loop() {
  uint16_t varA, varB, varC, varD, varE, varF, varG, varH;
  serial_recv(&varA, &varB, &varC, &varD, &varE, &varF, &varG, &varH);
  Serial.print("A = ");
  Serial.print(varA);
  Serial.print("  B = ");
  Serial.print(varB);
  Serial.print("  C = ");
  Serial.print(varC);
  Serial.print("  D = ");
  Serial.print(varD);
  Serial.print("  E = ");
  Serial.print(varE);
  Serial.print("  F = ");
  Serial.print(varF);
  Serial.print("  G = ");
  Serial.print(varG);
  Serial.print("  H = ");
  Serial.println(varH);
  

  

}
