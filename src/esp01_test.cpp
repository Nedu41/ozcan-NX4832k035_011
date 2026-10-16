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


// ---- .tft'yi WiFi'den ekrana yükleme (Nextion "whmi-wri" protokolü; dosya flash'a yazılmaz, seriye akıtılır) ----
constexpr uint32_t NX_BAUD = 9600, UP_BAUD = 460800;
constexpr size_t UP_BLOCK = 4096;  // Nextion her 4096 baytta 0x05 ile onay verir
static uint8_t upBuf[UP_BLOCK];
static size_t upFill = 0;
static uint32_t upSize = 0, upSent = 0;
static bool upOk = false;

static bool nxWait05(uint32_t ms) {
  uint32_t t = millis();
  while (millis() - t < ms) {
    while (Serial.available())
      if (Serial.read() == 0x05) return true;
    yield();
  }
  return false;
}

static bool upFlushBlock() {
  Serial.write(upBuf, upFill);
  upFill = 0;
  return nxWait05(3000);
}

void handleTftUpload() {
  HTTPUpload& u = web.upload();
  if (u.status == UPLOAD_FILE_START) {
    upSize = web.arg("size").toInt();
    upSent = 0;
    upFill = 0;
    upOk = upSize > 0;
    if (!upOk) return;
    while (Serial.available()) Serial.read();
    Serial.printf("whmi-wri %u,%u,0", upSize, UP_BAUD);
    Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
    Serial.flush();
    delay(150);
    Serial.begin(UP_BAUD);
    upOk = nxWait05(5000);
  } else if (u.status == UPLOAD_FILE_WRITE && upOk) {
    for (size_t i = 0; i < u.currentSize && upOk; i++) {
      upBuf[upFill++] = u.buf[i];
      if (upFill == UP_BLOCK) upOk = upFlushBlock();
    }
    upSent += u.currentSize;
  } else if (u.status == UPLOAD_FILE_END) {
    if (upOk && upFill) upOk = upFlushBlock();
    upOk = upOk && upSent == upSize;
    Serial.begin(NX_BAUD);
  } else if (u.status == UPLOAD_FILE_ABORTED) {
    upOk = false;
    Serial.begin(NX_BAUD);
  }
}

void handleTftDone() {
  web.send(upOk ? 200 : 500, "text/plain", upOk ? "tamam: ekran yeniden baslatiliyor" : "HATA: yukleme basarisiz (guc kes-ver, tekrar dene)");
}

void handleTftForm() {
  web.send(200, "text/html",
           "<meta name=viewport content='width=device-width'><body style='background:#14141A;color:#DADAE4;font-family:sans-serif;padding:20px'>"
           "<h3>Nextion .tft yukle</h3><form id=f method=POST enctype=multipart/form-data>"
           "<input type=file name=tft accept=.tft required> <button>Yukle</button></form>"
           "<script>f.onsubmit=()=>{f.action='/tft?size='+f.tft.files[0].size}</script>");
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
  web.on("/tft", HTTP_GET, handleTftForm);
  web.on("/tft", HTTP_POST, handleTftDone, handleTftUpload);
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
