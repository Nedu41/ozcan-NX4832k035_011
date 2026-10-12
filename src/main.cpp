// Nextion NX4832K035_011 sürücü iskeleti. HUB mantığı (WiFi, ESP01, hava, LedPanel) CrowPanel projesinden
// taşınacak (bkz. CLAUDE.md). Şimdilik: ekrandan gelen olayları seriye yazar, açılışta "page 0" ister.
#include <Arduino.h>

#include "nextion.h"

// ESP32 <-> Nextion UART (kart teyit edilince kesinleşir; docs/DONANIM.md)
constexpr int NX_RX = 16, NX_TX = 17;
constexpr uint32_t NX_BAUD = 9600;  // Nextion varsayılanı; araclar/nextion_probe.py ile doğrula

Nextion nx;

void setup() {
  Serial.begin(115200);
  nx.begin(Serial2, NX_RX, NX_TX, NX_BAUD);
  nx.setPage(0);
  Serial.println("Nextion iskeleti basladi");
}

void loop() {
  NxEvent ev;
  while (nx.poll(ev)) {
    switch (ev.type) {
      case NxEvent::TOUCH: Serial.printf("dokunma sayfa=%u id=%u %s\n", ev.page, ev.id, ev.press ? "basildi" : "birakildi"); break;
      case NxEvent::PAGE: Serial.printf("sayfa %u\n", ev.page); break;
      case NxEvent::STRING: Serial.printf("metin: %s\n", ev.str.c_str()); break;
      case NxEvent::NUMBER: Serial.printf("sayi: %d\n", ev.num); break;
      case NxEvent::READY: Serial.println("ekran hazir"); break;
      default: Serial.printf("olay kodu 0x%02X\n", ev.code); break;
    }
  }
  delay(2);
}
