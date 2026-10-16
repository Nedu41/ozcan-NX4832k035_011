// ESP-01 (ESP8266, 1 MB) bağlantı + OTA testi. Nextion henüz bağlı değil.
// WiFi: kayıtlı bilgi varsa onunla; yoksa "NXHub-Kurulum" AP'si açılır, telefondan ağ seçilir (WiFiManager).
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

constexpr const char* HOSTNAME = "nxhub";
constexpr const char* AP_NAME = "NXHub-Kurulum";

WiFiManager wm;
ESP8266WebServer web(81);  // Nextion sinama arayüzü (seri port ekranda olduğu için sonuç web'den okunur)

// Ekrana komut gönderir (sonuna 3 x 0xFF), cevabı hex + metin olarak döndürür.
String nxExchange(const String& c, uint32_t waitMs = 400) {
  while (Serial.available()) Serial.read();
  Serial.print(c);
  Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
  String hex, txt;
  uint32_t t = millis();
  while (millis() - t < waitMs) {
    while (Serial.available()) {
      uint8_t b = Serial.read();
      char h[4]; snprintf(h, sizeof h, "%02X ", b);
      hex += h;
      txt += (b >= 32 && b < 127) ? (char)b : '.';
    }
    yield();
  }
  return hex.length() ? hex + " | " + txt : String("(cevap yok)");
}

void handleProbe() {
  static const uint32_t bauds[] = {9600, 115200, 19200, 38400, 57600, 4800, 2400, 31250, 230400};
  String out;
  for (uint32_t b : bauds) {
    Serial.begin(b);
    delay(50);
    nxExchange("", 100);  // olası yarım komutu temizle
    String r = nxExchange("connect", 500);
    out += String(b) + ": " + r + "\n";
    if (r.indexOf("comok") >= 0) break;
  }
  web.send(200, "text/plain", out);
}

void handleNx() {  // /nx?c=page%201&b=9600
  if (web.hasArg("b")) Serial.begin(web.arg("b").toInt());
  web.send(200, "text/plain", nxExchange(web.arg("c")));
}

void setup() {
  Serial.begin(9600);  // UART0 ekrana bağlı: log yok, Nextion varsayılan hızı
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  wm.setConfigPortalTimeout(180);  // kurulum AP'si 3 dk açık kalır, sonra yeniden dener
  wm.setHostname(HOSTNAME);
  if (!wm.autoConnect(AP_NAME)) {
    delay(500);
    ESP.restart();
  }
  wm.startWebPortal();  // bağlıyken de http://<ip>/ ile WiFi değiştirilebilir
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
  web.on("/probe", handleProbe);
  web.on("/nx", handleNx);
  web.on("/heap", [] { web.send(200, "text/plain", String(ESP.getFreeHeap())); });
  web.begin();
}

void loop() {
  wm.process();
  ArduinoOTA.handle();
  web.handleClient();
}
