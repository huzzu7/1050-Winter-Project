
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_ENS160.h"
#include <Adafruit_AHTX0.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define FAN_PIN 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SparkFun_ENS160 myENS;
Adafruit_AHTX0 aht;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    pinMode(FAN_PIN, OUTPUT);

    // Initialize Display
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("Display not detected!");
        while (1);
    }
    display.setRotation(0);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.display();
    delay(1000);

    if (!myENS.begin() || !aht.begin()) {
        Serial.println("Sensor Error!");
    }

    myENS.setOperatingMode(SFE_ENS160_RESET);
    delay(100);
    myENS.setOperatingMode(SFE_ENS160_STANDARD);
}

void loop() {
    if (myENS.checkDataStatus()) {
      int TVOC = myENS.getTVOC();
      int eCO2 = myENS.getECO2();
      int AQI = myENS.getAQI();
      sensors_event_t humidity, temp;
      aht.getEvent(&humidity, &temp);

      Serial.print("TVOC: "); Serial.println(TVOC);
      Serial.print("eCO2: "); Serial.println(eCO2);
      Serial.print("Temperature: "); Serial.println(temp.temperature);
      Serial.print("Humidity: "); Serial.println(humidity.relative_humidity);
      Serial.println("----------------------");

      //Maybe make the logic continuous than discrete?
      int fanSpeed = 0;
      if (eCO2 > 1200 || TVOC > 500) {
          fanSpeed = 255; 
      } else if (eCO2 > 800 || TVOC > 300) {
          fanSpeed = 180;
      } else {
          fanSpeed = 100;
      }

      analogWrite(FAN_PIN, fanSpeed);

      display.clearDisplay();

      display.setCursor(0, 0);
      display.print("TVOC   AQI  eCO2");

      display.setCursor(0, 10);
      display.print(TVOC); display.print("ppb  ");
      display.print(AQI); display.print("    ");
      display.print(eCO2); display.print("ppm");

      display.setCursor(0, 30);
      display.print("Temp   RH     Fan");

      display.setCursor(0, 40);
      display.print(temp.temperature, 1); display.print("C  ");
      display.print(humidity.relative_humidity, 1); display.print("%  ");
      int displayFanSpeed = (fanSpeed * 100) / 255;

      display.print(displayFanSpeed);display.print("%");

      display.display();

    }
    delay(200);  
}

