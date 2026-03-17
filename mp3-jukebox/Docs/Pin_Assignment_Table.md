# Pin Assignment Table

|        Function       | ESP32 Pin | Connected To      | Interface |         Notes        |
|-----------------------|----------|--------------------|-----------|----------------------|
| DFPlayer TX           | GPIO17   | DFPlayer RX        |   UART    | Serial communication |
| DFPlayer RX           | GPIO16   | DFPlayer TX        |   UART    | Serial communication |
| OLED SDA              | GPIO21   | OLED SDA           |   I2C     |     Display data     |
| OLED SCL              | GPIO22   | OLED SCL           |   I2C     |     Display clock    |
| Button 1 (Up)         | GPIO27   | Push Button        |   GPIO    |     Digital input    |
| Button 2 (Down)       | GPIO13   | Push Button        |   GPIO    |     Digital input    |
| Button 3 (Back)       | GPIO14   | Push Button        |   GPIO    |     Digital input    |
| Button 4 (Play)       | GPIO12   | Push Button        |   GPIO    |     Digital input    |
| Volume Potentiometer  | GPIO34   | Potentiometer      |   ADC     |     Analog input     |
| 5V Input              | 5V Pin   | USB-C Board Output |   Power   |   Main supply rail   |
| Common Ground         | GND      | All modules        |   Power   |     Shared ground    |