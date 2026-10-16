// ESP-01 (ESP8266, 1 MB) bağlantı + OTA + Nextion web kontrolü (http://nxhub.local:81/).
// WiFi: kayıtlı bilgi varsa onunla; yoksa "NXHub-Kurulum" AP'si açılır, telefondan ağ seçilir (WiFiManager).
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

#include "foto.h"

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
constexpr uint32_t NX_BAUD = 9600, UP_BAUD = 115200;
constexpr size_t UP_BLOCK = 4096;  // Nextion her 4096 baytta 0x05 ile onay verir
static uint8_t upBuf[UP_BLOCK];
static size_t upFill = 0;
static uint32_t upSize = 0, upSent = 0;
static bool upOk = false;
static String upHata;

static String upGelen;  // onay beklerken gelen 0x05 dışı baytlar (teşhis)

static bool nxWait05(uint32_t ms) {
  uint32_t t = millis();
  while (millis() - t < ms) {
    while (Serial.available()) {
      uint8_t b = Serial.read();
      if (b == 0x05) return true;
      if (upGelen.length() < 60) { char h[4]; snprintf(h, sizeof h, "%02X ", b); upGelen += h; }
    }
    yield();
  }
  return false;
}

static bool upFlushBlock() {
  Serial.write(upBuf, upFill);
  upFill = 0;
  return nxWait05(10000);
}

void handleTftUpload() {
  HTTPUpload& u = web.upload();
  if (u.status == UPLOAD_FILE_START) {
    upSize = web.arg("size").toInt();
    upSent = 0;
    upFill = 0;
    upOk = upSize > 0;
    upHata = upOk ? "" : "boyut yok";
    upGelen = "";
    if (!upOk) return;
    nxExchange("", 100);  // yarım komut kalıntısını temizle
    if (nxExchange("connect", 500).indexOf("comok") < 0) { upOk = false; upHata = "ekran 9600'de cevap vermiyor"; return; }
    while (Serial.available()) Serial.read();
    Serial.printf("whmi-wri %u,%u,0", upSize, UP_BAUD);
    Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF);
    Serial.flush();
    Serial.begin(UP_BAUD);  // 0x05 onayı yeni hızda gelir; gecikme olursa kaçar
    upOk = nxWait05(5000);
    if (!upOk) upHata = "baslangic onayi (0x05) gelmedi";
  } else if (u.status == UPLOAD_FILE_WRITE && upOk) {
    for (size_t i = 0; i < u.currentSize && upOk; i++) {
      upBuf[upFill++] = u.buf[i];
      if (upFill == UP_BLOCK && !(upOk = upFlushBlock())) upHata = "blok onayi gelmedi, gonderilen: " + String(upSent + i + 1);
    }
    upSent += u.currentSize;
  } else if (u.status == UPLOAD_FILE_END) {
    if (upOk && upFill && !(upOk = upFlushBlock())) upHata = "son blok onayi gelmedi";
    if (upOk && upSent != upSize) { upOk = false; upHata = "boyut uyusmuyor: " + String(upSent) + "/" + String(upSize); }
    Serial.begin(NX_BAUD);
  } else if (u.status == UPLOAD_FILE_ABORTED) {
    upOk = false;
    upHata = "baglanti koptu";
    Serial.begin(NX_BAUD);
  }
}

void handleTftDone() {
  web.send(upOk ? 200 : 500, "text/plain", upOk ? String("tamam: ekran yeniden baslatiliyor") : "HATA: " + upHata + " | ekrandan: " + (upGelen.length() ? upGelen : String("-")) + " (ekranin gucunu kes-ver, tekrar dene)");
}

// ---- Web'den ekran kontrolü (ön camda dokunmatik kullanılmıyor; ornek sayfa: t_durum, b_led) ----
constexpr const char* SURUM = "v0.2.0";
constexpr uint16_t C_DUGME = 10599, C_YAZI = 57052, C_YESIL = 36628, C_KOYU = 2146;  // RGB565: #2C2C38 #DADAE4 #8FE0A0 #0E0E14
static bool ledAcik = false;
static String durumTxt = "bekleniyor", nxSon = "-";

// Komutu yollar; Nextion (bkcmd=2) yalnız hatada cevap verir. Hata yoksa "tamam", varsa hex kodu.
static String nxCmd(const String& c) {
  String r = nxExchange(c, 120);
  if (r.startsWith("(")) return "tamam";
  return r.startsWith("1A ") ? String("HATA: bilesen yok") : "HATA: " + r.substring(0, r.indexOf(" |"));
}

// Font ASCII: Türkçe harfleri sadeleştir, tırnak/ters bölü at, txt_maxl (30) ile sınırla.
static String nxMetin(String s) {
  static const char* tr[][2] = {{"ç","c"},{"Ç","C"},{"ğ","g"},{"Ğ","G"},{"ı","i"},{"İ","I"},{"ö","o"},{"Ö","O"},{"ş","s"},{"Ş","S"},{"ü","u"},{"Ü","U"}};
  for (auto& p : tr) s.replace(p[0], p[1]);
  s.replace("\"", ""); s.replace("\\", "");
  return s.substring(0, 30);
}

static void ledUygula() {
  String r = nxCmd(String("b_led.bco=") + (ledAcik ? C_YESIL : C_DUGME));
  String r2 = nxCmd(String("b_led.pco=") + (ledAcik ? C_KOYU : C_YAZI));
  String r3 = nxCmd(ledAcik ? "b_led.txt=\"ACIK\"" : "b_led.txt=\"KAPALI\"");
  nxSon = r != "tamam" ? r : r2 != "tamam" ? r2 : r3;  // ilk hata
}

static void apiDurum() {
  String j = "{\"led\":" + String(ledAcik) + ",\"durum\":\"" + durumTxt + "\",\"nx\":\"" + nxSon +
             "\",\"ssid\":\"" + WiFi.SSID() + "\",\"ip\":\"" + WiFi.localIP().toString() + "\",\"surum\":\"" + SURUM +
             "\",\"tarih\":\"" __DATE__ " " __TIME__ "\"}";
  web.send(200, "application/json", j);
}

static const char PANO_CSS[] PROGMEM = R"CSS(
/* === PANO TEMASI (SuDepoSis_v2/docs/tema/standart1_pano_temasi.css ile AYNI blok) === */
:root,.dark{--bg:#060607;--card:#0b0c0f;--text:#e5e7eb;--muted:#9ca3af;--border:#3a3f48;--border-strong:#5b616c;--primary:#60a5fa;--accent:#34d399;--warn:#fbbf24;--danger:#f87171;--danger-bg:#3a2222;--danger-bg-t:rgba(58,34,34,.6);--tab-bg:#101216;--shadow:0 2px 8px rgba(0,0,0,.8);--grid-dot:rgba(150,160,180,.05)}
body{color-scheme:dark;background-color:#08090a;background-attachment:fixed;background-image:radial-gradient(ellipse at 50% 38%,rgba(0,0,0,0) 30%,rgba(0,0,0,.85) 100%),linear-gradient(180deg,rgba(150,72,26,.30),rgba(150,72,26,0) 42%) 7% 0/5px 100% no-repeat,linear-gradient(180deg,rgba(150,72,26,.22),rgba(150,72,26,0) 42%) 23% 0/3px 100% no-repeat,linear-gradient(180deg,rgba(150,72,26,.26),rgba(150,72,26,0) 42%) 58% 0/6px 100% no-repeat,linear-gradient(180deg,rgba(150,72,26,.28),rgba(150,72,26,0) 42%) 81% 0/4px 100% no-repeat,linear-gradient(180deg,rgba(150,72,26,.2),rgba(150,72,26,0) 42%) 94% 0/3px 100% no-repeat,repeating-linear-gradient(118deg,transparent 0 170px,rgba(255,255,255,.05) 171px,transparent 173px,transparent 310px,rgba(255,255,255,.035) 311px,transparent 312px),url('data:image/svg+xml,%3Csvg%20xmlns%3D%27http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%27%20width%3D%27700%27%20height%3D%27700%27%3E%3Cfilter%20id%3D%27n%27%3E%3CfeTurbulence%20type%3D%27fractalNoise%27%20baseFrequency%3D%27.007%27%20numOctaves%3D%273%27%20stitchTiles%3D%27stitch%27%2F%3E%3CfeColorMatrix%20values%3D%270%200%200%200%20.75%20%200%200%200%200%20.78%20%200%200%200%200%20.85%20%201.1%200%200%200%20-.38%27%2F%3E%3C%2Ffilter%3E%3Crect%20width%3D%27100%25%27%20height%3D%27100%25%27%20filter%3D%27url%28%23n%29%27%2F%3E%3C%2Fsvg%3E'),url('data:image/svg+xml,%3Csvg%20xmlns%3D%27http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%27%20width%3D%27500%27%20height%3D%27500%27%3E%3Cfilter%20id%3D%27n%27%3E%3CfeTurbulence%20type%3D%27fractalNoise%27%20baseFrequency%3D%27.012%27%20numOctaves%3D%273%27%20stitchTiles%3D%27stitch%27%2F%3E%3CfeColorMatrix%20values%3D%270%200%200%200%200%20%200%200%200%200%200%20%200%200%200%200%200%20%201.3%200%200%200%20-.55%27%2F%3E%3C%2Ffilter%3E%3Crect%20width%3D%27100%25%27%20height%3D%27100%25%27%20filter%3D%27url%28%23n%29%27%2F%3E%3C%2Fsvg%3E'),url('data:image/svg+xml,%3Csvg%20xmlns%3D%27http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%27%20width%3D%27160%27%20height%3D%27160%27%3E%3Cfilter%20id%3D%27n%27%3E%3CfeTurbulence%20type%3D%27fractalNoise%27%20baseFrequency%3D%27.9%27%20numOctaves%3D%272%27%20stitchTiles%3D%27stitch%27%2F%3E%3CfeColorMatrix%20values%3D%270%200%200%200%201%20%200%200%200%200%201%20%200%200%200%200%201%20%200%200%200%20.16%200%27%2F%3E%3C%2Ffilter%3E%3Crect%20width%3D%27100%25%27%20height%3D%27100%25%27%20filter%3D%27url%28%23n%29%27%2F%3E%3C%2Fsvg%3E');background-size:auto,auto,auto,auto,auto,auto,700px 700px,500px 500px,160px 160px}
.card,details.card{background-color:transparent;border-width:4px;border-style:solid;border-top-color:#6a707c;border-right-color:#4b515c;border-bottom-color:#3a3f48;border-radius:6px;box-shadow:0 0 0 2px #000,inset 0 0 0 1px #000,inset 0 2px 0 rgba(255,255,255,.07),inset 0 0 24px rgba(0,0,0,.6),0 5px 14px rgba(0,0,0,.9);background-repeat:no-repeat;background-image:radial-gradient(circle 4.5px at 12px 12px,#a3a9b4 0,#59606c 38%,#16181d 75%,transparent 82%),radial-gradient(circle 4.5px at calc(100% - 12px) 12px,#a3a9b4 0,#59606c 38%,#16181d 75%,transparent 82%),radial-gradient(circle 4.5px at 12px calc(100% - 12px),#a3a9b4 0,#59606c 38%,#16181d 75%,transparent 82%),radial-gradient(circle 4.5px at calc(100% - 12px) calc(100% - 12px),#a3a9b4 0,#59606c 38%,#16181d 75%,transparent 82%)}
.card h3,.card>summary,details.card>summary{text-transform:uppercase;letter-spacing:1.6px;font-family:Consolas,'DejaVu Sans Mono','Courier New',monospace;font-size:13px;font-weight:700;color:#dcd2ac;text-shadow:0 1px 0 #000,0 0 7px rgba(220,210,172,.18)}
.card h3{padding-bottom:6px;border-bottom:1px dashed #454a54}
.btn{padding:10px 12px;border:1px solid #000;border-radius:9px;cursor:pointer;font-weight:600;background-color:#020203;background-image:linear-gradient(180deg,rgba(255,255,255,.09),rgba(255,255,255,0) 45%,rgba(0,0,0,.4));color:#a3a8b2;box-shadow:inset 0 1px 0 rgba(255,255,255,.14),0 1px 2px rgba(0,0,0,.5);transition:transform .08s ease,box-shadow .25s,color .25s;filter:none}
.btn:active{transform:translateY(2px);box-shadow:inset 0 2px 5px rgba(0,0,0,.6);filter:none}
.btn{color:#7d828c}
.btn-primary,.btn-mavi{--t:96,165,250}.btn-accent,.btn-yesil{--t:34,230,96}.btn-danger,.btn-kirmizi{--t:255,85,85}.btn-warn,.btn-turuncu{--t:255,190,70}
.btn:active{color:rgb(var(--t,200,205,215));text-shadow:0 0 5px rgb(var(--t,200,205,215)),0 0 12px rgba(var(--t,200,205,215),.6)}
.btn-primary,.btn-accent,.btn-danger,.btn-warn,.btn-yesil,.btn-turuncu,.btn-mavi,.btn-kirmizi{background-color:#020203}
.btn.btn-on{--t:34,230,96;font-weight:800;color:rgb(var(--t));text-shadow:0 0 4px rgb(var(--t)),0 0 10px rgba(var(--t),.8),0 0 20px rgba(var(--t),.6);box-shadow:inset 0 1px 0 rgba(255,255,255,.14),0 0 6px 1px rgba(var(--t),.6),0 0 22px 5px rgba(var(--t),.35);animation:btnNefes 3.6s ease-in-out infinite}
.btn.btn-on.ton-kirmizi{--t:255,85,85}.btn.btn-on.ton-amber{--t:255,190,70}.btn.btn-on.ton-mavi{--t:70,150,255}.btn.btn-on.sabit{animation:none}
@keyframes btnNefes{50%{box-shadow:inset 0 1px 0 rgba(255,255,255,.14),0 0 5px 1px rgba(var(--t),.45),0 0 16px 3px rgba(var(--t),.22);text-shadow:0 0 3px rgb(var(--t)),0 0 7px rgba(var(--t),.7),0 0 14px rgba(var(--t),.5)}}
@media(prefers-reduced-motion:reduce){.btn.btn-on{animation:none}}
.nav,.sekmeler,.dark .sekmeler{background:rgba(5,5,6,.35)}
body .nav button,body .sekme-btn{background:transparent;border:1px solid #3a3f48;color:#a3a8b2;box-shadow:none}
body .nav button.active,body .sekme-btn.aktif{background:transparent;color:#9ec3f5;border-color:#60a5fa;text-shadow:0 0 6px rgba(96,165,250,.8);box-shadow:0 0 8px rgba(96,165,250,.45)}
input,select,.input,textarea{background:#060708;border:1px solid #3a3f48;color:#e5e7eb}
.tema-btn{display:none}
/* === /PANO TEMASI === */
body{margin:0;font-family:system-ui,sans-serif;color:var(--text)}
.bar{display:flex;justify-content:space-between;flex-wrap:wrap;gap:6px;padding:10px 16px;background:rgba(0,0,0,.55);border-bottom:2px solid #000;font-size:13px;color:var(--muted)}
.bar b{color:var(--text)}main{max-width:520px;margin:16px auto;padding:0 16px}
.card{padding:16px 18px 18px;margin-bottom:16px}.satir{display:flex;gap:8px;margin-top:12px}
.satir input{flex:1;padding:10px;border-radius:8px;font-size:15px}.btn{font-size:15px}
.etiket{display:inline-block;width:110px;color:var(--muted)}#nx{font-family:Consolas,monospace}
)CSS";

static const char ANA_HTML[] PROGMEM = R"HTML(<!doctype html><html lang=tr><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>NX Hub</title><link rel=stylesheet href=/pano.css>
<div class=bar><b>NX Hub</b><span>WiFi: <span id=wifi>-</span></span><span id=srm>-</span></div>
<main><div class=card><h3>Ekran</h3>
<div class=satir><button id=led class="btn btn-yesil" style="flex:1" onclick="ac(led.dataset.v=='1'?0:1)">b_led</button></div>
<div class=satir><input id=dt maxlength=30 placeholder="t_durum metni"><button class="btn btn-mavi" onclick="yaz()">Gonder</button></div>
<p><span class=etiket>Son metin</span><span id=dm>-</span><br><span class=etiket>Son komut</span><span id=nx>-</span></p>
</div><p><a href=/foto style="color:#9ca3af">Resim gonder / .tft yukle</a></p></main>
<script>
function g(d){led.dataset.v=d.led;led.classList.toggle('btn-on',d.led==1);led.textContent=d.led?'b_led: ACIK':'b_led: KAPALI';
dm.textContent=d.durum;nx.textContent=d.nx;wifi.textContent=d.ssid+' ('+d.ip+')';srm.textContent=d.surum+' • Son güncelleme: '+d.tarih}
function f(u){fetch(u).then(r=>r.json()).then(g).catch(()=>nx.textContent='baglanti yok')}
function ac(v){f('/api/led?v='+v)}function yaz(){f('/api/durum?t='+encodeURIComponent(dt.value))}
dt.onkeydown=e=>{if(e.key=='Enter')yaz()};f('/api/durum')
</script>)HTML";

// ---- /foto: tarayıcıdan gelen "fill" komut satırlarını ekrana iletir ----
// Akış denetimi: bkcmd=1 ile ekran her başarılı komuta 0x01 yollar; en fazla FOTO_WINDOW komut onaysız bekler.
constexpr uint32_t FOTO_BAUD = 115200;
constexpr uint16_t FOTO_WINDOW = 24;
static char fotoLine[80];
static uint8_t fotoLen = 0;
static uint32_t fotoSent = 0, fotoAcked = 0;
static bool fotoOk = false;

static void nxEnd() { Serial.write(0xFF); Serial.write(0xFF); Serial.write(0xFF); }

static bool fotoDrain(uint32_t maxInFlight, uint32_t ms) {
  uint32_t t = millis();
  for (;;) {
    while (Serial.available())
      if (Serial.read() == 0x01) fotoAcked++;
    if (fotoSent - fotoAcked <= maxInFlight) return true;
    if (millis() - t > ms) return false;
    yield();
  }
}

static void fotoSendLine() {
  if (!fotoLen) return;
  if (!fotoDrain(FOTO_WINDOW, 2000)) { fotoOk = false; return; }
  Serial.write((const uint8_t*)fotoLine, fotoLen);
  nxEnd();
  fotoSent++;
  fotoLen = 0;
}

void handleFotoUpload() {
  HTTPUpload& u = web.upload();
  if (u.status == UPLOAD_FILE_START) {
    fotoOk = true; fotoLen = 0; fotoSent = fotoAcked = 0;
    Serial.print("bkcmd=1"); nxEnd();
    Serial.printf("bauds=%u", FOTO_BAUD); nxEnd();
    Serial.flush();
    delay(100);
    Serial.begin(FOTO_BAUD);
    delay(50);
    while (Serial.available()) Serial.read();
  } else if (u.status == UPLOAD_FILE_WRITE && fotoOk) {
    for (size_t i = 0; i < u.currentSize && fotoOk; i++) {
      char ch = (char)u.buf[i];
      if (ch == '\n') fotoSendLine();
      else if (fotoLen < sizeof fotoLine - 1) fotoLine[fotoLen++] = ch;
      else fotoOk = false;  // beklenmedik uzun satır
    }
  } else if (u.status == UPLOAD_FILE_END || u.status == UPLOAD_FILE_ABORTED) {
    if (u.status == UPLOAD_FILE_ABORTED) fotoOk = false;
    if (fotoOk) fotoSendLine();
    fotoOk = fotoOk && fotoDrain(0, 5000) && u.status == UPLOAD_FILE_END;
    Serial.print("bkcmd=2"); nxEnd();
    Serial.print("bauds=9600"); nxEnd();
    Serial.flush();
    delay(100);
    Serial.begin(NX_BAUD);
  }
}

void handleFotoDone() {
  web.send(fotoOk ? 200 : 500, "text/plain",
           fotoOk ? String("tamam: ") + fotoSent + " komut" : String("HATA: ") + fotoAcked + "/" + fotoSent + " komut onaylandi (ESP-01 ve ekranin gucunu kes-ver)");
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
  web.on("/", [] { web.send_P(200, "text/html", ANA_HTML); });
  web.on("/pano.css", [] { web.send_P(200, "text/css", PANO_CSS); });
  web.on("/api/led", [] {
    ledAcik = web.arg("v") == "1";
    ledUygula();
    apiDurum();
  });
  web.on("/api/durum", [] {
    if (web.hasArg("t")) {
      durumTxt = nxMetin(web.arg("t"));
      nxSon = nxCmd("t_durum.txt=\"" + durumTxt + "\"");
    }
    apiDurum();
  });
  web.on("/api/wifi", [] {
    web.send(200, "application/json", "{\"ssid\":\"" + WiFi.SSID() + "\",\"ip\":\"" + WiFi.localIP().toString() + "\",\"rssi\":" + WiFi.RSSI() + "}");
  });
  web.on("/api/wifisil", [] {
    web.send(200, "text/plain", "tamam");
    delay(300);
    wm.resetSettings();  // kayıtlı ağı siler; açılışta NXHub-Kurulum AP'si açılır
    ESP.restart();
  });
  web.on("/tft", HTTP_GET, [] { web.sendHeader("Location", "/foto"); web.send(302); });
  web.on("/tft", HTTP_POST, handleTftDone, handleTftUpload);
  web.on("/foto", HTTP_GET, [] { web.send_P(200, "text/html", FOTO_HTML); });
  web.on("/foto", HTTP_POST, handleFotoDone, handleFotoUpload);
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
