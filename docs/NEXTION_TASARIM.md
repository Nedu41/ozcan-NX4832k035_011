# Nextion sayfa tasarımı (taslak — Editor'de kurulacak)

Ekran 480x320. Renkler (CrowPanel ile aynı): zemin `#14141A`, kart `#1F1F28`, düğme `#2C2C38`, çerçeve siyah, yazı `#DADAE4`, soluk `#9A9AAE`, vurgu camgöbeği `#7FE0DC`, turuncu `#FFB27A`, yeşil `#8FE0A0`, kırmızı `#FF9A9A`. Pastel zemin üstünde koyu yazı (`#0E0E14`). Yazı tipi: Türkçe karakterli (ğ ş ı İ ö ü ç) font Editor'de "Font Creator" ile üretilmeli (Noto Sans önerilir); doğrulanmadı.

Bileşen adları ESP32 koduna sabit yazılacağı için bu liste kesindir; Editor'de **aynen** kullan.

| Sayfa (id) | Ad | İçerik |
|---|---|---|
| 0 | `ana` | `t_saat`, `t_tarih`, `t_hava` (durum+sıcaklık), `t_sehir`, `t_haber`; alt gezinme: `b_ana`, `b_lamba`, `b_led`, `b_ayar` (her sayfada aynı) |
| 1 | `lamba` | 6 yuva: `t_n0..t_n5` (cihaz adı), `b_r0a`, `b_r0b` … `b_r5a`, `b_r5b` (röle 1/2, aç-kapa), `t_d0..t_d5` (bağlı/değil) |
| 2 | `led` | `b_fx0..b_fx12` (13 efekt), `h_hiz` (10..300), `h_isik` (4..30), `b_yon`, `t_led` (bağlantı durumu) |
| 3 | `ayar` | `t_bilgi` (IP, SSID, sürüm), `b_wifi`, `b_yeniden` |
| 4 | `wifi` | `b_ag0..b_ag5` (ağ listesi, `t_ag0..t_ag5` adlar), `b_yenile`, `b_geri` |
| 5 | `sifre` | Nextion klavye bileşeni + `t_ssid`, `t_sifre`, `b_baglan`, `b_vazgec` |

Durum bildirimi: ESP32 `t_x.txt="..."`, `b_x.val=0/1`, `h_x.val=n`, düğme rengi `b_x.bco=...` ile günceller. Dokunma olayını ekran `65 page id 01/00 FF FF FF` olarak yollar (Editor'de bileşenin "Send Component ID" seçeneği açık olmalı).

Açık: yatay/dikey. Nextion'da yön sabittir; dikey kullanım için ayrı sayfa kümesi gerekir (karar bekliyor, CLAUDE.md).
