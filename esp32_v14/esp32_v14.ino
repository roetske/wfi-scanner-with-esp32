/****************************************************************
 * 
 * esp32
 * Modes: AP list
 *        Single AP scan
 * getting started credits to onetransistor especially graphics     
 * https://www.onetransistor.eu/2020/04/wifi-analyzer-esp8266-ili9341-lcd.html
 * version from onetransistor was for esp8266 mine esp32
 remarks
-----------

esp32 38 pins wroom32//use antenna on chip
or if prefer
esp with apex antenna connector esp32ue
external antenna (sticky version :))
TZT 2.4Ghz 4dBi WIFI Internal Antenna PCB Built-in Aerial IPX IPEX Airgain Omni Antenna UFL Connector
tft gmt 020-02 240x320
2push buttons
jst 3 connector pitch 2.54
jst 8 connector pitch 2.54
breadboard print (made soldering way easier)

connecting tft is spi but naming on tft confused me
no sda scl is normal i2c but no it is spi no miso
GND is connected to pin GND of the NodeMCU board,
i had an old esp8266 but could not get it to work got stuck and restarting 
continuously

vcc 3.3v
gnd gnd
tft_sclk 18 
tft_mosi 23 sda 
tft_rst 4 
tft_dc 2 
tft cs 15 

buttons g32 g35

lib used tft_espi 
tft_espi setup is in library you have to set correctly
tft_espi annoying tft touch button not defined extra line in user-SETUP-SELECT disable warning in setup_select
setup user
#define ST7789_DRIVER
#define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
#define TFT_HEIGHT 320 // ST7789 240 x 320
  specific esp32
  //#define TFT_MISO 19
  #define TFT_MOSI 23
  #define TFT_SCLK 18
  #define TFT_CS   15  // Chip select control pin
  #define TFT_DC    2  // Data Command control pin
  #define TFT_RST   4  // Reset pin (could connect to RST pin)
//#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

freq i choose this
#define SPI_FREQUENCY  27000000

in user setup select (annoying alarm touchup)
#define DISABLE_ALL_LIBRARY_WARNINGS

buttons antibounce necessary :)
scanning wifi is async takes normal scan takes too long 
async when results refresh -store data scan in mem 

buttons what a mess had to rethink how to use
g35 to single
g32 back to full and in full jump select ok

assembled loss signal is significant
compared with phone data with results esp 32 antenna  big difference 
next test esp with external antenna
jeezes standard antenna on esp32 not good (can also be related pos in box but still)
test with esp32 with external antenna 20db difference (allows you to pos your esp where you want and not worry about reception antenna)
 ****************************************************************/

#include <WiFi.h>
#include <WiFiScan.h>
#include  <esp_wifi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <elapsedMillis.h>
//var
#define TEXT_FONT 1
#define MAX_AP_COUNT 64
#define G32 32
#define G35 35

String version ="Version 14";
elapsedMillis emWifiscan;
elapsedMillis emrefreshtft;
long intrefreshtft = 2000; //ms
elapsedMillis embutton;
long intbutton = 200; //ms

//data Wifi
int nNet = 0; // number of APs found
int APfound = 99; // must be in dataloaded nNet changes when scandelete
struct idx_rssi_t {
  int idx;
  String bssidstr;
  String ssid;
  int32_t rssi;
  int channel;
  byte encrypt;
} idx_rssi[MAX_AP_COUNT];
bool startnewscan = true; // immediately start scan when esp starts
bool scanbusy = false;
bool scandone = false;
bool newscandataready = false;
bool dataloaded = false;
//tft
int selectedAP = 0;     // selected AP in list screen
uint8_t selectedChannel=0;
String selectedAUTH ="";
String selectedSSID = "";
String selectedBSSID="";
int selectedRSSI = 0;

bool showfreshscan = false;//when scanning first time or no AP found
bool singleNetworkDisplay = false;
int yoff = 16;
static const uint8_t reload_bitmap[] PROGMEM  = { 0x00, 0x00, 0x00, 0x10, 0x00, 0x18, 0x1f, 0xfc, 0x3f, 0xfe, 0x7f, 0xfc, 0x60, 0x18, 0x00, 0x10, 0x08, 0x00, 0x18, 0x06, 0x3f, 0xfe, 0x7f, 0xfc, 0x3f, 0xf8, 0x18, 0x00, 0x08, 0x00, 0x00, 0x00 };


//scherm definieren
TFT_eSPI tft = TFT_eSPI();


//*******
//setup
//*******
void setup() 
{
  Serial.begin(115200);
  pinMode(G32, INPUT);
  pinMode(G35,INPUT);
  
  
  //wifi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
  
  //tft init
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("WiFi Analyzer", 120, 172, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(version, 120, 192, TEXT_FONT);
  tft.setTextDatum(L_BASELINE);
  delay(1500);
}
//***************
//mainloop
//**************
void loop()
 { 
  //scannetwork when triggered and load data
   v_scannetwork();

  //no AP found rescan
  if ( APfound == 99 )
        { Serial.println("Nothing found rescan!!!!!!");
          selectedAP = 0;     
          selectedChannel=0;
          selectedAUTH = "";
          selectedSSID = "";
          selectedBSSID= "";
          selectedRSSI = 0;
          APfound = 0; //trigger once
          tft_espi_scanning();
          showfreshscan = true;
        }

  //show list firstscan
  if (showfreshscan && dataloaded)
    { //change from scanning to result show list display
           Serial.println("Showrefreshscan");
           tft.fillScreen(TFT_BLACK);
           networksListScreen();
           displaySelectionMarker(selectedAP);
           showfreshscan = false;
    }
   
  
  //rescan when finished
   if (!scanbusy && newscandataready && dataloaded)
     {  if (emrefreshtft > intrefreshtft)
        {//refresh screens
         if (!startnewscan )
           {//refresh when new data refreshes too fast does not allow to select correct ssid with pointer
              refreshtft();//single refresh immediately 
              startnewscan = true;
              emrefreshtft = 0;
           }
        } 
     }
   else {emrefreshtft =0;} //set also zero when scan is finished 
  
  //buttons display en select network
  if (embutton > intbutton)
   { buttonPressedG32();
     buttonPressedG35();
     embutton = 0;
   }
}
//******************************************************************
//anti bounce G35 display switch single or list
//            G32 go along list when full list -in single view when pressed go back to full list
void buttonPressedG35() //display to go from full list to selected AP
{
  static bool lastButtonState = HIGH; // start with button released
  bool currentButtonState = digitalRead(G35);
  
  if (currentButtonState == LOW && lastButtonState == HIGH)
   { 
         
    if (!singleNetworkDisplay)
       {singleNetworkDisplay = true;
        refreshtft();
      }
   }
  
  lastButtonState = currentButtonState;
}

void buttonPressedG32() //select nextwork -when pressed in single go back to full list
{
  static bool lastButtonState = HIGH; // start with button released
  bool currentButtonState = digitalRead(G32);
 
  if (currentButtonState == LOW && lastButtonState == HIGH && !singleNetworkDisplay)
   {//select next network in full list
    if (dataloaded) {selectNextNetwork();}
   }
   if (currentButtonState == LOW && lastButtonState == HIGH && singleNetworkDisplay)
    { //go back to full list
      singleNetworkDisplay= false;
      refreshtft();
    }
  lastButtonState = currentButtonState;
}






