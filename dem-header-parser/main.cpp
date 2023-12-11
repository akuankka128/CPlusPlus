#include <cstdio>   // `printf`
#include <fstream>  // `filebuf`
#include <cstdint>  // Fixed-width integers

#include "bstream.hpp"

#define INFO(...) do {\
  printf("[info] " __VA_ARGS__);\
  printf("\n");\
} while (0);

#define FAIL_QUIT(...) do {\
  printf("[error] " __VA_ARGS__);\
  printf("\n");\
  return 1;\
} while (0);

int main (int argc, char** argv) {
  using namespace std;

  if (argc < 2) {
    printf("usage: readdem <demo-file>\n");
    return 1;
  }

  INFO("--- %s ---", argv[1]);

  filebuf file_buffer;
  if (file_buffer.open(argv[1], ios::in) == nullptr) {
    FAIL_QUIT("could not open file %s", argv[1]);
  }

  bstream reader(&file_buffer);

  if (reader.expect_chars<8>("HL2DEMO\0")) {
    INFO("valid magic:  HL2DEMO");
  }

  INFO("demo protocol:  %d", reader.get<uint32_t>());
  INFO("net protocol:  %d", reader.get<uint32_t>());
  INFO("server name:  %s", reader.get<260>().get());
  INFO("client name:  %s", reader.get<260>().get());
  INFO("demo map name:  %s", reader.get<260>().get());
  INFO("game directory:  %s", reader.get<260>().get());
  INFO("playback seconds:  %f", reader.get<float>());
  INFO("number of ticks:  %d", reader.get<uint32_t>());
  INFO("number of frames:  %d", reader.get<uint32_t>());
  INFO("sign-on data length:  %d", reader.get<uint32_t>());
  return 0;
}
