# Baofeng-UV-5RM-5RH-RE
Reverse Engineering of Baofeng UV-5RM(Global Tri-band)/UV-5RH(CN Tri-band) Radio

## Introduction
Front Photo
![front](./teardown/1-front.jpg)
Back Photo
![back](./teardown/2-back.jpg)
Label Photo
![labels](./teardown/5-labels.jpg)

## Teardown
Teardown Overview
![teardown](./teardown/7-teardown-2.jpg)

## Hardware
### Overview
PCB Top Photo
![pcb-top](./teardown/10-pcb-top.jpg)
PCB Bottom Photo
![pcb-bottom](./teardown/11-pcb-bottom.jpg)

### Main Components

| Component | Silkprint | Description |
| --- | --- | --- |
| MCU | SYN2A-000 | Silkprint Customized Artery AT32F421C8T7 without bootloader (120MHz,64KByte Flash,16KByte SRAM) |
| RF | Beken BK4819 | |
| Flash | XMC 25QH16CJIG | 16Mbit |
| LCD | (unknow yet) | |

## Reverse Engineering
### 1). MCU pinout guessing

### 2). MCU part number detection over SWD

### 3). Bootloader binary dumping

### 4). Bootloader reverse engineering for Firmware Upgrade Protocol and Encryption details

### 5). Factory firmware decryption and reverse engineering

### 6). Opensource Flashing Tool development