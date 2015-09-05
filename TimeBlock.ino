#include "Wire.h"
#include <DHT.h>
#include <TM1637Display.h>

#define DS3231_I2C_ADDRESS 0x68
#define DHTTYPE DHT22

  float humidity;
  float temperature;
const int PinCLK 	= 2;
const int PinDIO 	= 3;
const int PinDHT 	= 4;
const int delayTime     = 2000;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
byte segto;
byte Digit0, Digit1, Digit2, Digit3;

TM1637Display display(PinCLK, PinDIO);
DHT dht(PinDHT, DHTTYPE);

int DisplayScroll;

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}


void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year) {
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

  void displaysegments() {
	// Set Brightness
    display.setBrightness(0x0a);

    // XGFEDCBA
    //0b00111111,    // 0
    
    //      A
    //     ---
    //  F |   | B
    //     -G-
    //  E |   | C
    //     ---
    //      D

    switch (DisplayScroll) {
      case 0:    
        // Get Time
        Digit0 = floor(hour / 10);
        Digit1 = floor(hour % 10);
        Digit2 = floor(minute / 10);
        Digit3 = floor(minute % 10);
        break;
      case 1:
        // Get Temperature
        Digit0 = floor(int(temperature) / 10);
        Digit1 = floor(int(temperature) % 10);
        Digit2 = 0b01100011; //degrees
        Digit3 = 0b00111001; // C
        break;
      case 2:
        // Get Humidity
        Digit0 = floor(int(humidity) / 10);
        Digit1 = floor(int(humidity) % 10);
        Digit2 = 0b01100011; //percentaje
        Digit3 = 0b01011100; // o
        break;
      default:
        Digit0 = 0b01000000; // -
        Digit1 = 0b01000000; // -
        Digit2 = 0b01000000; // -
        Digit3 = 0b01000000; // -
        break;
    }
    
    // Set digit 0 
    display.showNumberDec(Digit0, true, 1, 0);    

    if (DisplayScroll>0) {
      display.showNumberDec(Digit1, true, 1, 1);
      //Set Digit 2
      display.setSegments(&Digit2, 1, 2);
      //Set Digit 3
      display.setSegments(&Digit3, 1, 3);        
    }
    else {
      //Set digit 1
      segto = 0x80  | display.encodeDigit(Digit1);
      display.setSegments(&segto, 1, 1);
      //Set Digit 2
      display.showNumberDec(Digit2, true, 1, 2);
      //Set Digit 3
      display.showNumberDec(Digit3, true, 1, 3);
    }
    // Go to next case
    DisplayScroll++;
    if (DisplayScroll>2) {
      DisplayScroll=0;
    }
  }

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
  dht.begin();  
  // set the initial time here:
  //DS3231 seconds, minutes, hours, day, date, month, year
  setDS3231time(00,45,13,07,05,9,15);
  delay(500);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(delayTime);
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();


  
  displaysegments();

}

