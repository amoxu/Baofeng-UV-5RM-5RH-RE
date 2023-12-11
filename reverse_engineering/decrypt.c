#include <stdio.h>
#include <stdlib.h>

#define PACKAGE_SIZE 1024 // 1KB
#define XOR_KEY1 "KDHT"
#define XOR_KEY2 "RBGI"

void xor_decrypt(char *data, char *key, int len) {
  for (int i = 0; i < len; i++) {
    char byte = data[i];
    if (byte != 0 && byte != 0xFF && byte != key[i % 4] &&
        byte != (key[i % 4] ^ 0xFF)) {
      data[i] ^= key[i % 4];
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <input file> <output file>\n", argv[0]);
    return 1;
  }

  FILE *input = fopen(argv[1], "rb");
  FILE *output = fopen(argv[2], "wb");

  if (!input || !output) {
    printf("Error opening file.\n");
    return 1;
  }

  fseek(input, 0, SEEK_END);
  long file_size = ftell(input);
  rewind(input);

  int package_count = file_size / PACKAGE_SIZE;
  int last_package_size = file_size % PACKAGE_SIZE;
  if (last_package_size > 0) {
    package_count++;
  }

  char *buffer = (char *)malloc(PACKAGE_SIZE);

  for (int i = 0; i < package_count; i++) {
    int current_package_size = (i == package_count - 1 && last_package_size > 0)
                                   ? last_package_size
                                   : PACKAGE_SIZE;
    fread(buffer, current_package_size, 1, input);

    if (i >= 2 && i < package_count - 2) {
      if (i % 3 == 1) {
        xor_decrypt(buffer, XOR_KEY1, current_package_size);
      } else if (i % 3 == 2) {
        xor_decrypt(buffer, XOR_KEY2, current_package_size);
      }
    }

    fwrite(buffer, current_package_size, 1, output);
  }

  free(buffer);
  fclose(input);
  fclose(output);

  return 0;
}