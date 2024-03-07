#include <stddef.h>
#include <stdint.h>


#define PACKAGE_SIZE 1024 // 1KB

#define XOR_KEY1 "KDHT"
#define XOR_KEY2 "RBGI"

void encrypt_buf(uint8_t *data, const char *key, size_t len);
void encrypt_pkg(uint8_t *pkg, size_t pkg_idx, size_t pkg_size);