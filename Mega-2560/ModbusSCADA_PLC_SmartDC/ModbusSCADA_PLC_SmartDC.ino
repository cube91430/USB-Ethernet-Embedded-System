

#include <Ethernet.h>
#include <ModbusTCPSlave.h>
#include <SPI.h>
#include <DHT.h>
#include <PZEM004Tv30.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <MQUnifiedsensor.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

const int ledPin = LED_BUILTIN;

unsigned long ResultRead, ResultSend;

//Pzem Current Sensor UART Interface
PZEM004Tv30 pzem1(&Serial1);  //current sensor1
PZEM004Tv30 pzem2(&Serial2);  //current sensor2

#define DHTTYPE DHT11
DHT dht1(22, DHTTYPE);
DHT dht2(23, DHTTYPE);

//DHT Sensor Output
float Humidity0, Humidity1;        //Humidity Measurement
float Temperature0, Temperature1;  //Temperature Measurement

//Control Relay
const int relay_input;     //Relay Control Input
volatile int relay0 = 23;  //Relay pin (23)
volatile int relay1 = 25;  //Relay pin (25)
volatile int relay2 = 27;  //Relay pin (27)
volatile int relay3 = 29;  //Relay pin (29)
volatile int relay4 = 31;  //Relay pin (31)
volatile int relay5 = 33;  //Relay pin (33)
volatile int relay6 = 35;  //Relay pin (35)
volatile int relay7 = 37;  //Relay pin (37)
volatile int relay8 = 39;  //Relay pin (39)
volatile int relay9 = 41;  //Relay pin (41)

float voltage0, voltage1;  //Voltage Measurement
float current0, current1;  //Current Measurement
float power0, power1;      //Power Measurement
float pf0, pf1;            //Power Factor Measurement
float energy0, energy1;    //Energy Measurement
float freq0, freq1;        //Frequency Measurement


// Your MAC address may be different, see your ethernet shield documentation.
// If using an Adruino Ethernet Shield 2, there should be a sticker on the back telling you what this should be.
uint8_t macAddress[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x13, 0xA0 };

const int8_t buttonPins[2] = { 2, 3 };
const int8_t ledPins[4] = { 5, 6, 7, 8 };
const int8_t knobPins[2] = { A0, A1 };

#define MODBUS_TCP_SLAVE_DEFAULT_PORT 1502
EthernetServer server(MODBUS_TCP_SLAVE_DEFAULT_PORT);
ModbusTCPSlave modbus;

bool coils[16];
bool discreteInputs[10];
uint16_t holdingRegisters[24];
uint16_t inputRegisters[10];


void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000)
    ;

  pinMode(relay0, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  pinMode(relay7, OUTPUT);
  pinMode(relay8, OUTPUT);
  pinMode(relay9, OUTPUT);

  modbus.configureCoils(coils, 16);
  modbus.configureDiscreteInputs(discreteInputs, 10);
  modbus.configureHoldingRegisters(holdingRegisters, 24);
  modbus.configureInputRegisters(inputRegisters, 10);

  IPAddress ip(192, 168, 10, 250);     // IP static untuk Arduino
  IPAddress dns(8, 8, 8, 8);           // DNS server (Google DNS)
  IPAddress gateway(192, 168, 10, 1);  // Gateway (router)        - Flexible (Can be changed)
  IPAddress subnet(255, 255, 255, 0);

  Serial.println("Connecting");
  Ethernet.begin(macAddress, ip, dns, gateway, subnet);
  // Tampilkan informasi IP dan jaringan
  Serial.println("===== Network Configuration =====");
  Serial.print("MAC Address: ");
  for (byte i = 0; i < 6; i++) {
    Serial.print(macAddress[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  Serial.print("Gateway: ");
  Serial.println(Ethernet.gatewayIP());

  Serial.print("Subnet Mask: ");
  Serial.println(Ethernet.subnetMask());

  Serial.print("DNS Server: ");
  Serial.println(Ethernet.dnsServerIP());

  Serial.print("Modbus TCP Listening on port: ");
  Serial.println(MODBUS_TCP_SLAVE_DEFAULT_PORT);

  Serial.println("=================================");


  server.begin();
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      Real_Data();  //Read Real Data Sensor
      //dummy_data();

      output_coil();  //Control Output with Coil

      modbus.poll(client);
    }
  }
}

void output_coil() {
  digitalWrite(relay0, coils[0]);
  digitalWrite(relay1, coils[1]);
  digitalWrite(relay2, coils[2]);
  digitalWrite(relay3, coils[3]);
  digitalWrite(relay4, coils[4]);
  digitalWrite(relay5, coils[5]);
  digitalWrite(relay6, coils[6]);
  digitalWrite(relay7, coils[7]);
  digitalWrite(relay8, coils[8]);
  digitalWrite(relay9, coils[9]);
}

void Real_Data() {
  //Temperature - Variable
  holdingRegisters[0] = dht1.readHumidity();
  holdingRegisters[1] = dht2.readHumidity();
  holdingRegisters[2] = dht1.readTemperature();
  holdingRegisters[3] = dht2.readTemperature();

  //Power Meter - Variable
  holdingRegisters[4] = pzem1.voltage();
  holdingRegisters[5] = pzem2.voltage();
  holdingRegisters[6] = pzem1.current();
  holdingRegisters[7] = pzem2.current();

  holdingRegisters[8] = pzem1.pf();
  holdingRegisters[9] = pzem2.pf();
  holdingRegisters[10] = pzem1.frequency();
  holdingRegisters[11] = pzem2.frequency();
  holdingRegisters[12] = pzem1.energy();
  holdingRegisters[13] = pzem2.energy();
}

void dummy_data() {
  //Temperature - Variable
  holdingRegisters[0] = dht1.readHumidity();
  holdingRegisters[1] = dht2.readHumidity();
  holdingRegisters[2] = dht1.readTemperature();
  holdingRegisters[3] = dht2.readTemperature();

  //Power Meter - Variable
  holdingRegisters[4] = pzem1.voltage();
  holdingRegisters[5] = pzem2.voltage();
  holdingRegisters[6] = pzem1.current();
  holdingRegisters[7] = pzem2.current();

  holdingRegisters[8] = pzem1.pf();
  holdingRegisters[9] = pzem2.pf();
  holdingRegisters[10] = pzem1.frequency();
  holdingRegisters[11] = pzem2.frequency();
  holdingRegisters[12] = pzem1.energy();
  holdingRegisters[13] = pzem2.energy();
}