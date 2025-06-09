#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <constants.h>

String lastMessage = "";
bool hasNewMessage = false;
int lastRssi = 0;
unsigned long lastReceiveTime = 0;

Adafruit_NeoPixel pixels(1, D3, NEO_RGB + NEO_KHZ800);

void onReceive(int packetSize);
void sendAcknowledgement();
void notifyWhenNoTx();

uint32_t getColorRed() { return pixels.Color(255, 0, 0); }
uint32_t getColorGreen() { return pixels.Color(0, 255, 0); }
void setPixelColor(uint32_t color) {
  pixels.setPixelColor(0, color);
  pixels.show();
}

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);

  pixels.begin();
  pixels.clear();
  pixels.setBrightness(50);

  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");

  pinMode(LED_BUILTIN, OUTPUT);
  LoRa.setPins(D8, D0, D1);
  if (!LoRa.begin(LORA_CHANNEL)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.onReceive(onReceive);
  LoRa.receive();
}

void loop() {
  if (hasNewMessage) {
    hasNewMessage = false;

    digitalWrite(LED_BUILTIN, LOW);
    String hdoState = lastMessage;
    hdoState.trim();

    Serial.print("Received HDO GPIO state: '");
    if (hdoState == MESSAGE_ON) {
      Serial.print("ON");
      setPixelColor(getColorGreen());
    } else if (hdoState == MESSAGE_OFF) {
      Serial.print("OFF");
      setPixelColor(getColorRed());
    } else {
      Serial.printf("Unknown state: '%s'\n", hdoState.c_str());
    }

    Serial.printf("' with RSSI %d\n", lastRssi);
    sendAcknowledgement();
    digitalWrite(LED_BUILTIN, HIGH);
  }

  notifyWhenNoTx();
}

void onReceive(int packetSize) {
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
    setPixelColor(getColorRed());
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
