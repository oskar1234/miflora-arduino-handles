# Thanks to:
Documentation about the MiFlora interface: https://github.com/ChrisScheffler/miflora/wiki/The-Basics

General Documentation about HM-10: http://www.martyncurrey.com/hm-10-bluetooth-4ble-modules/ 

Reference implementation in python: https://github.com/basnijholt/miflora

# Steps you need to do before

0. Usually a firmware Update. You NEED the "Self-Learning Functions" --> So V700 or higher
1. Set your HM-10 in "Master Mode": AT+ROLE1
2. Set your HM-10 in "Manual Connect": AT+IMME1
3. Set the Baudrate to 19200: AT+BAUD1

# My Setup
* Original arduino UNO R3
* DSD Tech HM-10 Clone
* MiFlora Sensor: Flower Care / Model: HHCCJCY01HHCC
* CH340C - USB --> UART Converter for flashing and testing