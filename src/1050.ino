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

void setup() {
    Serial.begin(9600); // Start serial communication
    Wire.begin(); // Initialize I2C communication
    pinMode(FAN_PIN, OUTPUT); // Set fan pin as output

    // Initialize the OLED display
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

    // Initialize sensors
    if (!myENS.begin() || !aht.begin()) {
        Serial.println("Sensor Error!"); // Print error if sensors are not detected
    }

    // Reset and set ENS160 sensor to standard operating mode
    myENS.setOperatingMode(SFE_ENS160_RESET);
    delay(100);
    myENS.setOperatingMode(SFE_ENS160_STANDARD);
}

void loop() {
    // Check if ENS160 has new data available
    if (myENS.checkDataStatus()) {
        // Read air quality sensor data
        int TVOC = myENS.getTVOC(); // Total Volatile Organic Compounds (ppb)
        int eCO2 = myENS.getECO2(); // Equivalent CO2 (ppm)
        int AQI = myENS.getAQI(); // Air Quality Index

        // Read temperature and humidity from AHTX0 sensor
        sensors_event_t humidity, temp;
        aht.getEvent(&humidity, &temp);

        // Print sensor data to serial monitor
        Serial.print("TVOC: "); Serial.println(TVOC);
        Serial.print("eCO2: "); Serial.println(eCO2);
        Serial.print("Temperature: "); Serial.println(temp.temperature);
        Serial.print("Humidity: "); Serial.println(humidity.relative_humidity);
        Serial.println("----------------------");

        // Determine fan speed based on air quality conditions
        int fanSpeed = 0;
        if (eCO2 > 1200 || TVOC > 500) {
            fanSpeed = 255; // Maximum speed if air quality is poor
        } else if (eCO2 > 800 || TVOC > 300) {
            fanSpeed = 180; // Medium speed for moderate air quality
        } else {
            fanSpeed = 100; // Low speed for good air quality
        }

        // Adjust fan speed accordingly
        analogWrite(FAN_PIN, fanSpeed);

        // Update OLED display with sensor data
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
        
        // Calculate and display fan speed percentage
        int displayFanSpeed = (fanSpeed * 100) / 255;
        display.print(displayFanSpeed); display.print("%");

        display.display(); // Refresh OLED display
    }
    delay(200); // Short delay before next reading
}
