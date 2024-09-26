Arduino RFID Library for MFRC522 (13.56 Mhz)
--------------------------------------------

Pin order, starting from the bottom left hand pin (in case your
MFRC522 doesn't have pin markings like the B2CQSHOP one):

| Pins | SPI      | UNO  | Mega2560 | Leonardo/Due |
| ---- |:--------:|:----:|:--------:|:------------:|
| 1    | SDA (SS) |  10  |  53      | 10           |
| 2    | SCK      |  13  |  52      | SCK`1`       |
| 3    | MOSI     |  11  |  51      | MOSI`1`      |
| 4    | MISO     |  12  |  50      | MISO`1`      |
| 5    | IRQ      |  `*` |  `*`     | `*`          |
| 6    | GND      |  GND |  GND     | GND          |
| 7    | RST      |  5   |  ?       | Reset        |
| 8    | +3.3V    |  3V3 |  3V3     | 3.3V         |
`*` Not needed  
`1` on ICPS header

Using MFRC522 with other SPI components
---------------------------------------

If you are planning to use other SPI components you just have to make
sure each have an exclusive SS (Slave Select) line.  MISO, MOSI and
SCK lines may be shared. More reference regarding SPI may be found
[here](http://arduino.cc/en/Reference/SPI).
