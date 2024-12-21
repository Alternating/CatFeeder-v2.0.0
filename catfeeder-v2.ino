y #include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Espalexa.h>
#include "WebServerFunctions.h"

// Pin Definitions
#define MOTOR_PIN D1
#define LIMIT_SWITCH_PIN D3

// WiFi Credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASS";

// Global Variables
ESP8266WebServer server(80);
Espalexa espalexa;
bool motorRunning = false;
int rotationCount = 0;
bool lastSwitchState = HIGH;
unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_PRINT_INTERVAL = 1000; // Print debug info every 1 second

unsigned long timerDuration = 0;
unsigned long timerStart = 0;
int repeatCount = 0;
bool repeatForever = false;
int feedCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);

  connectWiFi();

  // Setup Alexa
  espalexa.addDevice("Cat Feeder", catFeederControl);

  // Setup Web Server
  setupWebServer();

  // Start Espalexa
  espalexa.begin(&server);
}

void loop() {
  espalexa.loop();
  server.handleClient();
  
  unsigned long currentTime = millis();
  
  if (motorRunning) {
    bool currentSwitchState = digitalRead(LIMIT_SWITCH_PIN);
    
    if (currentSwitchState == LOW && lastSwitchState == HIGH) {
      rotationCount++;
      Serial.print("Rotation detected. Count: ");
      Serial.println(rotationCount);
      
      if (rotationCount >= 2) {
        Serial.println("Two rotations completed. Stopping motor.");
        stopMotor();
        rotationCount = 0;
        feedCount++;
      }
    }
    
    lastSwitchState = currentSwitchState;
  } else if (timerDuration > 0 && currentTime - timerStart >= timerDuration) {
    Serial.println("Timer expired. Starting motor.");
    startMotor();
    
    if (repeatForever || repeatCount > 1) {
      timerStart = currentTime;
      if (!repeatForever) {
        repeatCount--;
      }
    } else {
      timerDuration = 0;
    }
  }
  
  if (currentTime - lastDebugPrint >= DEBUG_PRINT_INTERVAL) {
    printDebugInfo();
    lastDebugPrint = currentTime;
  }
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void catFeederControl(uint8_t brightness) {
  if (brightness == 0) {
    stopMotor();
    Serial.println("Cat Feeder turned OFF via Alexa");
  } else {
    startMotor();
    Serial.println("Cat Feeder turned ON via Alexa");
  }
}

void startMotor() {
  digitalWrite(MOTOR_PIN, HIGH);
  motorRunning = true;
  rotationCount = 0;
  Serial.println("Feeding started");
}

void stopMotor() {
  digitalWrite(MOTOR_PIN, LOW);
  motorRunning = false;
  Serial.println("Feeding stopped");
}

void printDebugInfo() {
  Serial.print("Limit Switch: ");
  Serial.print(digitalRead(LIMIT_SWITCH_PIN) == HIGH ? "HIGH" : "LOW");
  Serial.print(" | Motor: ");
  Serial.print(motorRunning ? "ON" : "OFF");
  Serial.print(" | Rotations: ");
  Serial.print(rotationCount);
  Serial.print(" | Total Feeds: ");
  Serial.print(feedCount);
  if (timerDuration > 0) {
    unsigned long remainingTime = (timerDuration - (millis() - timerStart)) / 1000;
    Serial.print(" | Timer: ");
    Serial.print(remainingTime);
    Serial.print("s");
    if (repeatForever) {
      Serial.print(" | Repeating forever");
    } else if (repeatCount > 0) {
      Serial.print(" | Repeats left: ");
      Serial.print(repeatCount);
    }
  }
  Serial.println();
}
