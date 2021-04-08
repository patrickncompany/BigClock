#include <Arduino.h>
#include <SPI.h>
#include <RTClib.h>

RTC_DS3231 RTC;

/***************** LCD Start ***********/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/***************** LCD End ***********/

void showTopTitle();
void refreshDisplay();
String currentTopTitle ="xx:xx:xx am";
uint8 cHour;
uint8 cMinute;
uint8 cSecond;
String ampm;

void setup() {
  Serial.begin(115200);

  //******************  LCD Setup Start ******************//
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //******************  LCD Setup END ******************//   

  /******************* RTC setup start***********************/
  RTC.begin();
  RTC.adjust(DateTime(__DATE__,__TIME__));
  //RTC.adjust(DateTime(2014,1,21,3,0,0));
  /******************* RTC setup end***********************/
}

void loop() {
  
  DateTime now = RTC.now();

  cHour=now.twelveHour();
  cMinute=now.minute();
  cSecond=now.second();

  String sHour=String(cHour);
  String sMinute=String(cMinute);
  String sSecond=String(cSecond);

  if (sHour.length()==1) sHour="0"+sHour;
  if (sMinute.length()==1) sMinute="0"+sHour;
  if (sSecond.length()==1) sSecond="0"+sHour;

  if (now.isPM()==1){ampm="P";} else {ampm="A"; }

  String sTime=sHour + ":" + sMinute + ":" + sSecond + " " + ampm;
  Serial.println(sTime);
  currentTopTitle=sTime;
  refreshDisplay();

 delay(1000);

}

void showTopTitle(){
  // top line of pixels (yellow)
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(currentTopTitle);
}

void refreshDisplay(){
  display.clearDisplay();
  showTopTitle();
  display.display();   
}