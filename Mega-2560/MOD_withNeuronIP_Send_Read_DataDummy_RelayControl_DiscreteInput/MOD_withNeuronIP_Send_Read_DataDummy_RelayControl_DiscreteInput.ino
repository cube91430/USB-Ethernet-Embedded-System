//This is a Dummy_Data from the Arduino Mega

#include <Ethernet.h>
#include <ArduinoModbus.h>
#include <DHT.h>
#include <SPI.h>


#define DHTTYPE DHT22
DHT dht1(2, DHTTYPE);

byte mac[] = { 0xBC, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // match your MAC address
IPAddress ip(192, 168, 10, 177);                       // match your Arduino IP
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
int temperature, humidity;

const int relay0 = 44;
const int relay1 = 45;
const int relay2 = 46;
const int relay3 = 47;

const int relay4;
const int relay5;
const int relay6;
const int relay7;

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
  temperature = 25.0;
  humidity = 45.0;

  Serial.println("🔁 Simulation Reset. Starting New Cycle...");
}

void simulateNormalBehavior() {
  // Electrical fluctuations
  voltage += random(-10, 11) * 0.1;      // ±1V
  current += random(-20, 21) * 1;     // ±0.2A
  powerFactor += random(-5, 6) * 0.001;  // ±0.005
  frequency += random(-2, 3) * 0.01;     // ±0.02Hz

  // Environmental fluctuations
  temperature += random(-5, 6) * 0.1;  // ±0.5°C
  humidity += random(-3, 4) * 0.5;     // ±1.5%

  // Clamp electrical values
  voltage = constrain(voltage, 210, 240);
  current = constrain(current, 0, 10);
  powerFactor = constrain(powerFactor, 0.7, 1.0);
  frequency = constrain(frequency, 49.8, 50.2);

  // Clamp environmental values
  //temperature = constrain(temperature, 20, 45);
  //humidity = constrain(humidity, 30, 80);
}

void updateHoldingRegister() {
  modbusTCP.holdingRegisterWrite(0, temperature);     //1!400001
  modbusTCP.holdingRegisterWrite(3, humidity);        //1!400004
  modbusTCP.holdingRegisterWrite(10, voltage);        //1!400011
  modbusTCP.holdingRegisterWrite(11, current);        //1!400012
  modbusTCP.holdingRegisterWrite(12, power);          //1!400013
  modbusTCP.holdingRegisterWrite(13, powerFactor);    //1!400014
  modbusTCP.holdingRegisterWrite(14, frequency);      //1!400015
  modbusTCP.holdingRegisterWrite(15, energy_kWh);     //1!400016
}

void updateCoil() {
  digitalWrite(relay0, modbusTCP.coilRead(0));        //1!0001
  digitalWrite(relay1, modbusTCP.coilRead(1));        //1!0002
  digitalWrite(relay2, modbusTCP.coilRead(2));        //1!0003
  digitalWrite(relay3, modbusTCP.coilRead(3));        //1!0004
}

void DiscreteInput_Write(int coil, int arr){
  if (modbusTCP.coilRead(coil) == 1) {
    modbusTCP.discreteInputWrite(arr, 1);
  } else {
    modbusTCP.discreteInputWrite(arr, 0);
  }
}

void WriteDiscreteInput() {
  DiscreteInput_Write(0, 0);
  DiscreteInput_Write(1, 1);
  DiscreteInput_Write(2, 2);
  DiscreteInput_Write(3, 3);

}

void setup() {
  Serial.begin(9600);
  dht1.begin();

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

  pinMode(relay0, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);

  delay(2000);    //Delay from start

  startTime = millis();
}

void loop() {
  temperature = dht1.readTemperature();
  humidity = dht1.readHumidity();

  simulateNormalBehavior();

  power = voltage * current * powerFactor;
  energy_kWh += (power / 1000.0) * (1.0 / 3600.0);  // kWh increment per second

  EthernetClient client = server.available();

  if (client) {
    modbusTCP.accept(client);

    Serial.println("Neuron Connected!");

    if (!modbusTCP.holdingRegisterWrite(4, 10) == 1 && !modbusTCP.coilRead(1) == 1) {
      //1!400001
      Serial.print("Writing Failed");
    }
    Serial.println("Writing Data");

    updateHoldingRegister();      //Update Holding Registers
  
    updateCoil();                 //Update Coils

    WriteDiscreteInput();         //Update DiscreteInputs

    modbusTCP.accept(client);

    modbusTCP.poll();
  }
  //delay(100);
}