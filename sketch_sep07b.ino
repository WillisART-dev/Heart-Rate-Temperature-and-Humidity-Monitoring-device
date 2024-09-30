#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DH T_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
WiFiClient client;


#define DHTPIN D2  // Pin connected to DHT11
#define DHTTYPE DHT11// DHT 11 Sensor
//#define DHTPIN 2
DHT dht(D2, DHT11);

ESP8266WebServer server(80);

int const PULSE_SENSOR_PIN = 0;
int heartRatePin = A0;
int threshold = 512; // Threshold for detecting heartbeat
unsigned long lastBeat = 0;  // Time of the last beat detection
int BPM = 0;  // Beats per minute
int beats = 0;
unsigned long startTime = 0;

// Wi-Fi credentials
String apikey = "EPHKEXWO46DW8DAD";
const char* server1 = "api.thingspeak.com";
const char* ssid = "iPhone";
const char* password = "qwertyuiop.";
int channelID = 2649326;

// Variables for heart rate sensor debounce
bool pulseDetected = false;
unsigned long pulseStartTime = 0;
unsigned long pulseInterval = 0;

void setup() {
  pinMode(D6,OUTPUT);
  Serial.begin(9600);
  delay(10);

  // Initialize DHT11 sensor
  dht.begin();
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(".");
  Serial.println("Wifi Connected");
  // Initialize Wi-Fi in Access Point mode

  // Initialize timing
  startTime = millis();
}

void loop() {
  // Detect pulse and calculate BPM
  detectPulse();
}

void detectPulse() {
  int heartRateValue = analogRead(heartRatePin);
  if (heartRateValue>threshold){
    digitalWrite(D6,HIGH);
    delay(1500);
  }else{
    digitalWrite(D6, LOW);
    delay(1500);
  }
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println(F("Error reading temperature!"));
    //return
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(temp);
    Serial.println(F("°C"));
  }
  float hum = dht.readHumidity();
  if (isnan(hum)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(hum);
    Serial.println(F("°C"));
  }
  // Check if heartRateValue crosses the threshold (detecting a pulse)
  if (heartRateValue > threshold && !pulseDetected) {
    pulseDetected = true;
    unsigned long currentTime = millis();
    pulseInterval = currentTime - lastBeat;
    lastBeat = currentTime;

    if (pulseInterval > 300 && pulseInterval < 2000) {  // Valid pulse range
      BPM = 60000 / pulseInterval;  // Calculate BPM
    }
  }

  // Reset pulse detection when value goes below threshold
  if (heartRateValue < threshold) {
    pulseDetected = false;
  }
  if (client.connect(server1,80))   //   "184.106.153.149" or api.thingspeak.com
  {
   String postStr = apikey;
   postStr +="&field1=";
   postStr += String(temp);
   postStr +="&field2=";
   postStr += String(hum);
   postStr +="&field3=";
   postStr += String(heartRateValue);
   postStr +="&field4=";
   postStr += String(BPM);
   postStr += "\r\n\r\n";

   client.print("POST /update HTTP/1.1\n");
   client.print("Host: api.thingspeak.com\n");
   client.print("Connection: close\n");
   client.print("X-THINGSPEAKAPIKEY: "+apikey+"\n");
   client.print("Content-Type: application/x-www-form-urlencoded\n");
   client.print("Content-Length: ");
   client.print(postStr.length());
   client.print("\n\n");
   client.print(postStr);
   Serial.println("%. Send to Thingspeak.");
}
client.stop();
 
Serial.println("Waiting...");
Serial.print(F("HeartRate: "));
Serial.print(heartRateValue);
Serial.println(F("°d"));
Serial.print(F("HBM: "));
Serial.print(BPM);
Serial.println(F("°d"));

  
  // thingspeak needs minimum 15 sec delay between updates
delay(1500);

  }

