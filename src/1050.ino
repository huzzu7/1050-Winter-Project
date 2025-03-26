#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_ENS160.h"
#include <Adafruit_AHTX0.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Fan control pin
#define FAN_PIN 10

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Initialize ENS160 air quality sensor
SparkFun_ENS160 myENS;
// Initialize AHTX0 temperature and humidity sensor
Adafruit_AHTX0 aht;

void initializeDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("Display not detected!");
        while (1); // Halt if display initialization fails
    }
    display.setRotation(0);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.display();
    delay(1000); // Small delay for initialization
}

void initializeSensors() {
    if (!myENS.begin() || !aht.begin()) {
        Serial.println("Sensor Error!"); // Print error if sensors are not detected
    }
    myENS.setOperatingMode(SFE_ENS160_RESET);
    delay(100);
    myENS.setOperatingMode(SFE_ENS160_STANDARD);
}

void readSensorData(int &TVOC, int &eCO2, int &AQI, sensors_event_t &humidity, sensors_event_t &temp) {
    TVOC = myENS.getTVOC();
    eCO2 = myENS.getECO2();
    AQI = myENS.getAQI();
    aht.getEvent(&humidity, &temp);
}

int determineFanSpeed(int eCO2, int TVOC) {
    if (eCO2 > 1200 || TVOC > 500) {
        return 255; // Maximum speed if air quality is poor
    } else if (eCO2 > 800 || TVOC > 300) {
        return 180; // Medium speed for moderate air quality
    } else {
        return 100; // Low speed for good air quality
    }
}

void printSensorData(int TVOC, int eCO2, float temperature, float humidity) {
    Serial.print("TVOC: "); Serial.println(TVOC);
    Serial.print("eCO2: "); Serial.println(eCO2);
    Serial.print("Temperature: "); Serial.println(temperature);
    Serial.print("Humidity: "); Serial.println(humidity);
    Serial.println("----------------------");
}

void updateDisplay(int TVOC, int AQI, int eCO2, float temperature, float humidity, int fanSpeed) {
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
    display.print(temperature, 1); display.print("C  ");
    display.print(humidity, 1); display.print("%  ");
    
    int displayFanSpeed = (fanSpeed * 100) / 255;
    display.print(displayFanSpeed); display.print("%");
    
    display.display();
}

void setup() {
    Serial.begin(9600); // Start serial communication
    Wire.begin(); // Initialize I2C communication
    pinMode(FAN_PIN, OUTPUT); // Set fan pin as output
    
    initializeDisplay();
    initializeSensors();
}

void loop() {
    if (myENS.checkDataStatus()) {
        int TVOC, eCO2, AQI;
        sensors_event_t humidity, temp;
        readSensorData(TVOC, eCO2, AQI, humidity, temp);
        
        printSensorData(TVOC, eCO2, temp.temperature, humidity.relative_humidity);
        
        int fanSpeed = determineFanSpeed(eCO2, TVOC);
        analogWrite(FAN_PIN, fanSpeed);
        
        updateDisplay(TVOC, AQI, eCO2, temp.temperature, humidity.relative_humidity, fanSpeed);
    }
    delay(200); // Short delay before next reading
}

