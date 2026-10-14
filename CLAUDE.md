# ozcan-NX4832k035_011 — devir notu

Nextion **NX4832K035_011** (3,5" 480x320 seri HMI) ile, `C:\Kodlar\ozcan-crow-panel-7inc` (CrowPanel 7" HUB) projesinin aynı işlevlerini yapan ikinci cihaz. Kullanıcı Türkçe konuşur, kısa cevap ister, kanıtsız deneme önerme. Cihaz Özcan abiye verilecek (kendi WiFi'sine kodsuz bağlanabilmeli; gizli bilgiler teslimden önce silinir) — kurallar CrowPanel projesinin CLAUDE.md'sindeki "Teslim gereksinimi" ile aynı.

## Çalışma kuralları
- Önce kanıt/ölçüm, sonra kod. `docs/DONANIM.md`'de yalnızca fotoğraftan/ölçümden teyit edilen bilgi durur.
- Arayüz: koyu/gotik tema, siyah çerçeve, pastel renk; açık üstüne açık renk yok. Bağlantı göstergesi yeşil/kırmızı; alarm göstergesi kırmızı/sönük.
- Commit sonuna: `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`. `src/secrets.h` commit'e girmez.

## Mimari fark (CrowPanel'den)
Nextion kendi içinde çalışan akıllı ekran: sayfalar/bileşenler **Nextion Editor** (Windows) ile tasarlanıp `.tft` olarak ekrana yüklenir (TF kart ya da seri). WiFi yok → bir **ESP32** UART ile sürer (komut: metin + `FF FF FF`; olaylar aynı biçimde gelir). `.HMI`/`.tft` dosyasını ben üretemem; sayfa/bileşen adlarını `docs/NEXTION_TASARIM.md` belirler, kullanıcı Editor'de kurar (ya da birlikte). ESP32 tarafı (`src/nextion.*`) bileşen adlarına göre yazılır.

## Durum (2026-10-12)
- Fotoğraftan kart bilgisi çıkarıldı (`docs/DONANIM.md`). Kart henüz PC'ye bağlanmadı.
- `src/nextion.*`: protokol sürücüsü (belgeden, kartta denenmedi), `src/main.cpp`: olay yazdıran iskelet. `pio run` derleniyor (board `esp32dev` **geçici varsayım**).
- `araclar/nextion_probe.py`: ekran bağlanınca ilk iş — "connect" ile model/hız doğrulama.

## Sıradaki
1. Ekranı USB-TTL ile bağla → `python -I araclar/nextion_probe.py COMx` → gerçek model/hız/comok satırını DONANIM.md'ye yaz.
2. Ekranı hangi ESP32'nin süreceğini belirle (açık soru) → pin/board kesinleşir.
3. `docs/NEXTION_TASARIM.md` sayfalarını Editor'de kur → `.tft` yükle.
4. HUB mantığını (WiFi listesi, ESP01 yönetimi, hava, LedPanel, web arayüzü) CrowPanel projesinden taşı; WiFi kurulumu ekranda yapılacak (Nextion'un ekran klavyesi bileşeni ile).

## Açık sorular (kullanıcıya)
- Bu ekranı hangi ESP32 sürecek (mevcut hangi kart: ESP32-DevKit, S3, C3, ESP-01 …)? ESP-01'de yalnız 2 GPIO + UART var; ekranla UART'ı paylaşamaz.
- Ekran yatay mı dikey mi kullanılacak (Nextion'da yön tasarımda sabit; ikisi birden için ayrı sayfalar gerekir)?
