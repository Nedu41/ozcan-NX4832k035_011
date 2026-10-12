// Nextion seri protokolü: komutlar 3 x 0xFF ile biter; ekrandan olaylar da aynı sonlandırıcıyla gelir.
// Kaynak: Nextion Instruction Set / Return Data (belgeden; kartta henüz denenmedi).
#pragma once
#include <Arduino.h>

struct NxEvent {
  enum Type : uint8_t { NONE, TOUCH, PAGE, STRING, NUMBER, STARTUP, READY, OTHER } type = NONE;
  uint8_t page = 0, id = 0;  // TOUCH: sayfa, bileşen id
  bool press = false;        // TOUCH: true = basıldı, false = bırakıldı
  String str;                // STRING (0x70)
  int32_t num = 0;           // NUMBER (0x71)
  uint8_t code = 0;          // ham olay kodu (OTHER dahil)
};

class Nextion {
 public:
  void begin(HardwareSerial& s, int rx, int tx, uint32_t baud);
  void cmd(const String& c);                           // örn "page 1"
  void setText(const char* obj, const String& txt);    // obj.txt="..."
  void setVal(const char* obj, int32_t v);             // obj.val=v
  void setPage(uint8_t p);                             // page p
  bool poll(NxEvent& ev);                              // tam bir olay okunduysa true (bloklamaz)

 private:
  HardwareSerial* ser = nullptr;
  uint8_t buf[96];
  uint8_t n = 0, ffCount = 0;
};
