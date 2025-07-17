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
//#include <DHT.h>
//#include <TaskScheduler.h>
//#include <CleanRTOS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <PZEM004Tv30.h>  // - Baudrate 9600
#include <MQUnifiedsensor.h>
#include <PubSubClient.h>
//#include "parse_data.cpp"

// WiFi
const char *ssid = "LST"; // Enter your WiFi name
const char *password = "";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "emqx/esp32";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

const int MAX_VALUES = 10;  // Maximum number of values to store
String values[MAX_VALUES];  // Array to hold the parsed values
int valueCount = 0;         // Counter for number of values received

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

String source_id;
String data_id;

unsigned char analog_con;
unsigned char digital_con;
unsigned char signal_con;
String control_bin;

String data_id_control;
String data_temp;
String data_power;

String data_id_temp = "0345";
String dlc_data = "8";
String data_id_volt = "1221";
String device_id = "872";

byte time0;
byte time1;

unsigned long previousMillis = 0;
const long interval = 100;  // 1 second interval

/* --- SENDING DATA TO HOST --- */
//Send data to Ethernet

//Protocol to send data (Device_ID#Data_ID#DLC#Byte1#Byte2#Byte3#Byte4)
void send_temperature() {
  Serial.print(device_id);
  Serial.print("#");
  Serial.print(data_id_temp);
  Serial.print("#");
  Serial.print(dlc_data);
  Serial.print("#");
  //Serial.print(adc_val0, 4);
  Serial.print(120);
  Serial.print("#");
  Serial.print(adc_val1, 4);
  Serial.print("#");
  Serial.println(adc_val2, 4);
}
void send_powermeter() {
  Serial.print(device_id);
  Serial.print("#");
  Serial.print(data_id_volt);
  Serial.print("#");
  Serial.print(dlc_data);
  Serial.print("#");
  Serial.print(adc_val3, 4);
  Serial.print("#");
  Serial.print(adc_val4, 4);
  Serial.print("#");
  Serial.print(adc_val5, 4);
  Serial.print("#");
  Serial.print(digi_val1);
  Serial.print("#");
  Serial.println(digi_val2);
}

/* ---- READ DATA FROM HOST ---- */
//Read Serial data from Ethernet
void ReadtoPinConfig() {
  if (Serial.available() > 0) {
    String read_data = Serial.readStringUntil('\n');
    read_data.trim();

    parseStringToArray(read_data, '#', values, MAX_VALUES);

    if (MAX_VALUES > 0) source_id = values[0];
    if (MAX_VALUES > 1) data_id_control = values[1];
    if (MAX_VALUES > 2) analog_con = values[2].toInt();
    if (MAX_VALUES > 3) digital_con = values[3].toInt();
    if (MAX_VALUES > 4) signal_con = values[4].toInt();
    if (MAX_VALUES > 5) control_bin = values[5];
  }
}

//Trigger Output Manually
void trigger_output(const int pin_number, int bin_locate) {
  digitalWrite(pin_number, bitRead(control_bin.toInt(), bin_locate));  //0 = ON, 1 = OFF
}

//Parsing Data from Ethernet
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

void storing_adc_dac_signal() {
  EEPROM.begin(512);  //Sets EEPROM - data structure

  EEPROM.write(1, analog_con);   //Analog - DAC Config
  EEPROM.write(2, digital_con);  //Digital - Signal Config
  EEPROM.write(3, signal_con);   //Signal - DAC Config
  EEPROM.commit();
}

int analog_to_digital(int num, int pin) {
  // Initialize EEPROM (must be done in setup() first)
  EEPROM.begin(512);  // Use appropriate size

  // Read configuration bit from EEPROM
  bool is_analog = bitRead(EEPROM.read(0), num);

  // Read the appropriate pin type
  if (is_analog) {
    return analogRead(pin);  // Returns 0-4095 on ESP32
  } else {
    return !digitalRead(pin);  // Returns inverted digital (0 or 1)
  }
}



void setup() {
  Serial.begin(115200);
  //dht.begin();

  if (!EEPROM.begin(512)) {  // Allocate 512 bytes of EEPROM
    Serial.println("Failed to init EEPROM");
    return;
  }

  //DEVICE ID, ID, DLC, DATA(HEX) - For multi Microcontroller communication wit Edge
  //F#10#566#8585

  pinMode(digi_pin0, INPUT);
  pinMode(digi_pin1, INPUT);
  pinMode(digi_pin2, INPUT);

  pinMode(adc0, INPUT);
  pinMode(adc1, INPUT);
  pinMode(adc2, INPUT);
  pinMode(adc3, INPUT);
  pinMode(adc4, INPUT);
  pinMode(adc5, INPUT);
  /*
  pinMode(out0, OUTPUT);
  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(out3, OUTPUT);
  pinMode(out5, OUTPUT); */

  pinMode(out4, INPUT);  //Trial Potentiometer
  pinMode(led, OUTPUT);  //Trial LED - Output
}

void loop() {

    if (source_id == "1220" && data_id_control == "A345") {
      Serial.println("Data Received: ");
      storing_adc_dac_signal();
      ReadtoPinConfig();

      trigger_output(out0, 0);
      trigger_output(out1, 1);
      trigger_output(out2, 2);
      trigger_output(out3, 3);
      trigger_output(out5, 4);
      trigger_output(led, 5);

      Serial.print(control_bin);
      Serial.print("  ");
      Serial.print(control_bin.toInt(), HEX);
      Serial.print("  ");
      Serial.print(control_bin.toInt(), BIN);
    }
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //Setup Analog Input
    adc_val0 = analog_to_digital(0, adc0);
    adc_val1 = analog_to_digital(0, adc1);
    adc_val2 = analog_to_digital(0, adc2);
    adc_val3 = analog_to_digital(0, adc3);
    adc_val4 = analog_to_digital(0, adc4);
    adc_val5 = analog_to_digital(0, adc5);

    //Set up digital input
    digi_val0 = !digitalRead(digi_pin0);
    digi_val1 = !digitalRead(digi_pin1);
    digi_val2 = !digitalRead(digi_pin2);

    //send_temperature();
    //send_powermeter();
    //Serial.println(bitRead(EEPROM.read(0), 5));
    //Serial.println(bitRead(EEPROM.read(0), 6));
    Serial.print("test!");

    Serial.println();
  }
}
