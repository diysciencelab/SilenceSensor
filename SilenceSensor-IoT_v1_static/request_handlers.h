// ***************************************************************************
// Request handlers
// ***************************************************************************


// ***************************************************************************
// HTTP request handlers
// ***************************************************************************



// ***************************************************************************
// MQTT callback / connection handler
// ***************************************************************************


// ***************************************************************************
// Button management
// ***************************************************************************
#ifdef ENABLE_BUTTON
  void shortKeyPress() {
    Serial.print(F("Short button press\n"));
   
  }
  

  // called when button is kept pressed for less than 2 seconds
  void mediumKeyPress() {
    Serial.print(F("Medium button press\n"));
    //entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
    Serial.println(F("Starting Config Portal..."));
    WiFiManager wifiManager;
    //set timeout for config portal (e.g. reboot after 3 minutes after power loss to retry connect to configured Wifi)
    wifiManager.setConfigPortalTimeout(180);
    //wifiManager.resetSettings();
    if (!wifiManager.startConfigPortal(HOSTNAME,WIFI_PASSWORD)) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
      
      //all done, restarting...
      delay(3000);
      ESP.reset();
      delay(5000);
    
  }

  // called when button is kept pressed for 2 seconds or more
  void longKeyPress() {
    Serial.print(F("Long button press\n"));
    //entered config mode, make led toggle faster
    ticker.attach(0.1, tick);
    
    Serial.println(F("ERASING SPIFS and WiFiManager settings in 5 seconds..."));
    delay(5000);
    SPIFFS.format();
      // WiFiManager, local intialization
    WiFiManager wifiManager;
    wifiManager.resetSettings();
  
    //set timeout for config portal (e.g. reboot after 3 minutes after power loss to retry connect to configured Wifi)
    wifiManager.setConfigPortalTimeout(180);
  
    if (!wifiManager.startConfigPortal(HOSTNAME,WIFI_PASSWORD)) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //all done, restarting...
      delay(3000);
      ESP.reset();
      delay(5000);
  }

  void button() {
    if (millis() - keyPrevMillis >= keySampleIntervalMs) {
      keyPrevMillis = millis();

      byte currKeyState = digitalRead(BUTTON);

      if ((prevKeyState == HIGH) && (currKeyState == LOW)) {
        // key goes from not pressed to pressed
        KeyPressCount = 0;
      }
      else if ((prevKeyState == LOW) && (currKeyState == HIGH)) {
        if (KeyPressCount < longKeyPressCountMax && KeyPressCount >= mediumKeyPressCountMin) {
          Serial.print(F("keyCount: "));
          Serial.println(KeyPressCount);
          mediumKeyPress();
        }
        else {
          if (KeyPressCount < mediumKeyPressCountMin) {
            Serial.print(F("keyCount: "));
            Serial.println(KeyPressCount);
            shortKeyPress();
          }
        }
      }
      else if (currKeyState == LOW) {
        KeyPressCount++;
        if (KeyPressCount >= longKeyPressCountMax) {
          Serial.print(F("keyCount: "));
          Serial.println(KeyPressCount);
          longKeyPress();
        }
      }
      prevKeyState = currKeyState;
    }
  }
#endif


