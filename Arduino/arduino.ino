#include <Servo.h>
#include <OneWire.h>
#include "pinAndDevices.h"
#include <SoftwareSerial.h>
#include <Arduino_FreeRTOS.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

Servo window;              // Servo object
SoftwareSerial ESP(7, 8);  // RX, TX
OneWire oneWire(temp_sensor);
LiquidCrystal_I2C screen(0x27, 16, 02);
DallasTemperature temperature(&oneWire);

// Avaiables to save value of temperature and gas leak
int temp_value = 0, gas_value = 0;
bool windowOpened = false, ledState = false, warningOn = false, kittchenFanOn = false;

void pin_mode();           // Config pin
void door(bool state);     // Close or open the window
void warning(bool state);  // Turn warning system ON or OFF

void TaskWarning(void *pv);        // Check gas leak value and turn warning system ON or OFF
void TaskScreenUpdate(void *pv);   // Show information about temperature and gas leak value
void TaskManualControl(void *pv);  // Control LED, window or kittchen fan with sensor or manual switch

/**************************** Main *******************************/
void setup() {
  pin_mode();
  ESP.begin(115200);
  Serial.begin(9600);

  bool s = false;
  char start;
  while (!s) {
    if (ESP.available()) start = ESP.read();
    if (start == '+') s = true;
  }

  // Set up FreeRTOS tasks
  Serial.println("Creating tasks...");
  xTaskCreate(TaskScreenUpdate, "Screen Update", 128, NULL, 2, NULL);
  xTaskCreate(TaskManualControl, "Manual Control", 64, NULL, 1, NULL);
  xTaskCreate(TaskWarning, "Warning system", 128, NULL, 1, NULL);
  Serial.println("Successful!");

  // Start FreeRTOS scheduler
  vTaskStartScheduler();
  Serial.println("Start tasks!");
}
void loop() {}  // FreeRTOS run

/******************** Contribute functions ***********************/
void pin_mode() {
  for (int port = 2; port <= 4; port++) pinMode(port, INPUT);
  for (int port = 10; port <= 13; port++) pinMode(port, OUTPUT);

  pinMode(5, OUTPUT);
  pinMode(gas_sensor, INPUT);
  pinMode(sound_sensor, INPUT);
  pinMode(pir_sensor, INPUT);
}
void warning(bool state) {  // False turn ON, true turn OFF
  if (state) {
    digitalWrite(cool_fan, 1);  // Fan ON
    digitalWrite(buzzer, 1);    // Alarm ON
  } else {
    digitalWrite(cool_fan, 0);  // Fan OFF
    digitalWrite(buzzer, 0);    // Alarm OFF
  }
}
void door(bool state) {  // False to close, true to open
  if (state) {
    window.attach(servo_pin);
    window.write(160);
    delay(110);
    window.detach();
  } else if (!state) {
    window.attach(servo_pin);
    window.write(0);
    delay(100);
    window.detach();
  }
}

/**************************** Tasks ******************************/
void TaskManualControl(void *pv) {
  while (1) {
    // Check sound sensor
    // if (analogRead(sound_sensor) > 800) {
    //   if (ledState) {
    //     ESP.println('d');
    //     digitalWrite(light, 0);
    //     ledState = false;
    //   } else {
    //     ESP.println('c');
    //     digitalWrite(light, 1);
    //     ledState = true;
    //   }
    // }
    // Check pir sensor
    if (analogRead(pir_sensor) > 300 && !ledState) {
      ESP.println('c');
      digitalWrite(light, 1);
    }
    // Button 1 - led
    if (digitalRead(btn_1)) {
      if (!ledState) {
        ESP.println('c');
        ledState = true;
        digitalWrite(light, 1);
      } else {
        ESP.println('d');
        ledState = false;
        digitalWrite(light, 0);
      }
      while (digitalRead(btn_1)) {}  // Do nothing when hold button
    }
    // Button 2 - window
    if (digitalRead(btn_2)) {
      if (!windowOpened) {
        ESP.println('a');
        door(true);
        windowOpened = true;
      } else {
        ESP.println('b');
        door(false);
        windowOpened = false;
      }
      while (digitalRead(btn_2)) {}  // Do nothing when hold button
    }
    // Button 3 - kittchen fan
    if (digitalRead(btn_3)) {
      if (!kittchenFanOn) {
        digitalWrite(kittchen_fan, 1);
        kittchenFanOn = true;
      } else {
        digitalWrite(kittchen_fan, 0);
        kittchenFanOn = false;
      }
      while (digitalRead(btn_3)) {}  // Do nothing of hold button
    }

    /* Execute command from Blynk */
    if (ESP.available()) {
      char command = ESP.read();

      switch (command) {
        case 'a':  // Open window
          if (!windowOpened) {
            door(true);
            windowOpened = true;
          }
          break;
        case 'b':  // Close window
          if (windowOpened) {
            door(false);
            windowOpened = false;
          }
          break;
        case 'c':  // Turn light ON
          if (!ledState) {
            digitalWrite(light, 1);
            ledState = true;
          }
          break;
        case 'd':  // Turn light OFF
          if (ledState) {
            digitalWrite(light, 0);
            ledState = false;
          }
          break;
        default:
          break;
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
void TaskWarning(void *pv) {
  temperature.begin();

  while (1) {
    gas_value = map(analogRead(gas_sensor), 200, 1024, 0, 100);
    temp_value = temperature.getTempCByIndex(0);

    // Check gas sensor, if leak turn on warning system, open window
    if (gas_value > 30 || temp_value > 50) {
      if (!warningOn) {
        warning(true);
        warningOn = true;
      }
      if (!windowOpened) {
        door(true);
        windowOpened = true;
      }
    } else if (gas_value < 10 && temp_value < 50 && warningOn) {
      warning(false);
      warningOn = false;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
void TaskScreenUpdate(void *pv) {
  // Init and display screen
  screen.init();
  screen.backlight();
  screen.clear();

  // Display temperature
  screen.setCursor(0, 0);
  screen.print("Temp: ");

  // Display gas leak value
  screen.setCursor(0, 1);
  screen.print("Gas leak: ");

  // Begin sensor
  temperature.begin();

  while (1) {
    temp_value = temperature.getTempCByIndex(0);
    gas_value = map(analogRead(gas_sensor), 200, 1024, 0, 100);
    if (gas_value < 0) gas_value = 0;

    screen.setCursor(6, 0);
    screen.print(temp_value);
    screen.setCursor(8, 0);
    screen.print("*C");
    screen.setCursor(10, 1);
    screen.print(gas_value);
    screen.setCursor(12, 1);
    screen.print("%");

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
