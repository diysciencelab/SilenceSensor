/****************************************
 * Environment noise sampling
 * Base sampling analog MIC in 50ms (20Hz)
 * Calculating min., max and average values for batches
 * bach lenght from 5sec ... 30 sec
 * 
 * Optimized for ESP8266 / Wemos D1 mini
 * by Jaani 2018
 * 
 * based on Adafruit Microfon Amplifier sample
 * 
 */
#include "config.h"

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <FS.h>

#define ENABLE_BUTTON
#define WIFI_MANAGER

//PIN definitions
#define BUILTIN_LED 2 // On-board blue LED
#define BUTTON 0      //GPIO0 as input
#define MIC 0         //Analog0 input

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
const int batchLenght=10000;

unsigned int sample;
const int numSamples = batchLenght/sampleWindow;;

unsigned long startBatchMillis=millis();


int readings[numSamples];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

unsigned int batchMaxPP=0;
unsigned int batchMinPP=1024;
unsigned int batchMaxdB=0;
unsigned int batchMindB=1024;



// ***************************************************************************
// Load library "ticker" for blinking status led
// ***************************************************************************
#include <Ticker.h>
Ticker ticker;

void tick(){
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

// ***************************************************************************
// Button handling
// ***************************************************************************
#ifdef ENABLE_BUTTON
  unsigned long keyPrevMillis = 0;
  const unsigned long keySampleIntervalMs = 25;
  byte longKeyPressCountMax = 100;       // 200 * 25 = 5000 ms
  byte mediumKeyPressCountMin = 20;     // 20 * 25 = 500 ms
  //byte mediumKeyPressCountMin = 80;     // 80 * 25 = 2000 ms
  byte KeyPressCount = 0;
  byte prevKeyState = HIGH;             // button is active low
  boolean buttonState = false;
  boolean stateConnected = false;
#endif

// ****************************************************************************
// WiFiManager
// ***************************************************************************
/*
  WiFiManager
  Doc: https://github.com/tzapu/WiFiManager
  Moves the ESP8266 into AP mode and spins up a DNS and WebServer, IP 192.168.4.1
  Scans the available AP, asks the password for the wished AP and tries to connect to it
  Custom parameters:
    - Adafruit.io username
    - Adafruit.io KEY
  How-to:
    - Define #WIFI_MANAGER, or
    - Set manually the variables
*/

  
  // Wifi AP: SSID: chip ID, passowrd: WIFI_PASSWORD
  const char              WIFI_PASSWORD[]           = "<ConfigAP_WPA2KEY>";		//add your WPA2 key here for config AP

  const char                    IO_USERNAME[]           = "<IO-Username_comes_here";	//Add your Adafruit.io username here
  const char                    IO_KEY[]                = "<IO-Key_comes_here";			//Add your Adafruit.IO API key here
  
  char                    HOSTNAME[30]         = {0};
  // flag for saving data
  bool                    m_shouldSaveConfig        = false;
  
  void saveConfigCallback() {
    Serial.println(F("INFO: Should save config"));
    m_shouldSaveConfig = true;
  }
  
  //gets called when WiFiManager enters configuration mode
  void configModeCallback(WiFiManager *myWiFiManager){
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    //if you used auto generated SSID, print it
    Serial.println(myWiFiManager->getConfigPortalSSID());
    //entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
  
  }
  
// ***************************************************************************
// Include: Request handlers
// ***************************************************************************
#include "request_handlers.h"



#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, HOSTNAME, WIFI_PASSWORD);

AdafruitIO_Feed *MinDB = io.feed("mindb");
AdafruitIO_Feed *MaxDB = io.feed("maxdb");
AdafruitIO_Feed *AvgDB = io.feed("avgdb");



 
void setup() 
{
  Serial.begin(115200);

  
  // button pin setup
  #ifdef ENABLE_BUTTON
    pinMode(BUTTON,INPUT_PULLUP);
  #endif
  
  // init the built-in led
  pinMode(BUILTIN_LED, OUTPUT);

  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.5, tick);

//#ifdef WIFI_MANAGER
  // clean FS, for testing
  //SPIFFS.format();

  sprintf(HOSTNAME, "Makerspace.hu_Silence-%06X", ESP.getChipId());
  



 // WiFiManager, local intialization
  WiFiManager wifiManager;

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  
  // set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set timeout for config portal (e.g. reboot after 3 minutes after power loss to retry connect to configured Wifi)
  wifiManager.setConfigPortalTimeout(180);



  
  // reset settings, for testing
 // wifiManager.resetSettings();

  // disable the debug output
  wifiManager.setDebugOutput(false);

  // fetches ssid and pass and tries to connect
  // if it does not connect it starts an access point with the specified name
  if (!wifiManager.autoConnect(HOSTNAME,WIFI_PASSWORD)) {
    Serial.println(F("Error: Failed to connect and hit timeout"));
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  // if you get here you have connected to the WiFi
  Serial.println(F("INFO: Successfully connected to the Wifi AP"));



  Serial.print(F("INFO: Adafruit.IO username: "));
  Serial.println(IO_USERNAME);

  Serial.print(F("INFO: Adafruit.IO key: "));
  Serial.println(IO_KEY);


  Serial.print(F("INFO: Local IP address: "));
  Serial.println(WiFi.localIP());

  delay(500);
  // start ticker with 1 because we have Wifi, but still try connecting to Adafruit.io
  ticker.attach(1, tick);


 
  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  int cnt=0;
  while(io.status() < AIO_CONNECTED) {
    stateConnected=false;
    button();
    Serial.print(".");
    delay(500);
    cnt++;
    if(cnt>=60){
      Serial.println(F(""));
      Serial.println(F("Cannot connect to Adafruit.IO for more than 30 seconds, restarting."));
      ESP.reset();
      delay(5000);
    }
  }
 
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);
  stateConnected=true;
  
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
   
   for (int thisReading = 0; thisReading < numSamples; thisReading++) {
    readings[thisReading] = 0;
   }
}




void loop() 
{
  #ifdef ENABLE_BUTTON
    button();
  #endif
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
 
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      
      sample = analogRead(MIC);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
    
    
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
     
    if(peakToPeak>batchMaxPP){
        batchMaxPP=peakToPeak;
    }
    else if(peakToPeak<batchMinPP){
        batchMinPP=peakToPeak;
    }

    
    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = peakToPeak;
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;

    if (readIndex >= numSamples) {
      // calculate the average:
      average = total / numSamples;
      
      // send it to the computer as ASCII digits
      Serial.print(F("Min: "));
      Serial.print(batchMinPP);
      Serial.print(F(" | Max: "));
      Serial.print(batchMaxPP);
      Serial.print(F(" | Average: "));
      Serial.println(average);  

      MinDB->save(batchMinPP);
      MaxDB->save(batchMaxPP);
      AvgDB->save(average);

      
      
      batchMaxPP=0;
      batchMinPP=1024;
      readIndex = 0;
    }

 


   io.run();
}

