#include <SPI.h>
#include <LoRa.h>
#include <constants.h>
#include "Arduino.h"

void sendAcknowledgement();
void notifyWhenNoTx();

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");
  LoRa.setPins(33, 32, 25); // NSS, RST, DIO0
  if (!LoRa.begin(LORA_CHANNEL)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received HDO GPIO state: '");

    String hdoState = LoRa.readStringUntil(END_DELIMITER);
    hdoState.trim();
    if (hdoState == MESSAGE_ON) {
      Serial.print("ON");
    } else if (hdoState == MESSAGE_OFF) {
      Serial.print("OFF");
    } else {
      Serial.printf("Unknown state: '%s'\n", hdoState.c_str());
    }

    printf("' with RSSI %d\n", LoRa.packetRssi());

    sendAcknowledgement();
  }

  notifyWhenNoTx();
}

void sendAcknowledgement()
{
  LoRa.beginPacket();
  LoRa.println(MESSAGE_ACK);
  LoRa.endPacket();
}

void notifyWhenNoTx() {
  static unsigned long lastCheck = 0;
  static bool packetReceived = false;

  if (millis() - lastCheck >= 1000) {
    if (!packetReceived) {
      Serial.println("No TX");
    }
    packetReceived = false;
    lastCheck = millis();
  } else if (LoRa.parsePacket()) {
    packetReceived = true;
  }
}
