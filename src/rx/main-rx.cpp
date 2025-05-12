#include <SPI.h>
#include <LoRa.h>
#include <constants.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");
  LoRa.setPins(16, 17, 21); // NSS, RST, DIO0
  if (!LoRa.begin(LORA_CHANNEL)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  // try to parse packet
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

    // send acknowledgment back
    LoRa.beginPacket();
    LoRa.println(MESSAGE_ACK);
    LoRa.endPacket();
  }
}