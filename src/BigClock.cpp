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

/*
 * Yellow Top
 * Yellow Pixels : 0 - 15 (16 pixels)
 * Note: 2 rows at size 1 with 1px padding not readable.
 *       1 row at size 2 fits perfect.
 * 
 * Blue Bottom
 * Blue Pixels : 16 - 63 (48 pixels)
 * Note: 
 * 
 * Text Size 1 : 8 pixels 7+1 (10 with padding due to small text size 1)
 * Text Size 2 : 16 pixels 15+1 (single pixels is enough at text size 2)
 * Text Size 3 : 32 pixels 31+1 
 * Text Size 4 : 64 pixels 63+1
 */

/***************** Rotary Control Start ***********/
#include <Button2.h>
#include <ESPRotary.h>
#define ROTARY_PIN1 14  // D5 = ESP12 GPIO14 // ESP32 D14 GPIO14
#define ROTARY_PIN2 12  // D6 = ESP12 GPIO12 // ESP32 D12 GPIO12
#define BUTTON_PIN  13  // D7 = ESP13 GPIO13 // ESP32 D13 GPIO13 
#define CLICKS_PER_STEP   4   // this number depends on your rotary encoder 
ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
Button2 b = Button2(BUTTON_PIN);
int curpos;
int minpos;
int maxpos;
void rotate(ESPRotary&);  // forward declares
void showDirection(ESPRotary&);  // forward declares
void click(Button2&);  // forward declares
// debounce mcu boot single click event. use as flag to ignore first click event on boot.
int firstRun=1;
/***************** Rotary Control End ***************/

/***************** Menu Start ***********/
const int numPages=4;  //must be a const to use as array index
const int numOptions=8; //must be a const to use as array index
String page[numPages]={"Main Menu","Run Menu","Wait Menu","Status"};
String menu[numPages][numOptions]={
  {"Run Time","Wait Time","Status","Quick Run","!! STOP !!","Restart","",""},
  {"3s","4s","5s","Back","","","",""},
  {"2h","+15m","-15m","Back","","","",""},
  {"Reset Wait","*****","*****","Back","","","",""}
  };

int maxPage = numPages-1; // max page INDEX
int minPage = 0;  // min page INDEX
int curPage = minPage; // current page INDEX
int prePage = 0;
int maxOption = numOptions-1; // max option INDEX
int minOption = 0;  // min option index
int curOption = minOption;  // current menu index
int preOption = 0;
int selOption = 0; // selected option INDEX (set on click)
String currentTopTitle = page[curPage]; // glogal string for top title
String currentMenuTitle = menu[curPage][curOption]; // glogal string for menu title
String currentMenuInfo = "wait time";
String selectedOption; // global string for clicked option
/***************** Menu End ***********/

/***************** RTC Start ***********/
uint8_t cHour;
uint8_t cMinute;
uint8_t cSecond;
String ampm;
/***************** RTC End ***********/

// display forward declares
void refreshDisplay();
void calibrateDisplay();
// rtc forward declares
String getTimeS();
// menu forward declares
void updatePage();  
void updateOption();  
void showSelect();  
void updateTitle(String);  
void goHome();
void goMenu(int);
void showTopTitle();  
void showMenuTitle();  
void showMenuInfo();

void setup() {
  Serial.begin(115200);

  //******************  LCD Setup Start ******************//
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // show buffer - splash image in lib folder.
  display.display();
  delay(2000); 
  //******************  LCD Setup END ******************//   

  /******************* RTC setup start***********************/
  RTC.begin();
  //RTC.adjust(DateTime(__DATE__,__TIME__));  // set time from pc at compile.
  RTC.adjust(DateTime(2025,5,5,5,5,55));  // set time manually.
  /******************* RTC setup end***********************/

  //******************  Rotary Setup Start  ******************//
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  b.setTapHandler(click);
  minpos=0;
  maxpos=numOptions-1;
  curpos=0;
  //******************  Rotary Setup End  ******************//   

  //******************  Menu Setup Stat  ******************//
  Serial.println(menu[curPage][curOption]);  
  //******************  Menu Setup End  ******************//

}

void loop() {

  currentTopTitle=getTimeS();
  refreshDisplay();
  delay(1000);

  // calibrateDisplay();

}

void goHome(){
  curPage=0; // new page
  curOption = minpos;  //option array index 
  curpos = minpos; //rotary position
  currentTopTitle = page[curPage];
  currentMenuTitle = menu[curPage][curOption];
  refreshDisplay();   
}

void goMenu(int cp){
  curPage=cp; // new page
  curOption = minpos;  //option array index 
  curpos = minpos; //rotary position
  currentTopTitle = page[curPage];
  currentMenuTitle = menu[curPage][curOption];
  refreshDisplay();   
}

void showTopTitle(){
  // top display (Yellow ?21 pixels)
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(currentTopTitle);
}

void showMenuTitle(){
  // first line after top yellow bar
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,20);
  display.println(currentMenuTitle);
}

void showMenuInfo(){
  // second line after top yello bar
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,40);
  display.println(currentMenuInfo);
}

// single click
void click(Button2& btn) { 
  selOption = curpos;
  selectedOption = menu[curPage][selOption];
  switch(curPage){
    // case for each page
    case 0:
      // Main Menu
      switch(selOption){
        // case for each option
        case 0:
          // First Menu
          if(firstRun){refreshDisplay();break;} //consume boot click event
          curPage=1; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 1:
        // 2nd Menu
          curPage=2; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 2:
        // 3rd Menu
          curPage=3; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 3:
        // 4th Menu
          curPage=4; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 4:
          goHome();
          break; 
        case 5:
          goHome();
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption);
      break;
    case 1:
      // Run Time
      switch(selOption){
        // case for each option
        case 0:
          goHome();
          break;
        case 1:
          goHome();
          break;
        case 2:
          goHome();
          break;
        case 3:
          // Go to Main Menu
          goHome();        
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption);   
      break;
    case 2:
      // Wait Time
      switch(selOption){
        // case for each option
        case 0:
          goHome();
          break;
        case 1:
          goHome();
          break;
        case 2:
          goHome();
          break;
        case 3:
          goHome();
          break;
      }
      Serial.print("Selected : ");Serial.println(selectedOption);   
      break;
    case 3:
      // Status
      switch(selOption){
        // case for each option
        case 0:
          goHome();
          break;
        case 1:
          goHome();
          break;
        case 2:
          goHome();
          break;
        case 3:
          goHome();
          break;
      }
      Serial.print("Selected : ");Serial.println(selectedOption); 
      break;
  }

}

// left/right rotation
void showDirection(ESPRotary& r) {
  Serial.println(r.directionToString(r.getDirection()));
  if (r.directionToString(r.getDirection())=="RIGHT"){
    curpos++;
  } else {
    curpos--;
  }
  
  // limit menu position with min/max; filter blanks

  // maxpos is always upper index of array
  // NOT 
  // index of last valid option in array.

  // IE: {"option1","option2","",""}
  // Above menu has 2 options(index 0,1). maxpos = max index = 3.
  // maxpos references invalid option. can NOT use.  

  // Full menu array {"option1","option2","option3","option4"}
  // Above menu has 4 options(index 0,1,2,3). maxpos = max index = 3.
  // maxpos references valid option. can use.

  if (curpos>maxpos){
    // >maxpos only true for full array
    curpos=maxpos;  // maxpos only valid for full array
  } else if (curpos<minpos){
    // All menus have index 0 (minpos)
    curpos=minpos;
  } else if (menu[curPage][curpos]==""){
    // empty option decrement pos
    curpos--;
  }

  curOption = curpos; //current option index
  currentTopTitle = page[curPage];
  currentMenuTitle = menu[curPage][curOption];

  refreshDisplay(); 
  
  if (firstRun){firstRun=0;} // consume first click event.
  Serial.print("Menu Item : ");Serial.println(curpos);
}

// all rotation
void rotate(ESPRotary& r) {
   Serial.println(r.getPosition());
   /*
    * with no call to r.getPosition() here
    * intermitent drop out of rotary polling
    * after click or change direction.
    * 
    */
}

void refreshDisplay(){
  display.clearDisplay();
  showTopTitle();
  showMenuTitle();
  showMenuInfo();
  display.display();   
}

String getTimeS(){
  DateTime now = RTC.now();
  cHour=now.twelveHour();
  cMinute=now.minute();
  cSecond=now.second();

  String sHour=String(cHour);
  String sMinute=String(cMinute);
  String sSecond=String(cSecond);

  if (sHour.length()==1) sHour="0" + sHour;
  if (sMinute.length()==1) sMinute="0" + sMinute;
  if (sSecond.length()==1) sSecond="0" + sSecond;

  if (now.isPM()==1){ampm="P";} else {ampm="A"; }

  String sTime=sHour + ":" + sMinute + ":" + sSecond + " " + ampm;
  Serial.print("Padded : ");
  Serial.println(sTime);

  return sTime;
}

void calibrateDisplay(){

  // sliding vertical row check
  // text size 2
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  for (int i=0; i<32;i++){
    display.clearDisplay();
    display.setCursor(0,i);
      Serial.println(i);
    display.println(i);
    display.display();     
    delay(250);
  }

  // fixed vertical row check - blue bottom
  // text size 1
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  for (int i=0; i<70;i=i+10){
    // loop to 63 for edge of screen - 70 to go past
    display.setCursor(0,i);
      Serial.println(i);
    display.println(i);
    display.display();     
  }
  delay(4000); 

  // fixed vertical row check
  // text size 2
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  for (int i=0; i<70;i=i+16){
    display.setCursor(0,i);
      Serial.println(i);
    display.println(i);
    display.display();     
  }
   delay(4000); 

  // fixed vertical row check
  // text size 3
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  for (int i=0; i<70;i=i+32){
    display.setCursor(0,i);
      Serial.println(i);
    display.println(i);
    display.display();     
  }
    delay(4000); 

 

  // multi row check


}