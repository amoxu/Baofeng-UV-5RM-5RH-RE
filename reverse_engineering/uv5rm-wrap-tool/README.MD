# uv5rm-wrap-tool

## Description
The `uv5rm-wrap-tool` is a utility tool for wrapping and unwrapping binary firmware files for the Baofeng UV-5RM radio.

## Features
- Encrypting Firmware binary file and wrapping with data file into the UV-5RM firmware format `*.BF`
- Unwrap & Decrypting Firmware binary and data file from the UV-5RM firmware format

## Compiling
1. Clone this repository.
2. Run `make` under the `reverse_engineering\uv5rm-wrap-tool` directory of the repository.

## Usage
To wrap a file:
```bash
./uv5rm-wrap-tool -m wrap -i <firmware.bin> -d <data.bin> -o <output.BF>
```

To unwrap a file:
```bash
./uv5rm-wrap-tool -m unwrap -i <input.BF> -o <output_firmware.bin>
```