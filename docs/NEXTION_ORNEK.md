# Nextion örnek sayfa (bağlantı denemesi)

Amaç: ekran + ESP-01 haberleşmesini doğrulamak. Tek sayfa, üç bileşen. Tam tasarım için `NEXTION_TASARIM.md`.

Proje: `ornek.HMI`, cihaz Basic NX4832K035 (011R), yön **Horizontal**, 480x320. Sayfa adı `ana` (id 0). Renkler `NEXTION_TASARIM.md`'deki palet: zemin `#14141A`, yazı `#DADAE4`, buton `#2C2C38`.

| Bileşen | Tür | `objname` | x, y, w, h | Ayar |
|---|---|---|---|---|
| Başlık | Text | `t_baslik` | 20, 20, 440, 50 | `txt`="NX Hub", ortalı, yazı `#7FE0DC` |
| Durum | Text | `t_durum` | 20, 100, 440, 50 | `txt`="bekleniyor", ortalı, yazı `#DADAE4`. ESP-01 `t_durum.txt="..."` ile yazar |
| Buton | Button | `b_led` | 140, 200, 200, 70 | `txt`="Dokun", zemin `#2C2C38` |

`b_led` davranışı: **Touch Press Event** ve **Touch Release Event** için "Send Component ID" işaretli. Ekran `65 00 02 01 FF FF FF` (basıldı) ve `65 00 02 00 FF FF FF` (bırakıldı) yollar (`02` = `b_led` bileşen id'si; Editor'de 3. eklenen bileşen olduğu için; sıra farklıysa bileşen id'sini not et). Ekranda başka mantık yok; ESP-01 olayı alınca `t_durum.txt` yazar.

Font: ASCII yeterli (Türkçe karakter yok), Font Generator ile 24 punto. Çıktı: `ornek.tft`, ESP-01 üzerinden kablosuz yüklenir: `http://nxhub.local:81/tft` (whmi-wri, 460800 baud). SD kart kullanılmaz.
