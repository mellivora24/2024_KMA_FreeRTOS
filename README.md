# 2024_KMA_FreeRTOS
## Trước khi nạp chương trình:
* Set up Blynk IoT:
  * Truy cập Blynk IoT: [tại đây](https://blynk.io/)
  * Đăng kí tài khoản và tạo template mới gồm 2 Virtual pin (V2, V3) tương ứng với 2 button trên Webdash Board
  * Chuyển đến phần Devices và add devices từ template đã tạo, sau đó copy ID, AUTHTOKEN, NAME và dán vào code của esp
* Cài thư viện cho Arduino:
  * [FreeRTOS](https://www.arduino.cc/reference/en/libraries/freertos)
  * [Servo](https://www.arduino.cc/reference/en/libraries/servo/)
  * [OneWire](https://www.arduino.cc/reference/en/libraries/onewire)
  * [DallasTemperature](https://www.arduino.cc/reference/en/libraries/dallastemperature)
  * [LiquidCrystal_I2C](https://www.arduino.cc/reference/en/libraries/liquidcrystal-i2c)
