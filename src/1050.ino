#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_ENS160.h"
#include <Adafruit_AHTX0.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define FAN_PIN 3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SparkFun_ENS160 myENS;
Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(ForwardPin, OUTPUT); 

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Display not detected!");
    while (1);
  }
  display.setRotation(1); 
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
    int AQI = myENS.getAQI();
    int TVOC = myENS.getTVOC();
    int eCO2 = myENS.getECO2();
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    display.clearDisplay();
    
    display.setCursor(0, 0);
    display.print("AQI:  "); display.println(AQI);
    display.setCursor(0, 16);
    display.print("TVOC: "); display.print(TVOC); display.println("ppb");
    display.setCursor(0, 32);
    display.print("eCO2: "); display.print(eCO2); display.println("ppm");

    display.setCursor(0, 56);
    display.print("Temp: "); display.print(temp.temperature, 1); display.println("C");
    display.setCursor(0, 72);
    display.print("RH:   "); display.print(humidity.relative_humidity, 1); display.println("%");
    
    display.display();
  }
  delay(2000);
}

void displayError(const char *message) {
  display.clearDisplay();
  display.setCursor(10, 28);
  display.println(message);
  display.display();
  while (1);
}

void enableFan(void){
  // if(condition){
  //   digitalWrite(FAN_PIN, HIGH);
  // }
}