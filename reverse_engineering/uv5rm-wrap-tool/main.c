/*
 * Author: Amo Xu BD4VOW <amo@git.ltd>
 */

#include "encrypt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct BFHeader_ {
  uint8_t cntPart;
  uint8_t lenFwBE[4];
  uint8_t lenDataBE[4];
  uint8_t reserved[7];
} BFHeader;

void print_help(char *argv[]) {
  fprintf(stdout,
          "Usage: %s -i input_file_path -o output_file_path [-m mode] [-d "
          "input_data_file_path] [-f] "
          "[-h]\n",
          argv[0]);
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  -i input_file_path       : Input file\n");
  fprintf(stdout, "  -o output_file_path      : Output file\n");
  fprintf(stdout,
          "  -m mode                  : Mode wrap | unwrap (default: wrap)\n");
  fprintf(stdout,
          "  -d input_data_file_path  : Data input file for WRAP mode\n");
  fprintf(stdout, "  -f                       : Force overwrite\n");
  fprintf(stdout, "  -h                       : Print this help message\n");
}

/**
 * Parse command line arguments
 * @param argc Argument count
 * @param argv Argument vector
 * @param input_file_path Input file path
 * @param output_file_path Output file path
 * @param mode Mode
 * @param input_data_path Input data file path
 * @param force Force overwrite
 */
void parseArgs(int argc, char *argv[], char **input_file_path,
               char **output_file_path, char **mode, char **input_data_path,
               int *force) {
  int opt;
  while ((opt = getopt(argc, argv, "i:o:m:d:fh")) != -1) {
    switch (opt) {
    case 'i':
      *input_file_path = optarg;
      break;
    case 'o':
      *output_file_path = optarg;
      break;
    case 'm':
      *mode = optarg;
      break;
    case 'd':
      *input_data_path = optarg;
      break;
    case 'f':
      *force = 1;
      break;
    case 'h':
      print_help(argv);
      exit(EXIT_SUCCESS);
    }
  }

  if (*input_file_path == NULL || *output_file_path == NULL) {
    fprintf(stderr, "Missing required arguments\n");
    print_help(argv);
    exit(EXIT_FAILURE);
  }
}

/**
 * Open file
 * @param filename File name
 * @param mode File mode
 * @return File pointer
 */
FILE *openFile(const char *filename, const char *mode) {
  FILE *file = fopen(filename, mode);
  if (file == NULL) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    exit(EXIT_FAILURE);
  }
  return file;
}

/**
 * Check and confirm overwrite
 * @param filename File name
 * @param force Force overwrite
 */
void checkAndConfirmOverwrite(char *filename, int force) {
  if (access(filename, F_OK) == 0 && !force) {
    char c;
    printf("File %s already exists. Overwrite? [y/N] ", filename);
    scanf("%c", &c);
    if (c != 'y' && c != 'Y') {
      exit(EXIT_SUCCESS);
    }
  }
}

/**
 * Calculate the path of the decrypted data file
 * @param outputFwPath Path to the decrypted firmware file
 * @return Path to the decrypted data file
 */
char *calcDecryptDataPath(char *outputFwPath) {
  char *dataFilePath =
      malloc(strlen(outputFwPath) + 6); // 6 for ".data" and null terminator
  if (dataFilePath == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");
    exit(EXIT_FAILURE);
  }
  snprintf(dataFilePath, strlen(outputFwPath) + 6, "%s%s", outputFwPath,
           ".data");
  return dataFilePath;
}

/**
 * Read header from file
 * @param file File pointer
 * @param header Header structure
 * @param size Size of header
 */
void readHeader(FILE *file, BFHeader *header, size_t size) {
  if (fread(header, 1, size, file) != size) {
    fprintf(stderr, "Failed to read file header\n");
    exit(EXIT_FAILURE);
  }
}

/**
 * Unwrap the encrypted firmware file
 * @param encryptedFwPath Path to the encrypted firmware file
 * @param decryptFwPath Path to the decrypted firmware file
 * @param force Force overwrite
 */
void unwrapFW(char *encryptedFwPath, char *decryptFwPath, int force) {
  // Open input file
  FILE *fileEncrypedFw = openFile(encryptedFwPath, "rb");
  fseek(fileEncrypedFw, 0, SEEK_END);
  size_t lenEncryptedFw = ftell(fileEncrypedFw);
  fseek(fileEncrypedFw, 0, SEEK_SET);

  checkAndConfirmOverwrite(decryptFwPath, force);
  FILE *fileDecrptedFw = openFile(decryptFwPath, "wb");

  // Read header
  BFHeader header = {0};
  readHeader(fileEncrypedFw, &header, sizeof(header));
  uint8_t cntParts = header.cntPart;

  if (cntParts != 1 && cntParts != 2) {
    fprintf(stderr, "Invalid file format\n");
    exit(EXIT_FAILURE);
  }

  // Read firmware length
  size_t lenFwPart = header.lenFwBE[0] << 24 | header.lenFwBE[1] << 16 |
                     header.lenFwBE[2] << 8 | header.lenFwBE[3];

  // Read data length
  size_t lenDataPart = 0;
  if (cntParts > 1) {
    lenDataPart = header.lenDataBE[0] << 24 | header.lenDataBE[1] << 16 |
                  header.lenDataBE[2] << 8 | header.lenDataBE[3];
  }

  // Calculate the number of packages
  size_t cntFwPkgs = lenFwPart / PACKAGE_SIZE;
  size_t lenLastPkg = lenFwPart % PACKAGE_SIZE;
  if (lenLastPkg > 0) {
    cntFwPkgs++;
  }

  // allocate buffer for reading and writing
  uint8_t *buffer = (uint8_t *)malloc(PACKAGE_SIZE);

  // Decrypt and write firmware file by packages
  for (size_t i = 0; i < cntFwPkgs; i++) {
    // Read package from file
    size_t lenCurPkg =
        (i == cntFwPkgs - 1 && lenLastPkg > 0) ? lenLastPkg : PACKAGE_SIZE;
    fread(buffer, 1, lenCurPkg, fileEncrypedFw);

    // Do not decrypt the first 2 and last 2 packages
    if (i >= 2 && i < cntFwPkgs - 2) {
      encrypt_pkg(buffer, i, lenCurPkg);
    }

    // Write decrypted package to file
    if (fwrite(buffer, 1, lenCurPkg, fileDecrptedFw) != lenCurPkg) {
      fprintf(stderr, "Failed to write firmware file\n");
      exit(EXIT_FAILURE);
    }
  }
  fprintf(stdout, "Firmware file written to %s\n", decryptFwPath);

  // Close Decrypted Firmware file
  fclose(fileDecrptedFw);

  // Write data file if available
  if (cntParts == 2 && lenDataPart > 0 &&
      lenEncryptedFw >= (lenFwPart + lenDataPart + 16)) {
    char *outputDataPath = calcDecryptDataPath(decryptFwPath);

    checkAndConfirmOverwrite(outputDataPath, force);
    FILE *fileOutputData = openFile(outputDataPath, "wb");

    // Calculate the number of packages for data file
    size_t cntDataPkg = lenDataPart / PACKAGE_SIZE;
    size_t lenLastDataPkg = lenDataPart % PACKAGE_SIZE;
    if (lenLastDataPkg > 0) {
      cntDataPkg++;
    }

    // Read and write data file by packages
    for (size_t i = 0; i < cntDataPkg; i++) {
      size_t lenCurDataPkg = (i == cntDataPkg - 1 && lenLastDataPkg > 0)
                                 ? lenLastDataPkg
                                 : PACKAGE_SIZE;
      fread(buffer, 1, lenCurDataPkg, fileEncrypedFw);
      if (fwrite(buffer, 1, lenCurDataPkg, fileOutputData) != lenCurDataPkg) {
        fprintf(stderr, "Failed to write data file\n");
        exit(EXIT_FAILURE);
      }
    }

    fclose(fileOutputData);
    fprintf(stdout, "Data file written to %s\n", outputDataPath);
    free(outputDataPath);
  }

  free(buffer);
  fclose(fileEncrypedFw);
}

/**
 * Wrap the firmware file
 * @param inputFwPath Path to the input firmware file
 * @param inputDataPath Path to the input data file
 * @param outputFwPath Path to the output firmware file
 * @param force Force overwrite
 */
void wrapFW(char *inputFwPath, char *inputDataPath, char *outputFwPath,
            int force) {
  // Open input file
  FILE *fileInputFw = openFile(inputFwPath, "rb");
  fseek(fileInputFw, 0, SEEK_END);
  size_t lenInputFw = ftell(fileInputFw);
  fseek(fileInputFw, 0, SEEK_SET);

  // Open input data file
  FILE *fileInputData = NULL;
  size_t lenInputData = 0;
  if (inputDataPath != NULL) {
    fileInputData = openFile(inputDataPath, "rb");
    fseek(fileInputData, 0, SEEK_END);
    lenInputData = ftell(fileInputData);
    fseek(fileInputData, 0, SEEK_SET);
  }

  // Calculate the number of packages for firmware file
  size_t cntFwPkgs = lenInputFw / PACKAGE_SIZE;
  size_t lenLastPkg = lenInputFw % PACKAGE_SIZE;
  if (lenLastPkg > 0) {
    cntFwPkgs++;
  }

  // Calculate the number of packages for data file
  size_t cntDataPkgs = lenInputData / PACKAGE_SIZE;
  size_t lenLastDataPkg = lenInputData % PACKAGE_SIZE;
  if (lenLastDataPkg > 0) {
    cntDataPkgs++;
  }

  // Calculate the length of the encrypted firmware file
  size_t lenEncryptedFw = sizeof(BFHeader) + lenInputFw;
  if (lenInputData > 0) {
    lenEncryptedFw += lenInputData;
  }

  // Allocate buffer for reading and writing
  uint8_t *buffer = (uint8_t *)malloc(PACKAGE_SIZE);

  // Open output file
  checkAndConfirmOverwrite(outputFwPath, force);
  FILE *fileOutputFw = openFile(outputFwPath, "wb");

  // Write header
  BFHeader header = {0};
  header.cntPart = lenInputData > 0 ? 2 : 1;
  header.lenFwBE[0] = (lenInputFw >> 24) & 0xFF;
  header.lenFwBE[1] = (lenInputFw >> 16) & 0xFF;
  header.lenFwBE[2] = (lenInputFw >> 8) & 0xFF;
  header.lenFwBE[3] = lenInputFw & 0xFF;
  header.lenDataBE[0] = (lenInputData >> 24) & 0xFF;
  header.lenDataBE[1] = (lenInputData >> 16) & 0xFF;
  header.lenDataBE[2] = (lenInputData >> 8) & 0xFF;
  header.lenDataBE[3] = lenInputData & 0xFF;

  // Write header to file
  fwrite(&header, 1, sizeof(header), fileOutputFw);

  // Encrypt and write firmware file by packages
  for (size_t i = 0; i < cntFwPkgs; i++) {
    size_t lenCurPkg =
        (i == cntFwPkgs - 1 && lenLastPkg > 0) ? lenLastPkg : PACKAGE_SIZE;
    fread(buffer, 1, lenCurPkg, fileInputFw);

    // Do not decrypt the first 2 and last 2 packages
    if (i >= 2 && i < cntFwPkgs - 2) {
      encrypt_pkg(buffer, i, lenCurPkg);
    }

    if (fwrite(buffer, 1, lenCurPkg, fileOutputFw) != lenCurPkg) {
      fprintf(stderr, "Failed to write firmware file\n");
      exit(EXIT_FAILURE);
    }
  }
  fclose(fileInputFw);

  // Write data file if available
  if (lenInputData > 0) {
    for (size_t i = 0; i < cntDataPkgs; i++) {
      size_t lenCurDataPkg = (i == cntDataPkgs - 1 && lenLastDataPkg > 0)
                                 ? lenLastDataPkg
                                 : PACKAGE_SIZE;
      fread(buffer, 1, lenCurDataPkg, fileInputData);
      if (fwrite(buffer, 1, lenCurDataPkg, fileOutputFw) != lenCurDataPkg) {
        fprintf(stderr, "Failed to write data file\n");
        exit(EXIT_FAILURE);
      }
    }

    fclose(fileInputData);
  }

  fprintf(stdout, "Firmware file written to %s\n", outputFwPath);
  free(buffer);
  fclose(fileOutputFw);
}

int main(int argc, char *argv[]) {
  // Define command line arguments
  char *inFilePath = NULL;
  char *outFilePath = NULL;
  char *mode = "wrap";
  char *inDataFilePath = NULL;
  int force = 0;

  // Parse command line arguments
  parseArgs(argc, argv, &inFilePath, &outFilePath, &mode, &inDataFilePath,
            &force);

  if (strcmp(mode, "wrap") == 0) {
    wrapFW(inFilePath, inDataFilePath, outFilePath, force);
  } else if (strcmp(mode, "unwrap") == 0) {
    unwrapFW(inFilePath, outFilePath, force);
  } else {
    fprintf(stderr, "Invalid mode: %s\n", mode);
    return 1;
  }

  return 0;
}