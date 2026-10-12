# Donanım (yalnızca teyit edilenler)

Kaynak: `NX4832k035_011.jpeg` (kartın arka yüzü fotoğrafı, 2026-10-12). Kart henüz bilgisayara bağlanmadı; ölçüm/seri testi yok.

## Karttan okunanlar (serigrafi / etiket)
- Model baskısı: **NX4832K035 011** (Nextion 3,5" "K" serisi, 480x320). Kart revizyonu baskısı: **V2.0**
- Etiket (QR + metin): `DB646C17`, `07181D30` (seri/parti no olduğu varsayımı; doğrulanmadı)
- Seri başlık (alt kenar): **+5V, TX, RX, GND**. Fotoğrafta 4 telli kablo lehimli
- I/O başlığı (üst, FPC/ince kablo): **+5V, IO_7 … IO_0, GND** (RN2/RN3 direnç dizileri yanında)
- TF (microSD) yuvası (K1), CR1220 tipi yedek pil yuvası (RTC için), J4 yazılı boş lehim noktaları
- Dokunmatik türü: model sonundaki "_011" Nextion'da dirençli panel anlamına gelir (kullanıcı da "R-Rezistif" demişti); fotoğraftan doğrulanamaz

## Teyit bekleyen
- Seri hızı (Nextion varsayılanı 9600) ve gerçek model: `araclar/nextion_probe.py` ("connect" cevabı)
- İşlemci/flash/RAM (cevaptaki comok satırı verir)
- Kablonun renk sırası ve hangi USB-TTL/ESP32'ye bağlı olduğu
- Ekranı sürecek ESP32 kartı (bkz. CLAUDE.md, açık soru)
