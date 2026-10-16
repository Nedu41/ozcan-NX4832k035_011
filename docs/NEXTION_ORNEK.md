# Nextion örnek sayfa (bağlantı denemesi)

Amaç: ekran + ESP-01 haberleşmesini doğrulamak. Tek sayfa, üç bileşen. Tam tasarım için `NEXTION_TASARIM.md`.

Proje: `ornek.HMI`, cihaz Basic NX4832K035 (011R), yön **Horizontal**, 480x320. Sayfa adı `ana` (id 0). Renkler `NEXTION_TASARIM.md`'deki palet: zemin `#14141A`, yazı `#DADAE4`, buton `#2C2C38`.

| Bileşen | Tür | `objname` | x, y, w, h | Ayar |
|---|---|---|---|---|
| Başlık | Text | `t_baslik` | 20, 20, 440, 50 | `txt`="NX Hub", ortalı, yazı `#7FE0DC` |
| Durum | Text | `t_durum` | 20, 100, 440, 50 | `txt`="bekleniyor", ortalı, yazı `#DADAE4`. ESP-01 `t_durum.txt="..."` ile yazar |
| Buton | Button | `b_led` | 140, 200, 200, 70 | `txt`="Dokun", zemin `#2C2C38` |

Dokunmatik kullanılmaz (ekran camın arkasında). Kontrol web'den: `http://nxhub.local:81/`. `b_led` gösterge olarak çalışır. ESP-01 açıkken `bco=36628` (`#8FE0A0`), `pco=2146`, `txt="ACIK"` yazar. Kapalıyken `bco=10599`, `pco=57052`, `txt="KAPALI"` yazar. `t_durum.txt` web'den girilen metni gösterir (en çok 30 karakter, Türkçe harfler ASCII'ye çevrilir). `t_durum` için `txt_maxl` ≥ 30, `b_led` için ≥ 6 olmalı.

Font: ASCII yeterli (Türkçe karakter yok), Font Generator ile 24 punto. Çıktı: `ornek.tft`, ESP-01 üzerinden kablosuz yüklenir: `http://nxhub.local:81/tft` (whmi-wri, 460800 baud). SD kart kullanılmaz.
