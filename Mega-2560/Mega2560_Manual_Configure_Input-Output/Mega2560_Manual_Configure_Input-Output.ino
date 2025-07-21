//Mapping port problem

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoRS485.h>  // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <DHT.h>
#include <PZEM004Tv30.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <MQUnifiedsensor.h>

// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);  //Arduino IP
IPAddress server(127, 0, 0, 1);  //Neuron emqx ip
//SubAdress subnet(255, 255, 255, 0);

EthernetServer ethServer(502);  //Open Port from the Arduino to send data to Neuron
volatile int NeuronPort = 7000;
EthernetClient client;
ModbusTCPServer modbusTCPServer;

const int ledPin = LED_BUILTIN;

unsigned long ResultRead, ResultSend;

PZEM004Tv30 pzem1(&Serial1);
PZEM004Tv30 pzem2(&Serial2);

#define DHTTYPE DHT11
DHT dht1(22, DHTTYPE);
DHT dht2(23, DHTTYPE);

float Humidity0, Humidity1;
float Temperature0, Temperature1;

float voltage0, voltage1;
float current0, current1;
float power0, power1;
float pf0, pf1;
float energy0, energy1;
float freq0, freq1;

int relay_input;

const int relay0 = 27;
const int relay1 = 14;
const int relay2 = 12;
const int relay3 = 13;
const int relay4 = 26;

void bit_control(int num, int pin_number) {
  if (bitRead(relay_input, num) > 0) {
    digitalWrite(pin_number, HIGH);
  } else {
    digitalWrite(pin_number, LOW);
  }
}

void relay_control() {
  bit_control(0, relay0);
  bit_control(1, relay1);
  bit_control(2, relay2);
  bit_control(3, relay3);
  bit_control(4, relay4);

}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  dht1.begin();  //Cofigure DHT Sensors
  dht2.begin();

  Serial.println("Ethernet Modbus TCP - Smart Rack");
  Ethernet.begin(mac, ip);  // start the Ethernet connection and the server:
  //client.begin(mac, server);
  client.connect(server, NeuronPort);  //Neuron port
  ethServer.begin();                   // start the server
  modbusTCPServer.begin();             // start the Modbus TCP server


  modbusTCPServer.configureHoldingRegisters(0, 100);  //Configure holding register address and number of holding
  modbusTCPServer.configureHoldingRegisters(1, 100);  //Configure holding register address and number of holding
  modbusTCPServer.configureCoils(0, 100);             //Configure Coil Read (10 nb - Reserved)

  // configure the LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // configure a single coil at address 0x00
}

void loop() {
  //Temperature and Humidity Sensor Reading
  temperature_logger();
  //Power Meter
  power_meter();

  // listen for incoming clients
  EthernetClient client = ethServer.available();

  if (client) {
    // a new client connected
    Serial.println("port connected");

    // let the Modbus TCP accept the connection
    modbusTCPServer.accept(client);

    while (client.connected()) {
      // poll for Modbus TCP requests, while client connected
      modbusTCPServer.poll();

      // update the LED
      if (!modbusTCPServer.holdingRegisterWrite(0, 0) < 1) {
        Serial.println("Writing Failed");
      }

      modbusTCPServer.coilRead(1);  //Request Data          - 0!1
      //Holding Register - holdingRegisterWrite(address, value)
      //1 - 400001
      //2 - 400002

      //Sending - Temperature Data - id = 0
      modbusTCPServer.holdingRegisterWrite(1, Temperature0);  //Temperature   - 0     - 0!400002
      modbusTCPServer.holdingRegisterWrite(2, Temperature1);  //Temperature   - 1     - 0!400003
      modbusTCPServer.holdingRegisterWrite(3, Humidity0);     //Humidity      - 0     - 0!400004
      modbusTCPServer.holdingRegisterWrite(4, Humidity1);     //Humidity      - 1     - 0!400005
      modbusTCPServer.holdingRegisterWrite(5, 45);            //Flame Sensor  - ADC   - 0!400006
      modbusTCPServer.holdingRegisterWrite(6, 239);           //Flame Sensor  - DAC   - 0!400007
      modbusTCPServer.holdingRegisterWrite(7, 45);            //Gas Sensor    - ADC   - 0!400008
      modbusTCPServer.holdingRegisterWrite(8, 239);           //Gas Sensor    - DAC   - 0!400009

      //Sendig - Power Meter Data - id = 1
      modbusTCPServer.holdingRegisterWrite(10, voltage0);  //Voltage0        - 1     - 0!400011
      modbusTCPServer.holdingRegisterWrite(11, voltage1);  //Voltage1        - 1     - 0!400012
      modbusTCPServer.holdingRegisterWrite(12, current0);  //Current0        - 1     - 0!400013
      modbusTCPServer.holdingRegisterWrite(14, current1);  //Current1        - 1     - 0!400014
      modbusTCPServer.holdingRegisterWrite(15, power0);    //Power0          - 1     - 0!400015
      modbusTCPServer.holdingRegisterWrite(16, power1);    //Power1          - 1     - 0!400016
      modbusTCPServer.holdingRegisterWrite(17, pf0);       //Power Factor0   - 1     - 0!400017
      modbusTCPServer.holdingRegisterWrite(18, pf1);       //Power Factor1   - 1     - 0!400018
      modbusTCPServer.holdingRegisterWrite(19, freq0);     //Frequency0      - 1     - 0!400019
      modbusTCPServer.holdingRegisterWrite(20, freq1);     //Frequency1      - 1     - 0!400020
      modbusTCPServer.holdingRegisterWrite(21, energy0);   //Energy0         - 1     - 0!400021
      modbusTCPServer.holdingRegisterWrite(22, energy1);   //Energy1         - 1     - 0!400022

      //Reading for Relay - Input
      relay_input = modbusTCPServer.holdingRegisterWrite(9, 45);  //Output - Binary       - 0!400010
      relay_control();

      //Serial.print
    }

    //Serial.println("client disconnected");
  }

  //delay(5);
}

void temperature_logger() {
  //Temperature and Humidity Sensor Reading
  Humidity0 = dht1.readHumidity();
  Humidity1 = dht2.readHumidity();

  Temperature0 = dht1.readTemperature();
  Temperature1 = dht2.readTemperature();
}

void power_meter() {
  voltage0 = pzem1.voltage();
  voltage1 = pzem2.voltage();

  current0 = pzem1.current();
  current0 = pzem2.current();

  pf0 = pzem1.pf();
  pf1 = pzem2.pf();

  freq0 = pzem1.frequency();
  freq1 = pzem2.frequency();

  energy0 = pzem1.energy();
  energy1 = pzem2.energy();
}
