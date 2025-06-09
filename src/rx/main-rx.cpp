#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <constants.h>
#include "Arduino.h"

#define RELAY_PIN D2

String lastMessage = "";
bool hasNewMessage = false;
int lastRssi = 0;
unsigned long lastReceiveTime = 0;

Adafruit_NeoPixel pixels(1, D3, NEO_RGB + NEO_KHZ800);

void onReceive(int packetSize);
void sendAcknowledgement();
void notifyWhenNoTx();
void triggerOn();
void triggerOff();

uint32_t getColorRed() { return pixels.Color(255, 0, 0); }
uint32_t getColorGreen() { return pixels.Color(0, 255, 0); }
void setPixelColor(uint32_t color) {
  pixels.setPixelColor(0, color);
  pixels.show();
}

/**
 * Main LED indication:
 * - Red: HDO OFF state received or no TX for 50 seconds
 * - Green: HDO ON state received in last 50 seconds
 * 
 * Onboard LED indication:
 * - Blink: received HDO state
 * - Still: no new HDO state received
 *  */
void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(50);

  Serial.begin(9600);
  delay(100);
  Serial.println("LoRa Receiver");

  LoRa.setPins(D8, D0, D1); // NSS, RST, DIO0
  if (!LoRa.begin(LORA_CHANNEL)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.onReceive(onReceive);
  LoRa.receive();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  triggerOff();
}

void loop() {
  if (hasNewMessage) {
    hasNewMessage = false;

    digitalWrite(LED_BUILTIN, LOW);
    String hdoState = lastMessage;
    hdoState.trim();

    Serial.print("Received HDO GPIO state: '");
    if (hdoState == MESSAGE_ON) {
      triggerOn();
    } else if (hdoState == MESSAGE_OFF) {
      triggerOff();
    } else {
      Serial.printf("Unknown state: '%s'\n", hdoState.c_str());
    }

    Serial.printf("' with RSSI %d\n", lastRssi);
    sendAcknowledgement();
    digitalWrite(LED_BUILTIN, HIGH);
  }

  notifyWhenNoTx();
}

void onReceive(int packetSize)
{
  if (packetSize == 0) return;

  // Přečti ručně jednotlivé bajty – bezpečné
  String msg = "";
  while (LoRa.available()) {
    char c = (char)LoRa.read();
    if (c == END_DELIMITER) break;
    msg += c;
  }

  lastReceiveTime = millis();

  if (msg.length() == 0) return;

  lastMessage = msg;
  lastRssi = LoRa.packetRssi();
  hasNewMessage = true;
  LoRa.receive();
}

void sendAcknowledgement() {
  LoRa.beginPacket();
  LoRa.println(MESSAGE_ACK);
  LoRa.endPacket();
  LoRa.receive();
}

void notifyWhenNoTx() {
  if ((unsigned long)(millis() - lastReceiveTime) > 50000UL) {
    Serial.println("No TX");
    if (digitalRead(RELAY_PIN) == HIGH) {
      triggerOff();
    }
  }
}


void triggerOn()
{
  Serial.print("ON");
  digitalWrite(RELAY_PIN, HIGH);
  setPixelColor(getColorGreen());
}


void triggerOff()
{
  Serial.print("OFF");
  digitalWrite(RELAY_PIN, LOW);
  setPixelColor(getColorRed());
}