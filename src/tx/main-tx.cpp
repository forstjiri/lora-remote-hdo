#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WiFi.h> // IF 8266
#include <Adafruit_NeoPixel.h>
#include <constants.h>

#define HDO_GPIO D2 // Pull-up GPIO pin for HDO state

bool waitForAcknowledgment(int timeout);
uint32_t getColorRed();
uint32_t getColorGreen();
uint32_t getColorBlue();
void setPixelColor(uint32_t color);
void ledBlink(int forPeriod, uint32_t color);

Adafruit_NeoPixel pixels(1, D3, NEO_RGB + NEO_KHZ800);

/**
 * Send HDO GPIO state over LoRa
 *
 * Led indication debug:
 * - Red still: HDO is OFF || Green still: HDO is ON
 * - Blue blinking: waiting for ACK
 * - Green blinking: ACK received || Red blinking: no ACK received
 */
void setup() {
  WiFi.mode(WIFI_OFF); // IF 8266
  WiFi.forceSleepBegin(); // IF 8266
  delay(1);

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(50);
  pinMode(HDO_GPIO, INPUT_PULLUP); // Set HDO GPIO as input with pull-up resistor
  Serial.begin(9600);
  Serial.println("LoRa transmitter, starting...");
  while (!Serial) {
    ledBlink(1000, getColorBlue());
  }
  LoRa.setPins(D8, D0, D1); // NSS, RST, DIO0
  if (!LoRa.begin(LORA_CHANNEL)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  String state = digitalRead(HDO_GPIO) ? MESSAGE_OFF : MESSAGE_ON;
  setPixelColor(state == MESSAGE_ON ? getColorGreen() : getColorRed());
  delay(1000);
  Serial.printf("Sending HDO GPIO state: %s\n", state.c_str());
  LoRa.beginPacket();
  LoRa.println(state);
  LoRa.endPacket();

  if (waitForAcknowledgment(500)) {
    ledBlink(1000, getColorGreen());
    Serial.printf("ACK received with RSSI: %d\n", LoRa.packetRssi());
  } else {
    ledBlink(1000, getColorRed());
    Serial.println("No ACK received");
  }
}

bool waitForAcknowledgment(int timeout) {
  long startTime = millis();
  bool ackReceived = false;

  while (millis() - startTime < timeout) {
    setPixelColor(getColorBlue());
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String received = LoRa.readStringUntil(END_DELIMITER);
      received.trim();
      Serial.printf("Received: '%s'\n", received.c_str());
      if (received == MESSAGE_ACK) {
        ackReceived = true;
        break;
      }
    }
    pixels.clear();
    pixels.show();
  }

  return ackReceived;
}

uint32_t getColorRed() {
  return pixels.Color(255, 0, 0);
}

uint32_t getColorGreen() {
  return pixels.Color(0, 255, 0);
}

uint32_t getColorBlue() {
  return pixels.Color(0, 0, 255);
}

void ledBlink(int forPeriod, uint32_t color) {
  int elapsed = 0;
  while (elapsed < forPeriod) {
    pixels.setPixelColor(0, color);
    pixels.show();
    delay(100);
    pixels.clear();
    pixels.show();
    delay(100);
    elapsed += 200;
  }
}

void setPixelColor(uint32_t color) {
  pixels.setPixelColor(0, color);
  pixels.show();
}
