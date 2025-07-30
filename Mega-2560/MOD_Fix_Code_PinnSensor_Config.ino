//This is a Dummy_Data from the Arduino Mega

#include <Ethernet.h>
#include <ArduinoModbus.h>
#include <DHT.h>
#include <SPI.h>
#include <MQ2.h>
#include <EEPROM.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
//#include <Arduino_FreeRTOS.h>
#include <TaskScheduler.h>

#define DHTTYPE DHT22
//DHT dht1(2, DHTTYPE);

//Pzem Initialization
PZEM004Tv30 pzem1(&Serial1);
PZEM004Tv30 pzem2(&Serial2);

byte mac[] = { 0xBC, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // match your MAC address
IPAddress ip(192, 168, 1, 177);                       // match your Arduino IP
IPAddress neuronEX(127, 0, 0, 1);                     // NeuronEX IP

EthernetServer server(502);  //Sending Data
ModbusTCPServer modbusTCP;

//EthernetClient client;
int NeuronPort = 7000;

EthernetClient client;

int Hold_Regis;

unsigned long lastUpdateTime = 0;
unsigned long startTime;
unsigned long duration = 20 * 60 * 1000UL;  // 20 minutes in milliseconds

// Simulation variables
int voltage, current, powerFactor, power, energy_kWh, frequency;
int voltage1, current1, powerFactor1, power1, energy_kWh1, frequency1;
//int temperature, humidity;

const int relay0 = 22;
const int relay1 = 24;
const int relay2 = 26;
const int relay3 = 28;
const int relay4 = 30;
const int relay5 = 32;
const int relay6 = 34;
const int relay7 = 36;
const int relay8 = 38;

volatile bool relay_sense0 = 23;
volatile bool relay_sense1 = 25;
volatile bool relay_sense2 = 27;
volatile bool relay_sense3 = 29;
volatile bool relay_sense4 = 31;
volatile bool relay_sense5 = 33;
volatile bool relay_sense6 = 35;
volatile bool relay_sense7 = 37;
volatile bool relay_sense8 = 39;

void digital_pinMode() {

  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  pinMode(16, INPUT);
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  pinMode(20, INPUT);
  pinMode(21, INPUT);
  pinMode(42, INPUT);
  pinMode(43, INPUT);
  pinMode(44, INPUT);
  pinMode(45, INPUT);
  pinMode(46, INPUT);
  pinMode(47, INPUT);
  pinMode(48, INPUT);
  pinMode(49, INPUT);
}

void bit_sensor_config(int coil, int eeprom_num) {
  if (modbusTCP.coilRead(coil) == 1) {
    EEPROM.write(eeprom_num, 1);
  } else {
    EEPROM.write(eeprom_num, 0);
  }
}

void update_pinout_dht(
  int eeprom_num, int pin_number, int temp_id, int hum_id, int ori_holdid) {
  if (EEPROM.read(eeprom_num) == 1) {
    //Initialize DHT Pin
    DHT dht(pin_number, DHTTYPE);
    dht.begin();
    //Read DHT Sensor Pin
    modbusTCP.holdingRegisterWrite(temp_id, dht.readTemperature());  //Read Temperature
    modbusTCP.holdingRegisterWrite(hum_id, dht.readHumidity());      //Read Humidity
  } else {
    //Digital Read
    modbusTCP.holdingRegisterWrite(ori_holdid, digitalRead(pin_number));
  }
}

void updateHoldingRegister() {
  modbusTCP.holdingRegisterWrite(1, digitalRead(2));  //1!400002
  modbusTCP.holdingRegisterWrite(2, digitalRead(3));  //1!400003
  modbusTCP.holdingRegisterWrite(3, digitalRead(4));  //1!400004
  modbusTCP.holdingRegisterWrite(4, digitalRead(5));  //1!400005
  modbusTCP.holdingRegisterWrite(5, digitalRead(6));  //1!400006
  modbusTCP.holdingRegisterWrite(6, digitalRead(7));  //1!400007
  modbusTCP.holdingRegisterWrite(7, digitalRead(8));  //1!400008
  modbusTCP.holdingRegisterWrite(8, digitalRead(9));  //1!400009

  //DHT - Enabled Pins
  /*modbusTCP.holdingRegisterWrite(9, digitalRead(10));   //1!400010
  modbusTCP.holdingRegisterWrite(10, digitalRead(11));  //1!400011
  modbusTCP.holdingRegisterWrite(11, digitalRead(12));  //1!400012
  modbusTCP.holdingRegisterWrite(12, digitalRead(13));*/
  //1!400013
  update_pinout_dht(0, 10, 50, 51, 9);   //1!400010
  update_pinout_dht(1, 11, 51, 52, 10);  //1!400011
  update_pinout_dht(2, 12, 52, 53, 11);  //1!400012
  update_pinout_dht(3, 13, 53, 54, 12);  //1!400013

  //Pzem - Enabled Pins
  //modbusTCP.holdingRegisterWrite(13, digitalRead(14));  //1!400014
  //modbusTCP.holdingRegisterWrite(14, digitalRead(15));  //1!400015
  //modbusTCP.holdingRegisterWrite(15, digitalRead(16));  //1!400016
  //modbusTCP.holdingRegisterWrite(16, digitalRead(17));  //1!400017

  modbusTCP.holdingRegisterWrite(17, digitalRead(18));  //1!400018
  modbusTCP.holdingRegisterWrite(18, digitalRead(19));  //1!400019
  modbusTCP.holdingRegisterWrite(19, digitalRead(20));  //1!400020
  modbusTCP.holdingRegisterWrite(20, digitalRead(21));  //1!400021

  modbusTCP.holdingRegisterWrite(22, digitalRead(42));  //1!400023
  modbusTCP.holdingRegisterWrite(23, digitalRead(43));  //1!400024
  modbusTCP.holdingRegisterWrite(24, digitalRead(44));  //1!400025
  modbusTCP.holdingRegisterWrite(25, digitalRead(45));  //1!400026
  modbusTCP.holdingRegisterWrite(26, digitalRead(46));  //1!400027
  modbusTCP.holdingRegisterWrite(27, digitalRead(47));  //1!400028
  modbusTCP.holdingRegisterWrite(28, digitalRead(48));  //1!400029
  modbusTCP.holdingRegisterWrite(29, digitalRead(49));  //1!400030
}

void updateCoil() {
  digitalWrite(relay0, modbusTCP.coilRead(0));  //1!0001
  digitalWrite(relay1, modbusTCP.coilRead(1));  //1!0002
  digitalWrite(relay2, modbusTCP.coilRead(2));  //1!0003
  digitalWrite(relay3, modbusTCP.coilRead(3));  //1!0004
  digitalWrite(relay4, modbusTCP.coilRead(4));  //1!0005
  digitalWrite(relay5, modbusTCP.coilRead(5));  //1!0006
  digitalWrite(relay6, modbusTCP.coilRead(6));  //1!0007
  digitalWrite(relay7, modbusTCP.coilRead(7));  //1!0008
}

void DiscreteInput_Write(int coil, int arr) {
  if (digitalRead(arr) == 0) {
    modbusTCP.discreteInputWrite(coil, 1);
  } else {
    modbusTCP.discreteInputWrite(coil, 0);
  }
}

void WriteDiscreteInput() {
  DiscreteInput_Write(0, 23);  //1!10001
  DiscreteInput_Write(1, 25);  //1!10002
  DiscreteInput_Write(2, 27);  //1!10003
  DiscreteInput_Write(3, 29);  //1!10004
  DiscreteInput_Write(4, 31);  //1!10005
  DiscreteInput_Write(5, 33);  //1!10006
  DiscreteInput_Write(6, 35);  //1!10007
  DiscreteInput_Write(7, 37);  //1!10008
  DiscreteInput_Write(8, 39);  //1!10009
}


void setup() {
  Serial.begin(9600);
  //dht.begin();
  //mq2.begin();

  Ethernet.begin(mac, ip);
  if (!modbusTCP.begin()) {
    Serial.println("Failed to start Modbus TCP Server!");
    while (1)
      ;
  }
  Serial.println("Modbus TCP Server started");
  modbusTCP.configureHoldingRegisters(0x00, 100);
  modbusTCP.configureCoils(0x00, 40);
  modbusTCP.configureDiscreteInputs(0x00, 40);

  client.connect(neuronEX, NeuronPort);
  server.begin();

  digital_pinMode();

  pinMode(relay0, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  pinMode(relay7, OUTPUT);

  delay(2000);  //Delay from start

  startTime = millis();
}

void loop() {

  EthernetClient client = server.available();

  if (client) {
    modbusTCP.accept(client);

    Serial.println("Neuron Connected!");

    if (!modbusTCP.holdingRegisterWrite(4, 10) == 1 && !modbusTCP.coilRead(1) == 1) {
      //1!400001
      Serial.print("Writing Failed");
    }
    Serial.println("Writing Data");

    bit_sensor_config(19, 0);  //1!00020 - DHT0
    bit_sensor_config(20, 1);  //1!00021 - DHT1
    bit_sensor_config(21, 2);  //1!00022 - DHT2
    bit_sensor_config(22, 3);  //1!00023 - DHT3
    bit_sensor_config(23, 4);  //1!00024 - Pzem0
    bit_sensor_config(24, 5);  //1!00025 - Pzem1
    bit_sensor_config(25, 6);  //1!00026 - MQ2_0
    bit_sensor_config(26, 7);  //1!00027 - MQ2_1
    bit_sensor_config(27, 8);  //1!00028 - MQ2_2
    bit_sensor_config(28, 9);  //1!00029 - MQ2_3

    if (EEPROM.read(23) == 1) {
      modbusTCP.holdingRegisterWrite(58, pzem1.voltage());    //1!400059
      modbusTCP.holdingRegisterWrite(59, pzem1.current());    //1!400060
      modbusTCP.holdingRegisterWrite(60, pzem1.pf());         //1!400061
      modbusTCP.holdingRegisterWrite(61, pzem1.energy());     //1!400062
      modbusTCP.holdingRegisterWrite(62, pzem1.frequency());  //1!400063
    } else {
      modbusTCP.holdingRegisterWrite(13, digitalRead(14));  //1!400014
      modbusTCP.holdingRegisterWrite(14, digitalRead(15));  //1!400015
    }

    if (EEPROM.read(24) == 1) {
      modbusTCP.holdingRegisterWrite(63, pzem2.voltage());    //1!400064
      modbusTCP.holdingRegisterWrite(64, pzem2.current());    //1!400065
      modbusTCP.holdingRegisterWrite(65, pzem2.pf());         //1!400066
      modbusTCP.holdingRegisterWrite(66, pzem2.energy());     //1!400067
      modbusTCP.holdingRegisterWrite(67, pzem2.frequency());  //1!400068
    } else {
      modbusTCP.holdingRegisterWrite(15, digitalRead(16));  //1!400016
      modbusTCP.holdingRegisterWrite(16, digitalRead(17));  //1!400017
    }
    
    updateHoldingRegister();  //Update Holding Registers
    updateCoil();  //Update Coils
    WriteDiscreteInput();  //Update DiscreteInputs

    modbusTCP.accept(client);
    modbusTCP.poll();
  }
  //delay(100);
}
/*
void resetSimulation() {
  startTime = millis();
  lastUpdateTime = millis();

  // Electrical values
  voltage = 230.0;
  current = 5.0;
  powerFactor = 0.95;
  power = 0.0;
  energy_kWh = 0.0;
  frequency = 50.0;

  // Environmental values
  //temperature = 25.0;
  //humidity = 45.0;

  Serial.println("🔁 Simulation Reset. Starting New Cycle...");
}
*/

/*
void simulateNormalBehavior() {
  // Electrical fluctuations
  voltage += random(-10, 11) * 0.1;      // ±1V
  current += random(-20, 21) * 1;        // ±0.2A
  powerFactor += random(-5, 6) * 0.001;  // ±0.005
  frequency += random(-2, 3) * 0.01;     // ±0.02Hz

  // Environmental fluctuations
  //temperature += random(-5, 6) * 0.1;  // ±0.5°C
  //humidity += random(-3, 4) * 0.5;     // ±1.5%

  // Clamp electrical values
  voltage = constrain(voltage, 210, 240);
  current = constrain(current, 0, 10);
  powerFactor = constrain(powerFactor, 0.7, 1.0);
  frequency = constrain(frequency, 49.8, 50.2);

  // Clamp environmental values
  //temperature = constrain(temperature, 20, 45);
  //humidity = constrain(humidity, 30, 80);
}
*/