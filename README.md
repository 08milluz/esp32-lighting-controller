# esp32-lighting-controller
This project is the hardware and software for an ESP32-based LED Pixel and DMX interface in Arduino.

## This project requires the following libraries:
* esp32 Board Model in Arduino at version 1.0.6
* FastLED at version 3.4.0
* Sparkfun DMX Shield Library at version 1.0.5
* ESP Asyc E1.31 at version 1.0.2


## The board version 1.0 pinouts:
This is the current version of the board under development.

| led  | pin |
| ------------- | ------------- |
| 01  | 15  |
| 02  | 00  |
| 03  | 23  |
| 04  | 16  |
| 05  | 17  |
| 06  | 05  |
| 07  | 18  |
| 08  | 19  |
| 09  | 22  |
| 10  | 21  |

DMX uses pins 16 and 17 for RX and TX and pin 4 for direction control.

## The board version 0.9 pinouts:
Note: version 0.9 has not been widely distributed

| led  | pin |
| ------------- | ------------- |
| 01  | 34  |
| 02  | 39  |
| 03  | 23  |
| 04  | 22  |
| 05  | 21  |
| 06  | 19  |
| 07  | 18  |
| 08  | 05  |
| 09  | 00  |
| 10  | 15  |

DMX uses pins 1 and 3 for RX and TX and pin 4 for direction control.

