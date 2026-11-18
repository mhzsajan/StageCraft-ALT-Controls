/*
 * PROTOTYPE - not hardware-tested.
 * Sends /alt/<UNIT_NAME>/toggle to AbleSet (UDP 39051) on each button press.
 * BLE? No. WiFi only. No credentials in this file - see config.h.example.
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#ifdef USE_DISPLAY
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#endif

#include "config.h"

WiFiUDP udp;
IPAddress ableSetIp;

unsigned long lastPress = 0;
int lastButton = HIGH;

// --- minimal OSC encoder ---------------------------------------------------
// OSC strings are NUL-terminated and padded to a 4-byte boundary.
static void oscAppendString(uint8_t *buf, int &pos, const char *text) {
  size_t len = strlen(text);
  memcpy(buf + pos, text, len);          // caller guarantees capacity
  pos += (int)len;
  buf[pos++] = 0;
  while (pos % 4 != 0) buf[pos++] = 0;
}

static void sendToggle() {
  uint8_t packet[160];
  int pos = 0;
  String address = String("/alt/") + UNIT_NAME + "/toggle";
  oscAppendString(packet, pos, address.c_str());
  oscAppendString(packet, pos, ",");     // empty type tag: message has no arguments
  udp.beginPacket(ableSetIp, ABLESET_PORT);
  udp.write(packet, pos);
  udp.endPacket();
  Serial.printf("sent %s -> %s:%d (%d bytes)\n",
                address.c_str(), ABLESET_IP, ABLESET_PORT, pos);
#ifdef USE_DISPLAY
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("SENT");
  display.setTextSize(1);
  display.println(address);
  display.display();
#endif
}

static void showStatus(const char *line1, const char *line2) {
  Serial.println(line1);
  Serial.println(line2);
#ifdef USE_DISPLAY
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(line1);
  display.setTextSize(1);
  display.println(line2);
  display.display();
#endif
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#if LED_PIN >= 0
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
#endif
#ifdef USE_DISPLAY
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);
#endif
  showStatus("BOOT", "connecting WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(250);
  }
  udp.begin(42123);  // arbitrary local port; replies are not needed for a toggle message
  ableSetIp.fromString(ABLESET_IP);

  if (WiFi.status() == WL_CONNECTED) {
    showStatus("WIFI OK", WiFi.localIP().toString().c_str());
  } else {
    showStatus("WIFI FAIL", "phone canvas fallback");
  }
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);
  unsigned long now = millis();

  if (reading == LOW && lastButton == HIGH && now - lastPress > DEBOUNCE_MS) {
    lastPress = now;
    if (WiFi.status() == WL_CONNECTED) {
      sendToggle();
#if LED_PIN >= 0
      digitalWrite(LED_PIN, HIGH);
      delay(80);
      digitalWrite(LED_PIN, LOW);
#endif
    } else {
      Serial.println("button pressed but WiFi is down - nothing sent");
    }
  }
  lastButton = reading;
  delay(10);
}
