# Donanım (yalnızca teyit edilenler)

Kaynak: `NX4832k035_011.jpeg` (kartın arka yüzü fotoğrafı, 2026-10-12). Kart henüz bilgisayara bağlanmadı; ölçüm/seri testi yok.

## Karttan okunanlar (serigrafi / etiket)
- Model baskısı: **NX4832K035 011** (Nextion 3,5" "K" serisi, 480x320). Kart revizyonu baskısı: **V2.0**
- Etiket (QR + metin): `DB646C17`, `07181D30` (seri/parti no olduğu varsayımı; doğrulanmadı)
- Seri başlık (alt kenar): **+5V, TX, RX, GND**. Fotoğrafta 4 telli kablo lehimli
- I/O başlığı (üst, FPC/ince kablo): **+5V, IO_7 … IO_0, GND** (RN2/RN3 direnç dizileri yanında)
- TF (microSD) yuvası (K1), CR1220 tipi yedek pil yuvası (RTC için), J4 yazılı boş lehim noktaları
- Dokunmatik türü: model sonundaki "_011" Nextion'da dirençli panel anlamına gelir (kullanıcı da "R-Rezistif" demişti); fotoğraftan doğrulanamaz

## Ölçülenler (2026-10-16, ESP-01 üzerinden "connect")
- Seri hızı **9600** (ilk denemede cevap verdi)
- Cevap: `comok 1,30614-0,NX4832K035_011R,183,61699,DB646C1707181D30,33554432`
  - Model **NX4832K035_011R** (R = dirençli dokunmatik), dokunmatik var
  - Firmware sürümü 183, MCU kodu 61699
  - Seri no `DB646C1707181D30` (etiketle aynı)
  - Flash 33554432 bayt = 32 MB

## Sürücü kart: ESP-01 (ESP8266EX, 1 MB flash)
- Çip 25Q80 (GigaDevice, 0xC8/0x4014) = 1 MB; kristal 26 MHz. Yeni modül MAC `fc:f5:c4:a8:0b:ac`
- Ekran bağlantısı çapraz: ESP-01 TX -> Nextion RX, ESP-01 RX -> Nextion TX, ortak GND. UART0 ekranda; log yok, güncelleme OTA (`pio run -e esp01_ota -t upload`, `nxhub.local`)
- GPIO0 yalnız flaş modu için GND'ye çekilir; adaptörde RTS yok, flaş/reset elle: `esptool --before no-reset --after no-reset`
- WiFi: kendi kurulum portalı (kayıtlı ağ yoksa `NXHub-Kurulum` AP'si, WiFiManager kullanılmıyor), tek ana sayfa `http://nxhub.local/` (kayan yazı mesajı/renk/hız/animasyon, WiFi ayarı, .tft yükle); tanı: `/probe`, `/nx?c=<komut>`

## Teyit bekleyen
- Kablonun renk sırası
- Ekranın yatay/dikey kullanımı
