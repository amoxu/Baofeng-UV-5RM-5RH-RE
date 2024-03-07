#include "encrypt.h"

void encrypt_buf(uint8_t *data, const char *key, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t byte = data[i];
    if (byte != 0 && byte != 0xFF && byte != key[i % 4] &&
        byte != (key[i % 4] ^ 0xFF)) {
      data[i] ^= key[i % 4];
    }
  }
}

void encrypt_pkg(uint8_t *pkg, size_t pkg_idx, size_t pkg_size) {
  if (pkg_idx % 3 == 1) {
    encrypt_buf(pkg, XOR_KEY1, pkg_size);
  } else if (pkg_idx % 3 == 2) {
    encrypt_buf(pkg, XOR_KEY2, pkg_size);
  }
}