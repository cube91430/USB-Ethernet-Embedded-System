/*

Upcoming Update: 
1. PID settings, lets the master set a max and min temperature or value
2  Check EEPROM config
3. Edge can change ID with a specific protocol 
   - Flash Bootloader or Firmware by Edge
4. Write Seperate Function to Read and Write
5. Edge can set How many time a second to Write data
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <TaskScheduler.h>
#include <PZEM004Tv30.h>          // - Baudrate 9600
#include <MQUnifiedsensor.h>
//#include "parse_data.cpp"

const int MAX_VALUES = 10; // Maximum number of values to store
String values[MAX_VALUES];  // Array to hold the parsed values
int valueCount = 0;        // Counter for number of values received

String data_array[6];  //Send Sensor and Relay Data

/* --- Digital INPUT --- */
volatile int digi_pin0 = 2;   //Clean Digital Transmission without Signal
volatile int digi_pin1 = 4;   //Clean Digital Transmission without Signal
volatile int digi_pin2 = 15;  //Clean Digital Transmission without Signal

/* --- Analog Input --- */
#define adc0 12
#define adc1 13
#define adc2 14
#define adc3 27
#define adc4 26
#define adc5 25

/* --- OUTPUT Relay - Trigger --- */
const int out0 = 32;
const int out1 = 33;
const int out2 = 34;
const int out3 = 35;
const int out4 = 36;
const int out5 = 39;

volatile int led = 2;
int eeprom_val;

int potenvalue;
volatile int output_sensor0;

volatile int adc_val0;
volatile int adc_val1;
volatile int adc_val2;  
volatile int adc_val3;  
volatile int adc_val4;  
volatile int adc_val5;  

volatile bool digi_val0;
volatile bool digi_val1;
volatile bool digi_val2;

String sensor0;
String sensor1;
String sensor2;
String sensor3;
String sensor4;
String sensor5;

byte time0;
byte time1;
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1000;  //the value is a number of milliseconds

/* --- --- */
int analog_to_digital(int num, int pin) {
    // Initialize EEPROM (must be done in setup() first)
    EEPROM.begin(512);  // Use appropriate size
    
    // Read configuration bit from EEPROM
    bool is_analog = bitRead(EEPROM.read(0), num);
    
    // Read the appropriate pin type
    if (is_analog) {
        return analogRead(pin);  // Returns 0-4095 on ESP32
    } else {
        return !digitalRead(pin); // Returns inverted digital (0 or 1)
    }
}

//To Trigger Output Every Time Difference
void blink_send_data (int time_each) {
  /* -- function to send data -- */
  delay(time_each);
}

//Trigger Output Manually
void trigger_output (const int pin_number, int bin_locate) {
  //digitalWrite(pin_number, bitRead(EEPROM.read(9), bin_locate));  //0 = ON, 1 = OFF
}


//DHT_Unified dht(DHTPIN, DHTTYPE);

void WritetoSend() {
    data_array = String(adc_val0) + "#" + String(adc_val1) + "#" + String(adc_val2) + "#" String(adc_val3) + "#" + String(adc_val4) + "#" + String(adc_val5);
    //Serial.print(data_array);

}

void setup() {
  Serial.begin(115200);
  //dht.begin();
                            
  if (!EEPROM.begin(512)) {     // Allocate 512 bytes of EEPROM
    Serial.println("Failed to init EEPROM");
    return;
  }

  pinMode(digi_pin0, INPUT);
  pinMode(digi_pin1, INPUT);
  pinMode(digi_pin2, INPUT);

  pinMode(adc0, INPUT);
  pinMode(adc1, INPUT);
  pinMode(adc2, INPUT);
  pinMode(adc3, INPUT);
  pinMode(adc4, INPUT);
  pinMode(adc5, INPUT);

  pinMode(out0, OUTPUT);
  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(out3, OUTPUT);
  pinMode(out5, OUTPUT);

  pinMode(out4, INPUT); //Trial Potentiometer
  pinMode(led, OUTPUT); //Trial LED - Output

}

void loop() {
  
  adc_val0 = analog_to_digital(0, adc0);  
  adc_val1 = analog_to_digital(0, adc1);  
  adc_val2 = analog_to_digital(0, adc2);  
  adc_val3 = analog_to_digital(0, adc3);  
  adc_val4 = analog_to_digital(0, adc4);  
  adc_val5 = analog_to_digital(0, adc5);  

  digi_val0 = !digitalRead(digi_pin0);
  digi_val1 = !digitalRead(digi_pin1);
  digi_val2 = !digitalRead(digi_pin2);

  /*

  trigger_output(out0, 0);
  trigger_output(out1, 1);
  trigger_output(out2, 2);
  trigger_output(out3, 3);
  trigger_output(out5, 4);

  */
    
  //Serial.println(sensorValue);

  Serial.println();

  delay(50);
  
}

void parseStringToArray(String data, char delimiter, String* output, int maxItems) {
  int count = 0;
  int index = 0;
  int lastIndex = 0;
  
  // Clear previous values
  for (int i = 0; i < maxItems; i++) {
    output[i] = "";
  }
  
  // Parse the string
  while (index >= 0 && count < maxItems) {
    index = data.indexOf(delimiter, lastIndex);
    if (index == -1) {
      // Last item
      output[count] = data.substring(lastIndex);
      count++;
    } else {
      output[count] = data.substring(lastIndex, index);
      count++;
      lastIndex = index + 1;
    }
  }
}  

void ReadtoRead() {
  if (Serial.available() > 0) {
    String read_data = Serial.readStringUntil('\n');
    read_data.trim();

    parseStringToArray(read_data, '#', values, MAX_VALUES);
    
    if (MAX_VALUES > 0) sensor0 = values[0];
    if (MAX_VALUES > 1) sensor1 = values[1];
    if (MAX_VALUES > 2) sensor2 = values[2];
    if (MAX_VALUES > 3) sensor3 = values[3];
    if (MAX_VALUES > 4) sensor4 = values[4];
    if (MAX_VALUES > 5) sensor5 = values[5];

  }
}

