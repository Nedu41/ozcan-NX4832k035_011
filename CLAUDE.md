# ozcan-NX4832k035_011 — devir notu

Nextion **NX4832K035_011** (3,5" 480x320 seri HMI) ile, `C:\Kodlar\ozcan-crow-panel-7inc` (CrowPanel 7" HUB) projesinin aynı işlevlerini yapan ikinci cihaz. Kullanıcı Türkçe konuşur, kısa cevap ister, kanıtsız deneme önerme. Cihaz Özcan abiye verilecek (kendi WiFi'sine kodsuz bağlanabilmeli; gizli bilgiler teslimden önce silinir) — kurallar CrowPanel projesinin CLAUDE.md'sindeki "Teslim gereksinimi" ile aynı.

## Çalışma kuralları
- Önce kanıt/ölçüm, sonra kod. `docs/DONANIM.md`'de yalnızca fotoğraftan/ölçümden teyit edilen bilgi durur.
- Arayüz: koyu/gotik tema, siyah çerçeve, pastel renk; açık üstüne açık renk yok. Bağlantı göstergesi yeşil/kırmızı; alarm göstergesi kırmızı/sönük.
- Commit sonuna: `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`. `src/secrets.h` commit'e girmez.

## Mimari fark (CrowPanel'den)
Nextion kendi içinde çalışan akıllı ekran: sayfalar/bileşenler **Nextion Editor** (Windows) ile tasarlanıp `.tft` olarak ekrana yüklenir (TF kart ya da seri). WiFi yok → bir **ESP32** UART ile sürer (komut: metin + `FF FF FF`; olaylar aynı biçimde gelir). `.HMI`/`.tft` dosyasını ben üretemem; sayfa/bileşen adlarını `docs/NEXTION_TASARIM.md` belirler, kullanıcı Editor'de kurar (ya da birlikte). ESP32 tarafı (`src/nextion.*`) bileşen adlarına göre yazılır.

## Durum (2026-10-16)
- Ekran teyit edildi: NX4832K035_011R, 9600 baud, 32 MB flash (`docs/DONANIM.md`).
- Sürücü kart **ESP-01 (ESP8266, 1 MB)** olarak kararlaştırıldı; ekrana çapraz TX/RX ile bağlı. WiFiManager + ArduinoOTA + Nextion sınama arayüzü `src/esp01_test.cpp` içinde çalışıyor (env `esp01`, OTA: `esp01_ota`).
- `src/nextion.*` ve `src/main.cpp` ESP32 içindi (env `esp32`, askıda); ESP8266'ya uyarlanacak.

## Sıradaki
1. `docs/NEXTION_TASARIM.md` sayfalarını Editor'de kur → `.tft` yükle (TF kart ya da seri).
2. `nextion.*` sürücüsünü ESP8266'ya uyarla (`Serial.begin(9600)`, UART0).
3. HUB mantığını (hava, LedPanel, web arayüzü) taşı; ESP-01 RAM'i ~40 KB boş, HTTPS/TLS ölçülerek karar verilecek.

## Açık sorular (kullanıcıya)
- Ekran yatay mı dikey mi kullanılacak (Nextion'da yön tasarımda sabit; ikisi birden için ayrı sayfalar gerekir)?
