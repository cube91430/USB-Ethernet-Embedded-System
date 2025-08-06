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
#include <HardwareSerial.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPI.h>

const char* mqtt_topic = "/home/lst_smartdc01/rack01";
const char* mqtt_server = "internal.mqtt.lawangsewu.com";

String wifi_ssid;
String wifi_pass;

char ssid[] = "LST";
char pass[] = "office45@pwt";

int status = WL_IDLE_STATUS;
WiFiServer wifi_server(80);

/**** DHT Sensor Initialization I****/
#define DHTTYPE DHT22
//DHT dht1(2, DHTTYPE);

//Pzem Initialization
PZEM004Tv30 pzem1(&Serial1);
PZEM004Tv30 pzem2(&Serial2);

volatile int port_input;                              //Ethernet-Server-Port
byte mac[] = { 0xBC, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // match your MAC address
/*
IPAddress ip(192, 168, 1, 177);                       // match your Arduino IP
IPAddress neuronEX(127, 0, 0, 1);                     // NeuronEX IP
*/
//EthernetServer server(502);  //Sending Data

EthernetServer server(port_input);  //Sending Data
ModbusTCPServer modbusTCP;

//EthernetClient client;
//int NeuronPort = 7000;

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

String blue_arr;
String ip_data[11];   // This will hold the string parts
int parsed_data[11];  // This will hold the parsed integers
bool parsing = false;
int req, a, b, c, d, e, f, g, h, i, j;

int ip_parse0, ip_parse1, ip_parse2, ip_parse3, ip_parse4, ip_parse5, ip_parse6, ip_parse7, ip_parse8, ip_parse9;

void Bluetooth_Config() {
  while (Serial3.available()) {
    String data_en = Serial3.readString();
    blue_arr += data_en;

    if (data_en.indexOf('$') != -1) {
      parsing = true;
      // Now parse the data
      int q = 0;
      int startIdx = blue_arr.indexOf('$') + 1;  // Skip the $
      int endIdx = blue_arr.indexOf('#', startIdx);

      while (endIdx != -1 && q < 11) {
        ip_data[q] = blue_arr.substring(startIdx, endIdx);
        parsed_data[q] = ip_data[q].toInt();  // Convert to integer
        startIdx = endIdx + 1;
        endIdx = blue_arr.indexOf('#', startIdx);
        q++;
      }

      // Get the last part after the last #
      if (q < 11 && startIdx < blue_arr.length()) {
        ip_data[q] = blue_arr.substring(startIdx);
        parsed_data[q] = ip_data[q].toInt();
      }

      // Assign to your variables
      req = parsed_data[0];
      a = parsed_data[1];
      b = parsed_data[2];
      c = parsed_data[3];
      d = parsed_data[4];
      e = parsed_data[5];
      f = parsed_data[6];
      g = parsed_data[7];
      h = parsed_data[8];
      i = parsed_data[9];
      j = parsed_data[10];

      EEPROM.write(38, a);  //192
      EEPROM.write(39, b);  //168
      EEPROM.write(40, c);  //1
      EEPROM.write(41, d);  //177
      EEPROM.write(42, e);  //127
      EEPROM.write(43, f);  //0
      EEPROM.write(44, g);  //0
      EEPROM.write(45, h);  //1
      EEPROM.write(46, i);  //502
      EEPROM.write(47, j);  //7000

      // Reset for next message
      blue_arr = "";
      parsing = false;
    }
  }
}

void config_from_bluetooth() {

  port_input = EEPROM.read(28);

  ip_parse0 = EEPROM.read(20);
  ip_parse1 = EEPROM.read(21);
  ip_parse2 = EEPROM.read(22);
  ip_parse3 = EEPROM.read(23);

  ip_parse4 = EEPROM.read(24);
  ip_parse5 = EEPROM.read(25);
  ip_parse6 = EEPROM.read(26);
  ip_parse7 = EEPROM.read(27);
  ip_parse8 = EEPROM.read(29);
}

void WiFi_Config() {

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true)
      ;  // don't continue
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);  // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  wifi_server.begin();     // start the web server on port 80
  //printWifiStatus();  // you're connected now, so print out the status
}

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

void update_pinout_dht(int eeprom_num, int pin_number, int temp_id, int hum_id, int ori_holdid) {
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

void update_pinout_pzem() {
  return;
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
  /*modbusTCP.holdingRegisterWrite(9, digitalRead(10)); //1!400010
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
  Serial.begin(115200);
  Serial3.begin(9600);

  /******************** INITIALIZE BLUETOOTH ****************************/
  Bluetooth_Config();
  config_from_bluetooth();

  /******************** INITIALIZE WIFI - CONFIG ************************/
  WiFi_Config();

  /******************** INITIALIZE ETHERNET CONFIG *********************/
  IPAddress ip(ip_parse0, ip_parse1, ip_parse2, ip_parse3);        // match your Arduino IP
  IPAddress neuronEX(ip_parse4, ip_parse5, ip_parse6, ip_parse7);  // NeuronEX IP

  int NeuronPort = ip_parse8;

  /******** ETHERNET - RUNNING  ********/
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
  /*
  if (!client.connect(neuronEX, NeuronPort) == 1) {
    Serial.println("Neuron CONNECTED!");
  } else {
    Serial.println("Neuron NOT CONNECTED!");
  }
  */

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
  WiFiClient wifi_client = wifi_server.available();   // listen for incoming clients
  EthernetClient client = server.available();

  if (client) {
    modbusTCP.accept(client);

    Serial.println("Neuron Connected!");

    if (!modbusTCP.holdingRegisterWrite(0, 0) == 1 && !modbusTCP.coilRead(1) == 1) {
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
    updateCoil();             //Update Coils
    WriteDiscreteInput();     //Update DiscreteInputs

    modbusTCP.accept(client);
    modbusTCP.poll();
  }
  //delay(100);
}