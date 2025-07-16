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
//#include <FreeRTOS.h>
#include <Wire.h>
#include <SPI.h>



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

volatile bool sense_check;
float pull_sense;

int eeprom_val;

int potenvalue;
volatile int output_sensor0;

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
  digitalWrite(pin_number, bitRead(EEPROM.read(9), bin_locate));  //0 = ON, 1 = OFF
}


void setup() {
  Serial.begin(115200);
                            
  if (!EEPROM.begin(512)) {     // Allocate 512 bytes of EEPROM
    Serial.println("Failed to init EEPROM");
    return;
  }

  //EEPROM.write(0, int(170));
  //EEPROM.commit();

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
  //input_bin_check(0);
  /*
  sense_check = bitRead(EEPROM.read(0), 1);

  if (sense_check == 1) {
    pull_sense = analogRead(out4);
  }
  else if (sense_check == 0) {
    pull_sense = !digitalRead(out4);
  }
  
  */

  Serial.print(EEPROM.read(0), BIN);
  Serial.print("  ");

  volatile int adc_val0 = analog_to_digital(0, adc0);  
  volatile int adc_val1 = analog_to_digital(0, adc1);  
  volatile int adc_val2 = analog_to_digital(0, adc2);  
  volatile int adc_val3 = analog_to_digital(0, adc3);  
  volatile int adc_val4 = analog_to_digital(0, adc4);  
  volatile int adc_val5 = analog_to_digital(0, adc5);  

  volatile bool digi_val0 = !digitalRead(digi_pin0);
  volatile bool digi_val1 = !digitalRead(digi_pin1);
  volatile bool digi_val2 = !digitalRead(digi_pin2);

  trigger_output(out0, 0);
  trigger_output(out1, 1);
  trigger_output(out2, 2);
  trigger_output(out3, 3);
  trigger_output(out5, 4);


  //Serial.println(sensorValue);
  
}



