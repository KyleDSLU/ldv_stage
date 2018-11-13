#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define STEP_PIN_0 4
#define DIR_PIN_0 5
#define STEP_PIN_1 6
#define DIR_PIN_1 7


long steps[2];
long cur_pos[2];
long msg;

int inByte;
int outByte;

bool fresh;
bool send_ser; 

void setup()
{
  Serial.begin(57600);
  send_ser = false;
  pinMode(DIR_PIN_0, OUTPUT); 
  pinMode(STEP_PIN_0, OUTPUT);
  pinMode(DIR_PIN_1, OUTPUT); 
  pinMode(STEP_PIN_1, OUTPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13, LOW);
  //Serial.print("LDV Stage Control - Arduino Slave 06-13-2017");
  //Serial.print("\n");
  inByte = 0;
  outByte = 0;
  send_ser = false;
}


void initialize_stage(){
    steps[0] = -10000;
    steps[1] = -10000;
    //rotate(steps, 1);
    cur_pos[0] = 0;
    cur_pos[1] = 0;
}


void loop()
{
  if (Serial.available() > 0) {
    // get incoming byte:
    //If using the Arudino Serial Monitor, you need to subtract '0' to offset the ASCII code by 48 (I think)
    inByte = Serial.read();
  
    if (inByte == 1){
      //Serial.println("Initializing");
      initialize_stage();
      msg = inByte;
      send_ser = true;
    }
    
    else if (inByte == 2){
      //main control case
      delay(50);
      digitalWrite(13,HIGH);
      send_ser = true;
      steps[0] = readfourbytes();
      steps[1] = readfourbytes();
      rotate(steps, 1, 'x');
      rotate(steps, 1, 'y');
      msg = steps[0]+steps[1]+inByte;
      digitalWrite(13, LOW);
    }
    else 
    {
      serial_flush();
    }
  
  if (send_ser){
    send_ser = false;
    SendFourByteLong(msg);
    }
  }
}
 
void rotate(long steps[], float speed, int motor){ 
  //rotate a specific number of microsteps (8 microsteps per step) - (negitive for reverse movement)
  //speed is any number from .01 -> 1 with 1 being fastest - Slower is stronger
  int dir_0 = (steps[0] >= 0)? HIGH:LOW;
  int dir_1 = (steps[1] >= 0)? HIGH:LOW;
  long steps_sent[2];
  long max_steps;
  bool x_larger = false;
  bool y_larger = false;
  bool x_flag = false;
  bool y_flag = false;
  
  steps_sent[0] = abs(steps[0]);
  steps_sent[1] = abs(steps[1]);

  digitalWrite(DIR_PIN_0,dir_0); 
  digitalWrite(DIR_PIN_1,dir_1); 
  
  float usDelay = (1/speed) * 100;

  if (motor == 'x')
  {
    max_steps = steps_sent[0];
    x_flag = true;
  }
  else if (motor == 'y')
  {
    max_steps = steps_sent[1];
    y_flag = true;
  }
  
  if (x_flag == true)
  {
    for(long i=0; i < max_steps; i++){ 
      digitalWrite(STEP_PIN_0, HIGH);
      delayMicroseconds(usDelay); 
  
      digitalWrite(STEP_PIN_0, LOW);
      delayMicroseconds(usDelay);
    }
  }

  if (y_flag == true)
  {
    for(long i=0; i < max_steps; i++){ 
      digitalWrite(STEP_PIN_1, HIGH);
      delayMicroseconds(usDelay); 
  
      digitalWrite(STEP_PIN_1, LOW);
      delayMicroseconds(usDelay);
    }
  }
}
//void rotate(long steps[], float speed){ 
//  //rotate a specific number of microsteps (8 microsteps per step) - (negitive for reverse movement)
//  //speed is any number from .01 -> 1 with 1 being fastest - Slower is stronger
//  int dir_0 = (steps[0] >= 0)? HIGH:LOW;
//  int dir_1 = (steps[1] >= 0)? HIGH:LOW;
//  long steps_sent[2];
//  long max_steps;
//  bool x_larger = false;
//  bool y_larger = false;
//  bool x_flag = false;
//  bool y_flag = false;
//  
//  steps_sent[0] = abs(steps[0]);
//  steps_sent[1] = abs(steps[1]);
//
//  digitalWrite(DIR_PIN_0,dir_0); 
//  digitalWrite(DIR_PIN_1,dir_1); 
//  
//  float usDelay = (1/speed) * 150;
//
//  if (steps_sent[0] >= steps_sent[1])
//  {
//    max_steps = steps_sent[0];
//    x_larger = true;
//  }
//  else
//  {
//    max_steps = steps_sent[1];
//    y_larger = true;
//  }
//
//  if (x_larger == true)
//  {
//    for(long i=0; i < max_steps; i++){ 
//      digitalWrite(STEP_PIN_0, HIGH);
//      if (y_flag == false)
//      {
//         digitalWrite(STEP_PIN_1, HIGH);
//      }
//      delayMicroseconds(usDelay); 
//  
//      digitalWrite(STEP_PIN_0, LOW);
//      if (y_flag == false)
//      {
//         digitalWrite(STEP_PIN_1, LOW);
//      } 
//      delayMicroseconds(usDelay);
//      if (i == steps_sent[1])
//      {
//        y_flag = true; 
//      }
//    }
//  }
//  else if (y_larger == true)
//  {
//    for(long i=0; i < max_steps; i++){ 
//      digitalWrite(STEP_PIN_1, HIGH);
//      if (x_flag == false)
//      {
//         digitalWrite(STEP_PIN_0, HIGH);
//      }
//      delayMicroseconds(usDelay); 
//  
//      digitalWrite(STEP_PIN_1, LOW);
//      if (x_flag == false)
//      {
//         digitalWrite(STEP_PIN_0, LOW);
//      } 
//      delayMicroseconds(usDelay);
//      if (i == steps_sent[0])
//      {
//        x_flag = true; 
//      }
//    } 
//  }
//}

long reassemblebytes(unsigned char byte0, unsigned char byte1, unsigned char byte2, unsigned char byte3) {
  byte in[4] = {byte3, byte2, byte1, byte0};  //byte array is big endian
  long *lval = (long *)in;
  return *lval;
}

long readfourbytes(void) {
  unsigned char byte0, byte1, byte2, byte3;
  long output;
  int iter = 0;
  bool ignore_flag = false;
  if (Serial.available())
  {
    while (Serial.available() < 4) 
    {
      iter++;
      if (iter > 100) {
      ignore_flag = true;
      break;
      }
    }
  }
  if (ignore_flag==false)
  {
    byte0 = Serial.read();
    byte1 = Serial.read();
    byte2 = Serial.read();
    byte3 = Serial.read();
//    Serial.write(byte3);
//    Serial.write(byte2);
//    Serial.write(byte1);
//    Serial.write(byte0);
    
    output = reassemblebytes(byte0, byte1, byte2, byte3);
    return output;
  }
}


void serial_flush()
{
  if (Serial.available())
  {
    char dump;
    while(Serial.available())
    {
       dump = Serial.read();
    }
  }
}

int readtwobytes(void){
    unsigned char msb, lsb;
    int output;
    int iter = 0;
    while (Serial.available() <2){
      iter++;
      if (iter > 1e5){
  break;
      }
    }
    msb = Serial.read();
    lsb = Serial.read();
    output = reassemblebytes(0, 0, msb, lsb);
    return output;
}

void SendFourByteLong(long longin){
    unsigned char byte0, byte1, byte2, byte3;  //big endian
    byte0 = (unsigned char)longin;
    byte1 = (unsigned char)(longin>>8);
    byte2 = (unsigned char)(longin>>16);
    byte3 = (unsigned char)(longin>>24);
    Serial.write(byte3);
    Serial.write(byte2);
    Serial.write(byte1);
    Serial.write(byte0);
}
