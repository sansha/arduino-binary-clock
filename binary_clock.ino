//#include <Arduino.h>
#include <Wire.h>
#include "RTClib.h"

#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define SECONDS // comment if not using seconds bar (6 LEDs below the other strips)
#define TIME_ZONE_SWTICH

RTC_DS1307 RTC; // RTC connected via I2C (on atmega328 SDA: A4, SCL: A5)

// pins for WS2812 digital LEDs
int hours_pin = 10;
int minutes_pin = 9;
#ifdef SECONDS
  int seconds_pin = 7;
#endif

// WS2812 strips
Adafruit_NeoPixel hours = Adafruit_NeoPixel(8, hours_pin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel minutes = Adafruit_NeoPixel(8, minutes_pin, NEO_GRB + NEO_KHZ800);
#ifdef SECONDS
  Adafruit_NeoPixel seconds = Adafruit_NeoPixel(6, seconds_pin, NEO_GRB + NEO_KHZ800);
#endif


// pin for TIME_ZONE on / off switch
#ifdef TIME_ZONE_SWTICH
  int time_zone_pin = 2;
  int time_adjust_hour = 4;
  int time_adjust_minutes = 30;
#endif

// color array for base and "on" color
//                              base                      on
uint32_t colors_day[] = {minutes.Color(0,0,10), minutes.Color(100, 20, 0)};
uint32_t colors_night[] = {minutes.Color(0,0,0), minutes.Color(2, 1, 0)};

// array for the actual colors to be used (filled with either day or night by loop())
uint32_t colors[2];

// times (full hours) at which the day / night mode is activated
int day_begin = 7;
int night_begin = 23;


void setup() {
  Serial.begin(57600);

  // set up connection to RTC
  Wire.begin();
  RTC.begin();
  #ifdef TIME_ZONE_SWTICH
    // set necessary pins modes
    // the TIME_ZONE is input of course, but the PIN 3 (next to it) needs to be output
    // & high in order to be able to reliably detect the switch setting
    pinMode(time_zone_pin, INPUT);
    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);
    
  #endif

  // init NeoPixels
  hours.begin();
  hours.show(); // Initialize all pixels to 'off'
  minutes.begin();
  minutes.show(); // Initialize all pixels to 'off'
  #ifdef SECONDS
    seconds.begin();
    seconds.show(); // Initialize all pixels to 'off'
  #endif
}

boolean half_hour_shift_flag = false;

void loop() {
/*
  show_number(&minutes, 5);
  delay(1000);
  show_number(&minutes, 7);
  minutes.show();
  delay(1000);
  minutes.setPixelColor(1, 0, 127, 0);
  minutes.setPixelColor(2, 0, 0, 200);
  minutes.show();
  for(int i = 0; i < 60; i+=10) {
    show_seconds(i);
    delay(1000);
  }*/
  DateTime rtcTime = RTC.now();
  DateTime t = rtcTime;
  #ifdef TIME_ZONE_SWTICH
    if(digitalRead(time_zone_pin) == 1) 
    {
      DateTime timeZoneTime (rtcTime + TimeSpan(0,time_adjust_hour,time_adjust_minutes,0));
      t = timeZoneTime;
    }
  #endif
 


  Serial.print("Seconds: ");
  Serial.println(t.second());
  Serial.print("Minutes: ");
  Serial.println(t.minute());
  Serial.print("Hours: ");
  Serial.println(t.hour());


  // choose day / night mode
  int hrs = t.hour();
  if(hrs >= night_begin || hrs < day_begin) {
    Serial.println("night");
    memcpy(colors, colors_night, sizeof(colors_night));
  } else {
    Serial.println("day");
    memcpy(colors, colors_day, sizeof(colors_day));
  }

  // get hours and display them
  show_number(&hours, t.hour());

  // get minutes and display them
  show_number(&minutes, t.minute());
  #ifdef SECONDS
    show_seconds(t.second());
  #endif

  delay(500);
}

void show_number(Adafruit_NeoPixel* strip, int number) {

  // separate 10 and 1 digits
  uint8_t tens = number / 10; // get the 10-digit
  uint8_t ones = number % 10; // get the 1-digit
  //Serial.println(tens);
  //Serial.println(ones);
  //Serial.println();
  // show the ones
  for(int i = 0; i < 4; i++) {
    uint8_t bit = ones % 2;
    ones = ones >> 1;
    //Serial.print(bit);
    //Serial.print(" - ");
    strip->setPixelColor(i, colors[bit]);
    //Serial.println(colors[bit]);
  }
  // show the tens
  for(int i = 0; i < 4; i++) {
    uint8_t bit = tens % 2;
    tens = tens >> 1;
    //Serial.println(bit);
    strip->setPixelColor(i + 4, colors[bit]);
    //Serial.println(colors[bit]);
  }
  // apply
  strip->show();
  //Serial.println();
}

#ifdef SECONDS
  void show_seconds(int number) {
    // separate 10 and 1 digits
    number = (number + 1) / 10;
    for(int i = 0; i < 6; i++) {
      uint8_t bit = i < number;
      seconds.setPixelColor(5-i, colors[bit]);
    }
    seconds.show();
  }
#endif
