"""Nextion ekranı yoklar: ortak hızlarda "connect" gönderir, "comok ..." cevabını yazar.
Kullanım:  python -I araclar/nextion_probe.py COM5
Cevap biçimi: comok <dokunmatik>,<ters>-<ters>,<model>,<fw>,<mcu>,<seri>,<flash>
Yalnızca okur; ekrana kalıcı bir şey yazmaz."""
import sys
import time

import serial

TERM = b"\xff\xff\xff"
BAUDS = [9600, 115200, 57600, 38400, 19200, 4800, 2400, 230400]


def probe(port: str):
    for baud in BAUDS:
        try:
            s = serial.Serial(port, baud, timeout=0.4)
        except serial.SerialException as e:
            print(f"{port} açılamadı: {e}")
            return
        s.reset_input_buffer()
        s.write(TERM)           # önceki yarım komutu temizle
        s.write(b"connect" + TERM)
        time.sleep(0.3)
        data = s.read(200)
        s.close()
        if b"comok" in data:
            txt = data.split(TERM)[0].decode("latin-1", "replace")
            print(f"HIZ {baud}: {txt}")
            return
        print(f"hız {baud}: cevap yok" + (f" (ham: {data!r})" if data else ""))
    print("Hiçbir hızda 'comok' gelmedi: TX/RX ters, 5V/GND eksik ya da ekran bu kabloda değil.")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    probe(sys.argv[1])
