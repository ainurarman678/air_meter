#include <dht.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Wire.h>

#define PRINT_DATA_TO_SERIAL 1
#define ONE_WIRE_BUS 3
#define DHT11_PIN 2
//#define SENSOR_MQ A1
dht DHT;
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
String temp1, temp2, humidity1, rawData;
int temp1Raw, temp2Raw, humidity1Raw;
char c, data[17];
//int sensorMqData;
int sendReady = 0;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200); 
  Serial.println("Air Meter"); 
  Serial.println("Initializing ....");
  Serial.print("DHT LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  sensors.begin();
//  Serial.print("Waiting gateway to ready");
//  while ( String(c) != "Ready" || sendReady == 0) {
//    Serial.print(".");
//    delay(500);
//    sendReady = 1;
//  }
//  Serial.println("");
  Serial.println("Ready!");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Request sensor output ....");
  sensors.requestTemperatures();
  temp1Raw = sensors.getTempCByIndex(0);
  int chk = DHT.read11(DHT11_PIN);
  //sensorMqData = analogRead(SENSOR_MQ);
  switch (chk)
  {
    case DHTLIB_OK:  
      temp2Raw = DHT.temperature;
      humidity1Raw = DHT.humidity;
      break;
    case DHTLIB_ERROR_CHECKSUM: 
      temp2Raw = "DHT Checksun Error !";
      humidity1Raw = "DHT Checksun Error !";
      break;
    case DHTLIB_ERROR_TIMEOUT: 
      temp2Raw = "DHT Time Out !";
      humidity1Raw = "DHT Time Out !";
      break;
    case DHTLIB_ERROR_CONNECT:
      temp2Raw = "DHT Connect Error !";
      humidity1Raw = "DHT Connect Error !";
      break;
    case DHTLIB_ERROR_ACK_L:
      temp2Raw = "DHT ACK Low Error !";
      humidity1Raw = "DHT ACK Low Error !";
      break;
    case DHTLIB_ERROR_ACK_H:
      temp2Raw = "DHT ACK High Error !";
      humidity1Raw = "DHT ACK High Error !";
      break;
    default: 
      temp2Raw = "DHT Unknown Error !";
      humidity1Raw = "DHT Unknown Error !";
      break;
  }
  temp1 = String(temp1Raw);
  temp2 = String(temp2Raw);
  humidity1 = String(humidity1Raw);
  if ( PRINT_DATA_TO_SERIAL == 1 ) {
    Serial.print("Temperature 1 Sensor Output : ");
    Serial.println(temp1);
    Serial.print("Temperature 2 Sensor Output : ");
    Serial.println(temp2);
    Serial.print("Humidity 1 Sensor Output : ");
    Serial.println(humidity1);
    Serial.print("Gas PPM : ");
    //Serial.println(sensorMqData);
    //Serial.println(rawData);
    Serial.println("");
  }
  rawData = "*" + temp1 + "," + temp2 + "," + humidity1 + "#";
  delay(1000);
}

void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    c = Wire.read();
  }
}

void requestEvent() {
 char buffer[20];
 rawData.toCharArray(buffer, 20);
  Wire.write(buffer);
}
