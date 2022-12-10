// Libraries
#include <LiquidCrystal.h>
#include <DHT.h>
#include <Stepper.h> 
#include "uRTCLib.h" 
// Temperature and Humidity Sensor
#define DHTPIN 7
#define DHTTYPE DHT11
// LCD set up
LiquidCrystal lcd(25, 23, 5, 4, 3, 2);
//DHT Sensor Setup 
DHT dht(DHTPIN, DHTTYPE);
//Stepper Motor Setup
Stepper stepper(250, 27,29,31,33);
//Setting up Clock 
uRTCLib rtc(0x68);
//Setting up registers 
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;
//Setting up the ports 
volatile unsigned char* port_b = (unsigned char*) 0x25; 
volatile unsigned char* ddr_b  = (unsigned char*) 0x24; 
volatile unsigned char* pin_b  = (unsigned char*) 0x23; 

volatile unsigned char* port_k = (unsigned char*) 0x108; 
volatile unsigned char* ddr_k  = (unsigned char*) 0x107; 
volatile unsigned char* pin_k  = (unsigned char*) 0x106;

//Defining values 
float tempThreshold = 75.00; 
int waterThreshold = 300;
int waterLevel;  

//Turn Fan on = *port_b |= 0b00000001;
//Red LED on = *port_b |= 0b10000000;
//Yellow LED on = *port_b |= 0b01000000;
//Green LED on = *port_b |= 0b00100000;
//Blue LED on = *port_b |= 0b00010000;
//Reset LEDs = *port_b &= 0b00001110;

//setup for the operation 
void setup() { 
  *ddr_b |= 0b11110001;
  *ddr_k &= 0b11111000;
  stepper.setSpeed(60);
  Serial.begin(9600);
  URTCLIB_WIRE.begin();
  rtc.set(0, 42, 7, 5, 13, 1, 22);
  }

//Loop for the whole operation 
void loop() {
  rtc.refresh();
  stepper.step(250);
  //Stop Button 
  if((*pin_k & 0b00000001) == 1){
    disableState();
  }
  //Start/Reset Button 
  if((*pin_k & 0b00000010) == 2){
    idleState();
  }  
}
//Function of Disable State
void disableState(){
  lcd.clear();   
  *port_b &= 0b00001110;
  *port_b |= 0b01000000;
  if((*pin_k & 0b00000010) == 2){
    Serial.print("Time Switched to Idle: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    idleState();
  }
  stepper.step(250);
} 
//Function of Idle State
void idleState(){
  *port_b &= 0b00001110;
  *port_b |= 0b00100000;
  waterLevel = analogRead(A7);
  //Using the temperature and humidty sensor to get and print temperature and humidity percentage 
  lcd.begin(16, 2);
  lcd.setCursor(7,1);
  dht.begin();
  
  float humidty = dht.readHumidity(true);
  float temperature = dht.readTemperature(true);
  bool running; 

  lcd.setCursor(0,0);
  lcd.print("Temp:  ");
  lcd.setCursor(7,0); 
  lcd.print(temperature);
  lcd.setCursor(12,0);
  lcd.print("F");
  lcd.setCursor(0,1);
  lcd.print("Humid: ");
  lcd.setCursor(7,1);
  lcd.print(humidty);
  lcd.setCursor(12,1);
  lcd.print("%");
  delay(50);
  //State to change to running state
  if(temperature > tempThreshold){
    Serial.print("Time Switched to Running: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    runningState();
  }
 //State to change to error state 
  if(waterLevel <= waterThreshold){
    Serial.print("Time Switched to Error: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    errorState();
  }
  //Stop Button to stop operation 
  if((*pin_k & 0b00000001) == 1){
    Serial.print("Time Switched to Disable: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    disableState(); 
  } 
}
//Function of Error State
void errorState(){
  //Lighting up LED and writing Error message on LCD
  lcd.begin(16, 2);
  *port_b &= 0b00001110;
  *port_b |= 0b10000000;
  lcd.setCursor(0,0);
  lcd.print("Water Level is");
  lcd.setCursor(4,1);
  lcd.print("too low");
  //Stop Button to go back to Disable State 
  if((*pin_k & 0b00000001) == 1){
    Serial.print("Time Switched to Disable: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    disableState(); 
  }
  //Reset Button to go back to Idle State
  if((*pin_k & 0b00000010) == 2){
    Serial.print("Time Switched to Idle: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    idleState(); 
  } 
}
////Function of Running State
void runningState(){
  //Lighting up Blue LED and Starting Fan 
  *port_b &= 0b00001110;
  *port_b |= 0b00010001;
  waterLevel = analogRead(A7);
  stepper.step(250);
  //Using the temperature and humidty sensor to get and print temperature and humidity percentage 
  lcd.begin(16, 2);
  dht.begin();
  
  float humidty = dht.readHumidity();
  float temperature = dht.readTemperature(true);

  lcd.setCursor(0,0);
  lcd.print("Temp:  ");
  lcd.setCursor(7,0); 
  lcd.print(temperature);
  lcd.setCursor(12,0);
  lcd.print("F");
  lcd.setCursor(0,1);
  lcd.print("Humid: ");
  lcd.setCursor(7,1);
  lcd.print(humidty);
  lcd.setCursor(12,1);
  lcd.print("%");
 //Going back to Idle State 
  if(temperature <= tempThreshold){
    Serial.print("Time Switched to Idle: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    idleState(); 
  }
 //Going to Error State 
  if(waterLevel < waterThreshold){
    Serial.print("Time Switched to Error: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    errorState(); 
  }
  //Stop Button to go back to Disable State 
  if((*pin_k & 0b00000001) == 1){
    Serial.print("Time Switched to Disable: ");
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.println(rtc.second());
    disableState(); 
  } 
}
