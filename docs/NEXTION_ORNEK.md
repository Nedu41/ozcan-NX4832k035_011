# Nextion örnek sayfa (bağlantı denemesi)

Amaç: ekran + ESP-01 haberleşmesini doğrulamak. Tek sayfa, üç bileşen. Tam tasarım için `NEXTION_TASARIM.md`.

Proje: `ornek.HMI`, cihaz **Enhanced** → NX4832K035_011 (K = Enhanced serisi; Basic seçilirse ekran "Model does not match" verir), yön **Horizontal**, 480x320. Sayfa adı `ana` (id 0). Renkler `NEXTION_TASARIM.md`'deki palet: zemin `#14141A`, yazı `#DADAE4`, buton `#2C2C38`.

| Bileşen | Tür | `objname` | x, y, w, h | Ayar |
|---|---|---|---|---|
| Başlık | Text | `t_baslik` | 20, 20, 440, 50 | `txt`="NX Hub", ortalı, yazı `#7FE0DC` |
| Durum | Text | `t_durum` | 20, 100, 440, 50 | `txt`="bekleniyor", ortalı, yazı `#DADAE4`. ESP-01 `t_durum.txt="..."` ile yazar |
| Buton | Button | `b_led` | 140, 200, 200, 70 | `txt`="Dokun", zemin `#2C2C38` |

Dokunmatik kullanılmaz (ekran camın arkasında). Kontrol web'den: `http://nxhub.local/`. `b_led` **WiFi göstergesi**: bağlıyken `bco=36628` (`#8FE0A0`, yeşil), `pco=2146`, `txt="WiFi"`; bağlı değilken `bco=64723` (`#FF9A9A`, kırmızı), `txt="YOK"`. Durum değişince ya da 30 sn'de bir yazılır. `t_durum.txt` varsayılan olarak `IP sürüm` yazar (ör. `192.168.1.144 v0.4.0`, WiFi yoksa `WiFi yok v0.4.0`); web'den girilen metin varsa onu gösterir, boş gönderilirse varsayılana döner (en çok 30 karakter, Türkçe harfler ASCII'ye çevrilir). `t_durum` için `txt_maxl` ≥ 30, `b_led` için ≥ 6 olmalı.

Font: ASCII yeterli (Türkçe karakter yok), Font Generator ile 24 punto. Çıktı: `ornek.tft`, ESP-01 üzerinden kablosuz yüklenir: `http://nxhub.local/ (".tft yukle" bölümü)` (whmi-wri, 115200 baud). SD kart kullanılmaz.

## Saat, sıcaklık, haber (v0.5.0)
| Bileşen | Tür | Ne yazılır |
|---|---|---|
| `t_baslik` | Text | saat `SS:DD` (NTP, UTC+3), dakikada bir |
| `n0` | Number | dış sıcaklık, tam sayı °C (Open-Meteo, 15 dk'da bir; şehir web'den ayarlanır, varsayılan İstanbul) |
| `g0` | Kayan yazı | AA RSS ilk başlıklar `  ***  ` ile birleşik, ASCII (10 dk'da bir). **`txt_maxl` 100** (varsayılan 10, metin kesilir); başlıklar sığdığı kadar birleştirilir, kayma bitince (süre hesaplanır) sıradaki gelir |

Number yalnız tam sayı gösterir, saat içine `:` konamaz; bu yüzden saat `t_baslik`'ta, sıcaklık `n0`'da.
