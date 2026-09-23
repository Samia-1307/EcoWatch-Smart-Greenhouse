#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define RX 2
#define TX 3
#define dht_apin 11  // Assuming A0 is the pin connected to the DHT sensor
#define soil_sensor_pin A1 // Pin connected to the soil moisture sensor
#define DHTTYPE DHT11  // Replace with your DHT sensor type (DHT11, DHT21, or DHT22)
#define FAN_RELAY_PIN A0 // Pin connected to the relay controlling the fan
#define LDR_PIN A3 // Pin connected to the LDR (Light Dependent Resistor)
#define MIN_TEMPERATURE 18 // Minimum temperature threshold (in degrees Celsius)
#define MAX_TEMPERATURE 24 // Maximum temperature threshold (in degrees Celsius)
#define MAX_HUMIDITY 80    // Maximum humidity threshold (in percentage)
#define LDR_THRESHOLD 30
#define OPTIMAL_MOISTURE_MIN 20
#define OPTIMAL_MOISTURE_MAX 60
const int soil_pump_RELAY_PIN = A2;  // the Arduino pin, which connects to the IN pin of relay
const int fire_pump_RELAY_PIN = 8;    // Pin connected to the relay controlling the fire pump
#define flamePin 12 // Pin connected to the flame sensor
#define buzzerPin 7 // Pin connected to the buzzer

DHT dhtObject(dht_apin, DHTTYPE);
String AP = "POCO M3";       // AP NAME
String PASS = "samia@1491"; // AP PASSWORD
String API = "LTY757W02W638C13";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
int value = 0;

SoftwareSerial esp8266(RX, TX); 

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on backlight
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");

  // Initialize the DHT sensor
  dhtObject.begin();

  // Set up the relay pins as outputs
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(soil_pump_RELAY_PIN, OUTPUT);
  pinMode(fire_pump_RELAY_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(flamePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(10, OUTPUT);
}

void loop() {
  float temperature = dhtObject.readTemperature();
  float humidity = dhtObject.readHumidity();
  float moisture_percentage = readSoilMoisture();

  // Read LDR value
  value = analogRead(LDR_PIN);

  // Check flame detection after DHT11 and soil sensor readings
  bool flameDetected = digitalRead(flamePin);

  // Display temperature, humidity, and soil moisture on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("Soil Moisture: ");
  lcd.print(moisture_percentage);
  lcd.print("%");
  lcd.setCursor(0, 3);

  // Print temperature and humidity readings to the serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

// Check flame detection and control fire pump
  flameSensorLoop();
  // Control the light based on the specified algorithm
  if (temperature < MIN_TEMPERATURE && value < LDR_THRESHOLD) {
    digitalWrite(10, HIGH); // Turn on the light
    Serial.println("Light turned on: Low temperature and no sunlight.");
  } else if (temperature > MAX_TEMPERATURE) {
    digitalWrite(10, LOW); // Turn off the light
    Serial.println("Light turned off: High temperature.");
  } else if (moisture_percentage > 65 && temperature >= MIN_TEMPERATURE && temperature <= MAX_TEMPERATURE) {
    digitalWrite(10, HIGH); // Turn on the light
    Serial.println("Light turned on: High soil moisture within optimal temperature range.");
  } else if (moisture_percentage >= OPTIMAL_MOISTURE_MIN && moisture_percentage <= OPTIMAL_MOISTURE_MAX) {
    digitalWrite(10, LOW); // Turn off the light
    Serial.println("Light turned off: Optimal soil moisture.");
  } else if (humidity > MAX_HUMIDITY) {
    digitalWrite(10, HIGH); // Turn on the light
    Serial.println("Light turned on: High humidity.");
  } else {
    digitalWrite(10, LOW); // Turn off the light
    Serial.println("Light turned off: No specific condition met.");
  }

// Check flame detection and control fire pump
  flameSensorLoop();
  // Control the soil pump based on the specified conditions
  if (moisture_percentage < 20) {
    Serial.println("Pump turned on: Soil moisture below 20%.");
    digitalWrite(soil_pump_RELAY_PIN, HIGH); // Turn on the pump
    // Keep the pump on until the soil moisture is between 50% and 60%
    while (moisture_percentage < 50) {
        moisture_percentage = readSoilMoisture(); // Update soil moisture reading
    }
    digitalWrite(soil_pump_RELAY_PIN, LOW); // Turn off the pump
    Serial.println("Pump turned off.");
} else if (moisture_percentage > 65) {
    // Turn on the fan until soil moisture reaches 60%
    Serial.println("Fan turned on: Soil moisture above 65%.");
    digitalWrite(FAN_RELAY_PIN, HIGH); // Turn on the fan
    digitalWrite(10, HIGH); // Turn on the light

    // Keep the fan and light on until soil moisture is below 60%
    while (moisture_percentage > 60) {
        moisture_percentage = readSoilMoisture(); // Update soil moisture reading
        temperature = dhtObject.readTemperature(); // Update temperature reading
        
        // Check if temperature reaches 24 degrees Celsius and turn off the light
        if (temperature >= 24) {
            digitalWrite(10, LOW); // Turn off the light
            Serial.println("Light turned off: Temperature reached 24 degrees Celsius.");
        }
    }

    digitalWrite(FAN_RELAY_PIN, LOW); // Turn off the fan
    Serial.println("Fan turned off: Soil moisture below 60%.");
} else if (moisture_percentage > 20 && moisture_percentage <60 ) {
    digitalWrite(soil_pump_RELAY_PIN, LOW);
    // Do nothing if soil moisture is between 20% and 60%
}

  // Check flame detection and control fire pump
  flameSensorLoop();
  
  // Update ThingSpeak with sensor data
  String getData = "GET /update?api_key=" + API + "&field3=" + String(temperature) + "&field2=" + String(humidity) + "&field1=" + String(moisture_percentage);
  sendCommand("AT+CIPMUX=1", 5, "OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
  sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 4, ">");
  esp8266.println(getData);
  delay(3000);
  countTrueCommand++;
  //sendCommand("AT+CIPCLOSE=0", 5, "OK");
}

float readSoilMoisture() {
  int sensor_analog = analogRead(soil_sensor_pin);
  float moisture_percentage = (100.0 - (sensor_analog / 10.23)); // Assuming soil sensor output is in the range 0-1023
  Serial.print("Soil Moisture Percentage: ");
  Serial.print(moisture_percentage);
  Serial.println("%");
  return moisture_percentage;
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1)) {
    esp8266.println(command);
    if (esp8266.find(readReplay)) {
      found = true;
      break;
    }
    countTimeCommand++;
  }

  if (found == true) {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false) {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}

// Flame sensor code
void flameSensorLoop() {
    // Variables to track flame detection
    Serial.println("Checking if there is fire!");
    int flameCount = 0;
    const int flameThreshold = 600; // Adjust as needed (600 * 5ms = 3 seconds)
    const int requiredFlameReadings = 2; // Adjust as needed
    bool fireDetected = false;

    // Take multiple readings to debounce
    for (int i = 0; i < flameThreshold; i++) {
        if (digitalRead(flamePin) == LOW) {
            flameCount++;
        }
        delay(5); // Adjust as needed
    }

    // If flame detected in most of the readings, trigger fire relay
    if (flameCount >= requiredFlameReadings) {
        fireDetected = true;
    } else {
        fireDetected = false;
    }

    // Control the fire relay based on flame detection
    if (fireDetected) {
        Serial.println("Fire detected!");
        tone(buzzerPin, 1000); // Activate the buzzer at a frequency of 1000Hz
        digitalWrite(fire_pump_RELAY_PIN, HIGH); // Turn on the fire relay

        // Continuously check for fire until extinguished
        while (fireDetected) {
            // Check flame status
            flameCount = 0;
            for (int i = 0; i < flameThreshold; i++) {
                if (digitalRead(flamePin) == LOW) {
                    flameCount++;
                }
                delay(5); // Adjust as needed
            }
            // Update fire detection status
            if (flameCount >= requiredFlameReadings) {
                fireDetected = true;
            } else {
                fireDetected = false;
            }
        }

        // Fire extinguished, turn off the fire relay
        Serial.println("No fire detected.");
        digitalWrite(fire_pump_RELAY_PIN, LOW); // Turn off the fire relay
        noTone(buzzerPin); // Turn off the buzzer
    } else {
        // No fire detected, turn off the fire relay
        Serial.println("No fire detected.");
        digitalWrite(fire_pump_RELAY_PIN, LOW); // Turn off the fire relay
        noTone(buzzerPin); // Turn off the buzzer
    }
}
