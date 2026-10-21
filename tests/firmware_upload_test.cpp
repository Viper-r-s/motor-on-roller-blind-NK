#include "../FirmwareUpload.h"
#include <assert.h>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <iterator>
struct Writer {
  bool running = false, committed = false, beginFails = false, writeFails = false, verifyFails = false;
  size_t expected = 0, begins = 0;
  std::vector<uint8_t> bytes;
  bool begin(size_t n) { ++begins; bytes.clear(); expected = n; return running = !beginFails; }
  size_t write(uint8_t* p, size_t n) {
    if (!running || writeFails || bytes.size() + n > expected) return 0;
    bytes.insert(bytes.end(), p, p+n); return n;
  }
  bool end(bool allowRemaining) {
    assert(!allowRemaining);
    committed = running && bytes.size() == expected && !verifyFails;
    running = false; return committed;
  }
};
int main(int argc, char** argv) {
  std::vector<uint8_t> image(1024, 0xAB);
  const uint8_t header[] = {0xE9,1,0,0x40,0xB8,0xF0,0x10,0x40};
  for (size_t i=0;i<8;++i) image[i]=header[i];
  assert(FirmwareUpload::validName("firmware.bin") && FirmwareUpload::validName("firmware.BIN"));
  assert(!FirmwareUpload::validName("f.bin.gz") && !FirmwareUpload::validName("f.zip"));
  assert(!FirmwareUpload::validSize(0,2048) && !FirmwareUpload::validSize(2049,2048));
  // Fragmented header and every possible upload chunk size must preserve every byte.
  for (size_t chunk : {size_t(1),size_t(3),size_t(8),size_t(97),size_t(1024)}) {
    Writer w; FirmwareUpload f; assert(f.begin(w,image.size(),2048));
    for (size_t pos=0;pos<image.size();) {
      size_t count=std::min(chunk,image.size()-pos);
      assert(f.write(w,image.data()+pos,count)); pos+=count;
    }
    assert(w.bytes.size()==image.size()-1 && !w.committed);
    assert(f.fileEnd(w,image.size())); assert(!w.committed);
    assert(f.finish(w) && w.committed && w.bytes==image);
  }
  {
    Writer w; FirmwareUpload f; assert(f.begin(w,image.size(),2048));
    assert(f.write(w,image.data(),image.size())); assert(f.fileEnd(w,image.size()));
    f.reject(w,400,"Disconnected after FILE_END"); assert(!w.committed && !w.running);
    assert(!f.finish(w));
  }
  {
    Writer w; FirmwareUpload f; assert(f.begin(w,image.size(),2048));
    assert(f.write(w,image.data(),100)); assert(!f.fileEnd(w,100)); assert(!w.committed && !w.running);
    assert(f.begin(w,image.size(),2048)); // Retry after an aborted upload.
    assert(f.write(w,image.data(),image.size())); assert(f.fileEnd(w,image.size()));
    assert(!f.begin(w,image.size(),2048)); assert(!w.committed && !w.running); // Multiple files.
  }
  {
    Writer w; FirmwareUpload f; assert(f.begin(w,100,2048));
    assert(!f.write(w,image.data(),101)); assert(!w.committed);
  }
  for (bool esp32 : {false,true}) {
    Writer w; FirmwareUpload f; auto bad=image;
    if (esp32) { bad[6]=8; bad[7]=0x40; } else bad[0]=0xFF;
    assert(f.begin(w,bad.size(),2048)); assert(!f.write(w,bad.data(),bad.size()));
    assert(w.bytes.empty() && !w.committed);
  }
  {
    Writer w; FirmwareUpload f; w.beginFails=true;
    assert(!f.begin(w,image.size(),2048));
    w.beginFails=false; assert(f.begin(w,image.size(),2048)); w.writeFails=true;
    assert(!f.write(w,image.data(),image.size())); assert(!w.committed);
  }
  {
    Writer w; FirmwareUpload f; assert(f.begin(w,image.size(),2048));
    assert(f.write(w,image.data(),image.size())); assert(f.fileEnd(w,image.size()));
    w.verifyFails=true; assert(!f.finish(w) && !w.committed);
  }
  if (argc > 1) {
    std::ifstream stream(argv[1], std::ios::binary);
    std::vector<uint8_t> actual((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    assert(actual.size() > 32 && FirmwareUpload::validHeader(actual.data()));
    Writer w; FirmwareUpload f; assert(f.begin(w, actual.size(), actual.size()));
    for (size_t pos=0;pos<actual.size();) {
      size_t count=std::min(size_t(1460),actual.size()-pos);
      assert(f.write(w,actual.data()+pos,count)); pos+=count;
    }
    assert(f.fileEnd(w,actual.size()) && !w.committed);
    assert(f.finish(w) && w.bytes==actual);
    puts("PASS: actual compiled firmware image accepted and streamed byte-identically (fake flash writer)");
  }
  puts("PASS: real streaming controller: fragmented headers/chunks, exact bytes, size limits, wrong images, partial/late abort, multi-file, retry, write/verify failures");
}
