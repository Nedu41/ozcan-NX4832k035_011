// Saat (NTP), hava (Open-Meteo, düz HTTP) ve haber (RSS, HTTPS) verisi. Ekrana yazmayı esp01_test.cpp yapar.
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <time.h>

constexpr const char* NEWS_URL = "https://www.aa.com.tr/tr/rss/default?cat=dunya";  // CrowPanel ile aynı kaynak
constexpr size_t HABER_MAX = 240;  // g0.txt_maxl bundan büyük olmalı (Editor'de 250)
constexpr uint8_t HABER_BASLIK = 6;

// Şehir (EEPROM'da kalıcı); varsayılan İstanbul
struct SehirKayit { uint32_t magic; float lat, lon; char ad[32]; };
constexpr uint32_t SEHIR_MAGIC = 0x4E58484C;
static String sehirAd = "Istanbul";
static float sehirLat = 41.0138f, sehirLon = 28.9497f;

static bool havaVar = false;
static float havaC = 0;
static String havaDurum = "-", haberMetin = "", haberDurum = "-";
static bool haberYeni = false;

static void sehirYukle() {
  SehirKayit k;
  EEPROM.begin(64);
  EEPROM.get(0, k);
  EEPROM.end();
  if (k.magic == SEHIR_MAGIC && k.ad[0]) { k.ad[31] = 0; sehirAd = k.ad; sehirLat = k.lat; sehirLon = k.lon; }
}

static void sehirKaydet() {
  SehirKayit k = {SEHIR_MAGIC, sehirLat, sehirLon, {0}};
  strncpy(k.ad, sehirAd.c_str(), sizeof k.ad - 1);
  EEPROM.begin(64);
  EEPROM.put(0, k);
  EEPROM.commit();
  EEPROM.end();
}

static void veriBaslat() {
  sehirYukle();
  configTime(3 * 3600, 0, "pool.ntp.org", "time.google.com");  // Türkiye UTC+3 (yaz saati yok)
}

static bool saatHazir() { return time(nullptr) > 1700000000; }

static String saatStr() {
  time_t t = time(nullptr);
  struct tm* m = localtime(&t);
  char b[8];
  snprintf(b, sizeof b, "%02d:%02d", m->tm_hour, m->tm_min);
  return b;
}

static String urlEnc(const String& s) {
  String o;
  for (size_t i = 0; i < s.length(); i++) {
    uint8_t c = s[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.') o += (char)c;
    else { char h[4]; snprintf(h, sizeof h, "%%%02X", c); o += h; }
  }
  return o;
}

// Ekran fontu ASCII: Türkçe harfleri sadeleştir, tipografik işaretleri düzelt, geri kalan ASCII dışını at.
static String asciiMetin(String s) {
  static const char* tr[][2] = {{"ç","c"},{"Ç","C"},{"ğ","g"},{"Ğ","G"},{"ı","i"},{"İ","I"},{"ö","o"},{"Ö","O"},{"ş","s"},{"Ş","S"},{"ü","u"},{"Ü","U"},
                                {"’","'"},{"‘","'"},{"“","'"},{"”","'"},{"–","-"},{"—","-"},{"…","..."},{"&amp;","&"},{"&quot;","'"},{"&apos;","'"},
                                {"&#39;","'"},{"&#x27;","'"},{"&lt;","<"},{"&gt;",">"},{"&nbsp;"," "}};
  for (auto& p : tr) s.replace(p[0], p[1]);
  String o;
  for (size_t i = 0; i < s.length(); i++) {
    uint8_t c = s[i];
    if (c == '"' || c == '\\') continue;
    if (c >= 32 && c < 127) o += (char)c;
  }
  o.trim();
  return o;
}

static bool havaCek() {
  WiFiClient c;
  HTTPClient http;
  http.setTimeout(6000);
  String url = String("http://api.open-meteo.com/v1/forecast?latitude=") + String(sehirLat, 4) + "&longitude=" + String(sehirLon, 4) + "&current=temperature_2m";
  if (!http.begin(c, url)) { havaDurum = "begin basarisiz"; return false; }
  int code = http.GET();
  if (code != 200) { havaDurum = "HTTP " + String(code); http.end(); return false; }
  String p = http.getString();
  http.end();
  JsonDocument d;
  if (deserializeJson(d, p) || d["current"]["temperature_2m"].isNull()) { havaDurum = "yanit okunamadi"; return false; }
  havaC = d["current"]["temperature_2m"].as<float>();
  havaVar = true;
  havaDurum = "tamam";
  return true;
}

// Şehir adından koordinat bulur (Open-Meteo geocoding, düz HTTP) ve kalıcı kaydeder.
static bool sehirAra(const String& ad) {
  WiFiClient c;
  HTTPClient http;
  http.setTimeout(6000);
  if (!http.begin(c, "http://geocoding-api.open-meteo.com/v1/search?name=" + urlEnc(ad) + "&count=1&language=tr&format=json")) return false;
  int code = http.GET();
  if (code != 200) { http.end(); return false; }
  String p = http.getString();
  http.end();
  JsonDocument d;
  if (deserializeJson(d, p) || d["results"][0].isNull()) return false;
  sehirAd = asciiMetin(d["results"][0]["name"].as<String>());
  sehirLat = d["results"][0]["latitude"].as<float>();
  sehirLon = d["results"][0]["longitude"].as<float>();
  sehirKaydet();
  return true;
}

// RSS'ten ilk başlıkları akıştan okur (dosyanın tamamı RAM'e alınmaz); başlıkları "  ***  " ile birleştirir.
static bool haberCek() {
  BearSSL::WiFiClientSecure c;
  c.setInsecure();  // sertifika doğrulanmaz (yalnız herkese açık haber başlıkları okunuyor)
  c.setBufferSizes(16384, 512);  // sunucu TLS kaydı 16 KB olabilir; küçük tamponda bağlantı kopuyor (-5)
  HTTPClient http;
  http.setTimeout(8000);
  http.useHTTP10(true);
  http.setUserAgent("NXHub/1.0");
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  uint32_t heap0 = ESP.getFreeHeap();
  if (!http.begin(c, NEWS_URL)) { haberDurum = "begin basarisiz"; return false; }
  int code = http.GET();
  if (code != 200) {
    haberDurum = "HTTP " + String(code) + (code < 0 ? " " + http.errorToString(code) : "") + " (heap " + String(heap0) + ")";
    http.end();
    return false;
  }
  WiFiClient* s = http.getStreamPtr();
  String acc, out;
  uint8_t tmp[256];
  uint8_t sayi = 0;
  bool itemGordu = false;  // kanal başlığı <item>'dan önce gelir; yalnız haber başlıkları alınır
  uint32_t t0 = millis(), minHeap = ESP.getFreeHeap();
  while ((http.connected() || s->available()) && millis() - t0 < 15000 && sayi < HABER_BASLIK && out.length() < HABER_MAX) {
    int n = s->available();
    if (n <= 0) { delay(5); continue; }
    n = s->readBytes(tmp, n > (int)sizeof tmp ? sizeof tmp : n);
    acc.concat((const char*)tmp, n);
    minHeap = min<uint32_t>(minHeap, ESP.getFreeHeap());
    for (;;) {
      if (!itemGordu) {
        int i = acc.indexOf("<item>");
        if (i < 0) { if (acc.length() > 8) acc.remove(0, acc.length() - 8); break; }
        acc.remove(0, i);
        itemGordu = true;
      }
      int a = acc.indexOf("<title>");
      if (a < 0) { if (acc.length() > 8) acc.remove(0, acc.length() - 8); break; }
      int b = acc.indexOf("</title>", a);
      if (b < 0) { if (a > 0) acc.remove(0, a); if (acc.length() > 700) acc = ""; break; }
      String t = acc.substring(a + 7, b);
      acc.remove(0, b + 8);
      t.trim();
      if (t.startsWith("<![CDATA[")) t = t.substring(9, t.length() >= 12 ? t.length() - 3 : 9);
      t = asciiMetin(t);
      if (!t.length()) continue;
      out += (out.length() ? "  ***  " : "") + t;
      sayi++;
    }
  }
  http.end();
  if (!out.length()) { haberDurum = "baslik bulunamadi (heap en az " + String(minHeap) + ")"; return false; }
  if (out.length() > HABER_MAX) out = out.substring(0, HABER_MAX);
  if (out != haberMetin) { haberMetin = out; haberYeni = true; }
  haberDurum = "tamam, " + String(sayi) + " baslik, heap en az " + String(minHeap);
  return true;
}
