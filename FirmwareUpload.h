#ifndef FIRMWARE_UPLOAD_H
#define FIRMWARE_UPLOAD_H
#include <stdint.h>
#include <stddef.h>

// Streaming policy independent of Arduino: test with a fake writer, run with Update.
// Retain the last byte until the COMPLETE HTTP request has been accepted. Thus an
// aborted request (even after FILE_END) cannot accidentally finalize via end(false).
class FirmwareUpload {
 public:
  enum State { Idle, Receiving, Received, Failed, Complete };
  State state = Idle;
  size_t expected = 0, received = 0;
  int errorCode = 400;
  const char* error = "Keine Firmware empfangen.";

  bool busy() const { return state == Receiving || state == Received || state == Complete; }
  static bool validSize(size_t size, size_t maximum) { return size >= 32 && size <= maximum; }
  static bool validName(const char* name) {
    if (!name) return false;
    size_t n = 0; while (name[n]) ++n;
    return n > 4 && name[n-4] == '.' && (name[n-3] == 'b' || name[n-3] == 'B') &&
      (name[n-2] == 'i' || name[n-2] == 'I') && (name[n-1] == 'n' || name[n-1] == 'N');
  }
  static bool validHeader(const uint8_t* h) {
    uint32_t entry = uint32_t(h[4]) | (uint32_t(h[5]) << 8) | (uint32_t(h[6]) << 16) | (uint32_t(h[7]) << 24);
    return h[0] == 0xE9 && h[1] > 0 && h[1] <= 16 && entry >= 0x40100000UL && entry < 0x40110000UL;
  }
  template<class Writer> bool reject(Writer& writer, int code, const char* reason) {
    if (state == Complete) return false;
    if (state == Receiving || state == Received) writer.end(false); // Last byte is still withheld.
    state = Failed; errorCode = code; error = reason;
    return false;
  }
  template<class Writer> bool begin(Writer& writer, size_t size, size_t maximum) {
    if (busy()) return reject(writer, 409, "Nur eine Firmware-Datei pro Anfrage erlaubt.");
    expected = size; received = 0; headerSize = 0; hasTail = false;
    state = Idle;
    if (!validSize(size, maximum)) return reject(writer, 413, "Firmware leer, zu klein oder groesser als der freie Update-Speicher.");
    if (!writer.begin(size)) return reject(writer, 500, "Firmware-Update konnte nicht gestartet werden.");
    state = Receiving; error = "";
    return true;
  }
  template<class Writer> bool write(Writer& writer, uint8_t* data, size_t length) {
    if (state != Receiving) return false;
    if (length > expected - received) return reject(writer, 400, "Mehr Firmware-Daten als angekuendigt empfangen.");
    received += length;
    size_t offset = 0;
    if (headerSize < sizeof(header)) {
      while (headerSize < sizeof(header) && offset < length) header[headerSize++] = data[offset++];
      if (headerSize < sizeof(header)) return true;
      if (!validHeader(header)) return reject(writer, 400, "Keine unkomprimierte ESP8266-Firmware (.bin). Keine ZIP-/Dateisystem-/ESP32-Datei verwenden.");
      if (!stage(writer, header, sizeof(header))) return false;
    }
    return stage(writer, data + offset, length - offset);
  }
  template<class Writer> bool fileEnd(Writer& writer, size_t total) {
    if (state != Receiving) return false;
    if (received != expected || total != expected || !hasTail) return reject(writer, 400, "Firmware unvollstaendig empfangen.");
    state = Received;
    return true;
  }
  template<class Writer> bool finish(Writer& writer) {
    if (state != Received) return false;
    if (writer.write(&tail, 1) != 1) return reject(writer, 500, "Letztes Firmware-Byte konnte nicht geschrieben werden.");
    // No further HTTP/form validation after this point: end() may arm eboot.
    if (!writer.end(false)) { state = Failed; errorCode = 400; error = "Firmware-Pruefung fehlgeschlagen."; return false; }
    state = Complete;
    return true;
  }
 private:
  uint8_t header[8] = {}, tail = 0;
  size_t headerSize = 0;
  bool hasTail = false;
  template<class Writer> bool stage(Writer& writer, uint8_t* data, size_t length) {
    if (!length) return true;
    if (hasTail && writer.write(&tail, 1) != 1) return reject(writer, 500, "Flash-Schreiben fehlgeschlagen.");
    if (length > 1 && writer.write(data, length - 1) != length - 1) return reject(writer, 500, "Flash-Schreiben fehlgeschlagen.");
    tail = data[length - 1]; hasTail = true;
    return true;
  }
};
#endif
