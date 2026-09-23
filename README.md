# EcoWatch-Smart-Greenhouse
An IoT-based smart greenhouse management and fire safety system utilizing an Arduino UNO, ESP8266, and ThingSpeak to automate environmental controls, monitor plant health, and autonomously suppress fires.
# EcoWatch: Smart Greenhouse Environment Management with Fire Safety

## Overview
EcoWatch is an IoT-based automation system designed to manage greenhouse environments and provide integrated fire safety mechanisms. Developed as a final project for the EEE 310 Communication Laboratory at the Bangladesh University of Engineering and Technology (BUET), the system utilizes sensor technology and intelligent control to optimize resource utilization and ensure plant health. 

The system relies on an Arduino UNO and an ESP8266 Wi-Fi module to process and transmit environmental data (temperature, humidity, and soil moisture) to the ThingSpeak Cloud platform for real-time monitoring and analysis. Alongside environmental automation, EcoWatch addresses fire safety by featuring a comprehensive detection and autonomous water-pumping suppression system.

## Key Features
* **Cloud-Based Monitoring:** Aggregates and visualizes live temperature, humidity, and soil moisture data streams remotely via the ThingSpeak API.
* **Automated Irrigation:** Dynamically activates a submersible water pump based on real-time soil moisture percentage readings.
* **Climate Control:** Automates a DC cooling fan and lighting system in response to specific temperature, humidity, and soil moisture thresholds to maintain optimal growing conditions.
* **Fire Detection and Suppression:** Utilizes a flame sensor to detect fire outbreaks, immediately triggering an electromagnetic buzzer and a dedicated fire-suppression water pump to extinguish the flame.
* **Light Intensity Control:** Integrates an LDR (Light Dependent Resistor) and LED to adapt greenhouse lighting based on external light availability and temperature readings.

## Hardware Components
The physical prototype was built using the following primary components:
* **Microcontroller & Connectivity:** Arduino UNO Board, ESP8266 ESP-01 Wi-Fi Wireless Transceiver.
* **Sensors:** DHT11 Digital Relative Humidity & Temperature Sensor, YL-69 Soil Hygrometer Moisture Sensor, Flame Sensor Module, 20mm LDR.
* **Actuators:** 3V Mini DC Submersible Water Pumps (x2), 12V/5V Mini Cooling Fan, 1207 Passive Electromagnetic Buzzer, 5V Relay Modules (x3), 5mm Red LED.
* **Display & Power:** 16x2 Serial LCD Module with I2C Adapter, 9V Batteries.

## System Logic and Workflow
The Arduino firmware follows a strict algorithmic workflow to manage the greenhouse autonomously:
* **Irrigation Logic:** If soil moisture drops below 20%, the soil pump turns on and waters the plants until the moisture reading reaches 50%.
* **Ventilation Logic:** If soil moisture exceeds 65%, the DC fan and lighting turn on. The fan continues to run until the soil moisture drops below 60%.
* **Temperature & Lighting Logic:** If the temperature reaches or exceeds 24°C, the lighting is automatically turned off to prevent overheating. 
* **Fire Safety Logic:** The flame sensor continuously scans the area. If a fire is detected (requiring multiple consecutive readings to debounce false alarms), the buzzer sounds at 1000Hz and the fire relay activates a water pump. The pump and alarm run continuously until the flame is extinguished.
