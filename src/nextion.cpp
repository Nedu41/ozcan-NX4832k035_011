#include "nextion.h"

void Nextion::begin(HardwareSerial& s, int rx, int tx, uint32_t baud) {
  ser = &s;
  ser->begin(baud, SERIAL_8N1, rx, tx);
  cmd("");  // yarım kalmış komut varsa temizle
}

void Nextion::cmd(const String& c) {
  if (!ser) return;
  ser->print(c);
  ser->write(0xFF);
  ser->write(0xFF);
  ser->write(0xFF);
}

void Nextion::setText(const char* obj, const String& txt) {
  String t = txt;
  t.replace("\\", "\\\\");
  t.replace("\"", "\\\"");
  cmd(String(obj) + ".txt=\"" + t + "\"");
}

void Nextion::setVal(const char* obj, int32_t v) { cmd(String(obj) + ".val=" + String(v)); }
void Nextion::setPage(uint8_t p) { cmd("page " + String(p)); }

bool Nextion::poll(NxEvent& ev) {
  while (ser && ser->available()) {
    uint8_t b = ser->read();
    if (b == 0xFF) {
      if (++ffCount < 3) continue;
      // 3 x 0xFF: olay tamam (n = olay kodu + veri)
      ffCount = 0;
      uint8_t len = n;
      n = 0;
      if (len == 0) continue;
      ev = NxEvent();
      ev.code = buf[0];
      switch (buf[0]) {
        case 0x65:  // touch: 65 page id event
          if (len >= 4) { ev.type = NxEvent::TOUCH; ev.page = buf[1]; ev.id = buf[2]; ev.press = buf[3] == 1; return true; }
          break;
        case 0x66: ev.type = NxEvent::PAGE; ev.page = buf[1]; return true;
        case 0x70: ev.type = NxEvent::STRING; for (uint8_t i = 1; i < len; i++) ev.str += (char)buf[i]; return true;
        case 0x71:
          if (len >= 5) { ev.type = NxEvent::NUMBER; ev.num = (int32_t)(buf[1] | (buf[2] << 8) | (buf[3] << 16) | ((uint32_t)buf[4] << 24)); return true; }
          break;
        case 0x88: ev.type = NxEvent::READY; return true;   // ekran hazır
        case 0x00: case 0x01: ev.type = NxEvent::OTHER; return true;  // 0x01 = komut başarılı, 0x00 = geçersiz komut
        default: ev.type = NxEvent::OTHER; return true;
      }
      continue;
    }
    // 0xFF dışı bayt: önce bekleyen kısmi 0xFF'leri değil, yalnızca veriyi biriktir
    ffCount = 0;
    if (n < sizeof(buf)) buf[n++] = b;
  }
  return false;
}
