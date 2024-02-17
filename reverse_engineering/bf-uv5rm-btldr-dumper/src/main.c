#include "at32_clock.h"
#include "console.h"
#include "sys.h"

/*
 * Device Flash Size Register, KB as unit
 */
#define DEV_FLASH_SIZE (REG32(0x1FFFF7E0U))
/*
 * Device Type Register
 * 2-403,4-413,5-415,7-403A,
 * 8-407,9-421,D-435,E-437
 */
#define DEV_DEV_TYPE (REG32(0x1FFFF7F3U) & 0xFF)

#define FLASH_READ_BUF_SIZE 256
volatile uint32_t FLASH_READ_BUF[FLASH_READ_BUF_SIZE] = {0};
volatile uint32_t tick;

void dumpflash(uint32_t addr, uint32_t len);
void flash_read_words(uint32_t addr, uint32_t *data, uint32_t len);
void hex_dump(void *data, int len, int startAddress);

int main(void) {
  console_init(115200);
  for (size_t i = 0; i < 32; i++) {
    print("*");
  }
  print("\r\nFlash Dumper by Amo BD4VOW\r\n");
  for (size_t i = 0; i < 32; i++) {
    print("*");
  }
  print("\r\n");

  print("*** BOOTLOADER ***\r\n");
  dumpflash(FLASH_BASE, 0x1000);

  print("*** USER SYSTEM DATA ***\r\n");
  dumpflash(USD_BASE, 0x200);

  print("*** SYS BOOTLOADER ***\r\n");
  dumpflash(0x1FFFE400, 0x1000);

  while (true) {
  }
}

void dumpflash(uint32_t addr, uint32_t len) {
  // /* Address out of range. */
  // if (addr > (DEV_FLASH_SIZE * 1024) ||
  //     (addr + len) > (DEV_FLASH_SIZE * 1024)) {
  //   return;
  // }

  /* Address is not 4 bytes aligned, will start from (addr - addr % 4) */
  if (addr % 4 != 0) {
    addr = addr - addr % 4;
    len = len + addr % 4;
  }

  /* Length is not 4 bytes aligned, will read (4 * ((len / 4) + 1))) bytes. */
  if (len % 4 != 0) {
    len = 4 * ((len / 4) + 1);
  }

  uint32_t *readBuf = (uint32_t *)FLASH_READ_BUF;

  /* First Page Read */
  uint32_t pageEnd = (addr & 0xFFFFFF00) + FLASH_READ_BUF_SIZE;
  uint32_t pageLen = pageEnd - addr;
  if (pageLen > len) {
    pageLen = len;
  }
  flash_read_words(addr, readBuf, pageLen / 4);
  hex_dump(readBuf, pageLen, addr);

  uint32_t leftLen = (len - pageLen);

  /* Batch Read left pages if needed */
  while (leftLen > 0) {
    addr += pageLen;
    if (leftLen > FLASH_READ_BUF_SIZE) {
      pageLen = FLASH_READ_BUF_SIZE;
    } else {
      pageLen = leftLen;
    }
    flash_read_words(addr, readBuf, pageLen / 4);
    hex_dump(readBuf, pageLen, addr);

    leftLen -= pageLen;
  }
}

void flash_read_words(uint32_t addr, uint32_t *data, uint32_t len) {
  uint8_t i;
  for (i = 0; i < len; i++) {
    data[i] = *(__IO int32_t *)addr;
    addr += 4;
  }
}

static char *itohexa_helper(char *dest, unsigned x, int padding) {
  if (--padding)
    dest = itohexa_helper(dest, x >> 4, padding); // recursion
  *dest++ = "0123456789ABCDEF"[x & 0xF];          // mask and offset
  return dest;
}

char *itohexa(char *dest, unsigned x, int padding) {
  *itohexa_helper(dest, x, padding) = '\0';
  return dest;
}

/* Print memory data as hex aligned by 16 byte with line address */
void hex_dump(void *data, int len, int startAddress) {
  char array[8];
  int startAddressAlign = startAddress & 0xFFFFFFF0;
  int currentAddress = startAddressAlign;
  bool firstLine = true;

  for (size_t i = 0; i < len; i++) {
    if (currentAddress % 16 == 0) {
      print("0x");
      print(itohexa(array, currentAddress, 8));

      if (firstLine) {
        firstLine = false;
        for (size_t j = 0; j < (startAddress & 0x0F); j++) {
          print("   ");
          currentAddress++;
        }
      }
    }
    print(" ");
    print(itohexa(array, ((uint8_t *)data)[i], 2));

    currentAddress++;
    if (currentAddress % 16 == 0) {
      print("\r\n");
    }
  }
}