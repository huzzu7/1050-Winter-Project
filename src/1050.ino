#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_ENS160.h"
#include <Adafruit_AHTX0.h>

// OLED display settings
#define SCREEN_WIDTH 128  // Width of the OLED display in pixels
#define SCREEN_HEIGHT 64   // Height of the OLED display in pixels
#define OLED_RESET -1       // OLED reset pin (not used here)
#define SCREEN_ADDRESS 0x3C // I2C address for the OLED display

// Fan control pin
#define FAN_PIN 10  // Pin used to control the fan speed using PWM

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Initialize ENS160 air quality sensor
SparkFun_ENS160 myENS;
// Initialize AHTX0 temperature and humidity sensor
Adafruit_AHTX0 aht;

// Function to initialize and configure the OLED display
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

// Function to initialize the air quality and temperature sensors
void initializeSensors() {
    if (!myENS.begin() || !aht.begin()) {
        Serial.println("Sensor Error!"); // Print error if sensors are not detected
    }
    myENS.setOperatingMode(SFE_ENS160_RESET); // Reset air quality sensor
    delay(100);
    myENS.setOperatingMode(SFE_ENS160_STANDARD); // Set standard operating mode
}

// Function to read sensor data and update the provided variables
void readSensorData(int &TVOC, int &eCO2, int &AQI, sensors_event_t &humidity, sensors_event_t &temp) {
    TVOC = myENS.getTVOC(); // Get Total Volatile Organic Compounds level
    eCO2 = myENS.getECO2(); // Get estimated CO2 level
    AQI = myENS.getAQI();   // Get Air Quality Index
    aht.getEvent(&humidity, &temp); // Get temperature and humidity data
}

// Function to determine the fan speed based on air quality levels
int determineFanSpeed(int eCO2, int TVOC) {
    if (eCO2 > 1200 || TVOC > 500) {
        return 255; // Maximum speed if air quality is poor
    } else if (eCO2 > 800 || TVOC > 300) {
        return 180; // Medium speed for moderate air quality
    } else {
        return 100; // Low speed for good air quality
    }
}

// Function to print sensor data to the Serial Monitor for debugging
void printSensorData(int TVOC, int eCO2, float temperature, float humidity) {
    Serial.print("TVOC: "); Serial.println(TVOC);
    Serial.print("eCO2: "); Serial.println(eCO2);
    Serial.print("Temperature: "); Serial.println(temperature);
    Serial.print("Humidity: "); Serial.println(humidity);
    Serial.println("----------------------");
}

// Function to update the OLED display with sensor readings and fan speed
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
    
    int displayFanSpeed = (fanSpeed * 100) / 255; // Convert fan speed to percentage
    display.print(displayFanSpeed); display.print("%");
    
    display.display(); // Update the display
}

// Setup function that runs once at the start
void setup() {
    Serial.begin(9600); // Start serial communication
    Wire.begin(); // Initialize I2C communication
    pinMode(FAN_PIN, OUTPUT); // Set fan pin as output
    
    initializeDisplay(); // Initialize OLED display
    initializeSensors(); // Initialize air quality and temperature sensors
}

// Main loop function that runs continuously
void loop() {
    if (myENS.checkDataStatus()) { // Check if new data is available
        int TVOC, eCO2, AQI;
        sensors_event_t humidity, temp;
        readSensorData(TVOC, eCO2, AQI, humidity, temp);
        
        printSensorData(TVOC, eCO2, temp.temperature, humidity.relative_humidity); // Debugging output
        
        int fanSpeed = determineFanSpeed(eCO2, TVOC); // Get appropriate fan speed
        analogWrite(FAN_PIN, fanSpeed); // Set fan speed using PWM
        
        updateDisplay(TVOC, AQI, eCO2, temp.temperature, humidity.relative_humidity, fanSpeed); // Refresh display
    }
    delay(200); // Short delay before next reading
}
