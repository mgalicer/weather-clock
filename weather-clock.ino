#include <SPI.h>
#include "WiFi101.h"
#include "keys.h"
#include "ArduinoJson.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// initializes the ring with arguments(number of pixels, input pin, type of pixels)
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);

char ssid[] = WIFI_NAME; //  your network SSID (name)
char pass[] = WIFI_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "weatherclockserver.herokuapp.com";


StaticJsonBuffer<600> jsonBuffer;
char c;
int i = 0;
String response = "";
String placeholder = "";
char json[600];
// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

// void setLEDColor(temp) {
  // if temp is under 60 degrees, find the difference from 60 and multiply that by 4.25 to find the G value (B stays at 255)
  // if the temperature is above 60 degrees, increase the G value by 5 (R stays at 255)
// }

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(60);
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    Serial.print("here");
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /weather HTTP/1.1");
    client.println("Host: weatherclockserver.herokuapp.com");
    client.println("Connection: close");
    client.println();
  }
}

void loop() {
  while(!client.available()) {};

  while(client.available()) {
    c = client.read();
    response += c;
  }


  for(int hour = 1; hour <= 12; hour++) {
    if(hour < 10) {
      placeholder = "0" + String(hour) += ",";
    } else {
      placeholder = String(hour) += ",";
    }

    int startTemp = response.indexOf(placeholder) + 3;
    int endTemp = startTemp + 3;

    int temp = response.substring(startTemp, endTemp).toInt();

    int startPrecip = endTemp + 1;
    int endPrecip = startPrecip + 3;

    int precip = response.substring(startPrecip, endPrecip).toInt();
    Serial.print(hour);
    Serial.print(temp);
    Serial.print(precip);

    assignLedColor(hour, temp, precip);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
     while (true);
  }

}

void assignLedColor(int hour, int temp, int precip) {
  if(temp > 0 && temp < 60) {
    pixels.setPixelColor(hour * 2, 0, temp * 4, 255);
    pixels.setPixelColor(hour * 2 + 1, 0, temp * 4, 255);
  } else if (temp > 60 && temp < 110) {
    pixels.setPixelColor(hour * 2, 255, 255 - temp * 5, 0);
    pixels.setPixelColor(hour * 2 + 1, 255, 255 - temp * 5, 0);
  }

  pixels.show();
}



void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
