/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "*************"
#define BLYNK_TEMPLATE_NAME "***********"
#define BLYNK_AUTH_TOKEN "**************"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "****";
char pass[] = "********";

BLYNK_WRITE(V2) {
  int ledState = param.asInt();  // Get the value from the Blynk app (0 or 1)
  if (ledState) Serial2.println('c');
  else Serial2.println('d');
}
BLYNK_WRITE(V3) {
  int windowOpened = param.asInt();  // Get the value from the Blynk app (0 or 1)
  if (windowOpened) Serial2.println('a');
  else Serial2.println('b');
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Started!");
  Serial2.println('+');
}

void loop() {
  Blynk.run();

  if (Serial2.available()) {
    char receivedData = Serial2.read();

    if (receivedData == 'a') {
      Blynk.virtualWrite(V3, 1);
    } else if (receivedData == 'b') {
      Blynk.virtualWrite(V3, 0);
    } else if (receivedData == 'c') {
      Blynk.virtualWrite(V2, 1);
    } else if (receivedData == 'd') {
      Blynk.virtualWrite(V2, 0);
    }
  }
}
