# ozcan-NX4832k035_011 — devir notu

Nextion **NX4832K035_011** (3,5" 480x320 seri HMI) ile, `C:\Kodlar\ozcan-crow-panel-7inc` (CrowPanel 7" HUB) projesinin aynı işlevlerini yapan ikinci cihaz. Kullanıcı Türkçe konuşur, kısa cevap ister, kanıtsız deneme önerme. Cihaz Özcan abiye verilecek (kendi WiFi'sine kodsuz bağlanabilmeli; gizli bilgiler teslimden önce silinir) — kurallar CrowPanel projesinin CLAUDE.md'sindeki "Teslim gereksinimi" ile aynı.

## Çalışma kuralları
- Önce kanıt/ölçüm, sonra kod. `docs/DONANIM.md`'de yalnızca fotoğraftan/ölçümden teyit edilen bilgi durur.
- Arayüz: koyu/gotik tema, siyah çerçeve, pastel renk; açık üstüne açık renk yok. Bağlantı göstergesi yeşil/kırmızı; alarm göstergesi kırmızı/sönük.
- Commit sonuna: `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`. `src/secrets.h` commit'e girmez.

## Mimari fark (CrowPanel'den)
Nextion kendi içinde çalışan akıllı ekran: sayfalar/bileşenler **Nextion Editor** (Windows) ile tasarlanıp `.tft` olarak ekrana yüklenir (TF kart ya da seri). WiFi yok → bir **ESP32** UART ile sürer (komut: metin + `FF FF FF`; olaylar aynı biçimde gelir). `.HMI`/`.tft` dosyasını ben üretemem; sayfa/bileşen adlarını `docs/NEXTION_TASARIM.md` belirler, kullanıcı Editor'de kurar (ya da birlikte). ESP32 tarafı (`src/nextion.*`) bileşen adlarına göre yazılır.

## Durum (2026-10-16)
- Ekran NX4832K035_011R (Enhanced), 9600 baud; sürücü kart **ESP-01 (ESP8266, 1 MB)**, çapraz TX/RX, UART0 ekranda (log yok). Ayrıntı: `docs/DONANIM.md`.
- Yazılım `src/esp01_test.cpp` + `src/veri.h` + `src/arayuz.h` (env `esp01`; OTA: `pio run -e esp01_ota -t upload`, `nxhub.local`; ilk flaş/kurtarma: GPIO0'ı GND'ye çekip `esptool --before no-reset`). ESP32 dosyaları (`nextion.*`, `main.cpp`) askıda.
- Çalışan: tek ana sayfa `http://nxhub.local/` (kayan yazı g1 mesajı/renk paleti/animasyon/hız, g1 ve sayfa arka plan rengi, karakter kümesi, WiFi ayarı, .tft yükleme), saat (NTP) `t_baslik`, sıcaklık `n0` (Open-Meteo, şehir web'den), haber `g0` (AA RSS, HTTPS/BearSSL, 6 başlık dönüşümlü), `t_durum` = IP+sürüm, `b_led` = WiFi göstergesi. Ekran tasarımı: `docs/NEXTION_ORNEK.md`.
- **Flash/OTA sınırı:** bölümleme `eagle.flash.1m.ld`, imaj ~468 KB, kalıcı OTA sınırı ~500 KB; büyütmeden önce `docs/DONANIM.md`'deki sınıra bak.
- Kayar yazı: `tim` en az 80 ms (ekran sınırı), hız `dis` ile ayarlanır; adım büyüdükçe titrer.
- **Denenmedi:** `NXHub-Kurulum` AP/captive portal akışı (başka ağa götürmeden önce telefonla denenmeli), web'den "Ağ değiştir".
- Bekleyen (Editor): `°C` etiketi 8859-9 fontuyla (font 1), ekran 180° yön (Device → Display → Direction), Karakter menüsünü ISO-8859-9 yapmak.

## Sıradaki
1. Kurulum AP akışını telefonla dene; dönüşten sonra teslim hazırlığı (gizli bilgi yok: WiFi/şehir ayarı cihazda, `src/secrets.h` kullanılmıyor).
2. Kayar yazı akıcılık/hız tercihi (bkz. yukarı).
3. HUB'dan kalanlar (LedPanel, röle/cihaz yönetimi) ihtiyaç olursa; flash payı dar, RAM ~6 KB TLS sırasında.

## Açık sorular (kullanıcıya)
- Ekran yatay mı dikey mi kullanılacak (Nextion'da yön tasarımda sabit; ikisi birden için ayrı sayfalar gerekir)?
