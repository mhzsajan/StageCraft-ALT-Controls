/*
 * PROTOTYPE - not hardware-tested.
 *
 * Direct-to-WING ALT toggle (recommended design, proven by the phone-canvas test).
 * On each button press:
 *   1) GET  /%<localport>/ch/N/in/set/altsrc   -> WING replies ,sfi (ascii, raw, int)
 *   2) flip the int (0<->1)
 *   3) SET  /ch/N/in/set/altsrc with a FLOAT tag (,f) - WING ignores integer-tagged SETs!
 * No AbleSet inbound, no subscribe, no computer needed.
 * See docs/PROTOCOL-EVIDENCE.md and docs/ABLESET-CONTROL.md.
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
IPAddress wingIp;

unsigned long lastPress = 0;
int lastButton = HIGH;

// ---- OSC helpers ----------------------------------------------------------
static void oscString(uint8_t *buf, int &pos, const char *text) {
  size_t len = strlen(text);                 // caller guarantees 160-byte capacity
  memcpy(buf + pos, text, len);
  pos += (int)len;
  buf[pos++] = 0;
  while (pos % 4 != 0) buf[pos++] = 0;
}

static void oscFloat(uint8_t *buf, int &pos, float value) {
  uint32_t bits;
  memcpy(&bits, &value, 4);                  // ESP32 is little-endian
  buf[pos++] = (uint8_t)(bits >> 24);
  buf[pos++] = (uint8_t)(bits >> 16);
  buf[pos++] = (uint8_t)(bits >> 8);
  buf[pos++] = (uint8_t)(bits);
}

static uint32_t be32(const uint8_t *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int readPaddedString(const uint8_t *buf, int len, int &pos,
                            char *out, int outSize) {
  if (pos >= len) return -1;
  int start = pos;
  while (pos < len && buf[pos] != 0) {
    if (pos - start < outSize - 1) out[pos - start] = (char)buf[pos];
    pos++;
  }
  if (pos >= len) return -1;
  out[pos - start] = '\0';
  int chars = pos - start;
  pos++;                                      // consume NUL
  while (pos % 4 != 0) pos++;                 // consume padding
  return chars;
}

// Parse WING's GET reply: address + ",sfi" + (ascii string, float raw, int value)
static bool parseAltValue(const uint8_t *buf, int len, int &value) {
  int pos = 0;
  char text[128];
  if (readPaddedString(buf, len, pos, text, sizeof(text)) < 0) return false;
  if (readPaddedString(buf, len, pos, text, sizeof(text)) < 0) return false;
  if (strcmp(text, ",sfi") != 0) return false;
  if (readPaddedString(buf, len, pos, text, sizeof(text)) < 0) return false; // ascii
  if (pos + 8 > len) return false;             // float raw + int value
  pos += 4;                                    // skip raw float
  value = (int)be32(buf + pos);
  return true;
}

static bool getCurrent(int channel, int &value) {
  uint8_t packet[160];
  int pos = 0;
  String address = String("/%") + LOCAL_PORT + "/ch/" + channel + "/in/set/altsrc";
  oscString(packet, pos, address.c_str());
  oscString(packet, pos, ",");                 // no arguments
  udp.beginPacket(wingIp, WING_PORT);
  udp.write(packet, pos);
  udp.endPacket();

  uint8_t reply[256];
  unsigned long deadline = millis() + 800;
  while (millis() < deadline) {
    int size = udp.parsePacket();
    if (size > 0) {
      int n = udp.read(reply, sizeof(reply));
      return parseAltValue(reply, n, value);
    }
    delay(5);
  }
  return false;                                // timeout: do NOT guess the state
}

static void setAlt(int channel, int value) {
  uint8_t packet[160];
  int pos = 0;
  String address = String("/ch/") + channel + "/in/set/altsrc";
  oscString(packet, pos, address.c_str());
  oscString(packet, pos, ",f");                // float tag - mandatory on WING
  oscFloat(packet, pos, (float)value);
  udp.beginPacket(wingIp, WING_PORT);
  udp.write(packet, pos);
  udp.endPacket();
  Serial.printf("SET /ch/%d/in/set/altsrc = %d\n", channel, value);
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
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) delay(250);
  udp.begin(LOCAL_PORT);                       // fixed port for GET replies
  wingIp.fromString(WING_IP);

  if (WiFi.status() == WL_CONNECTED)
    showStatus("WIFI OK", WiFi.localIP().toString().c_str());
  else
    showStatus("WIFI FAIL", "phone canvas fallback");
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);
  unsigned long now = millis();

  if (reading == LOW && lastButton == HIGH && now - lastPress > DEBOUNCE_MS) {
    lastPress = now;
    if (WiFi.status() == WL_CONNECTED) {
      int current = -1;
      if (getCurrent(WING_CHANNEL, current)) {
        int next = (current == 1) ? 0 : 1;
        setAlt(WING_CHANNEL, next);
        showStatus(next == 1 ? "ALT" : "MAIN", "WING toggled");
#if LED_PIN >= 0
        digitalWrite(LED_PIN, HIGH);
        delay(80);
        digitalWrite(LED_PIN, LOW);
#endif
      } else {
        showStatus("READ FAIL", "no toggle sent");
      }
    } else {
      Serial.println("button pressed but WiFi is down - nothing sent");
    }
  }
  lastButton = reading;
  delay(10);
}
