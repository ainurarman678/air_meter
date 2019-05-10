#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>

#define I2C_MASTER_ADDRESS 8
#define BOTtoken "743559213:AAHql-Egse7NxA3SIS27_CRva5Xyyhi3iqM"
#define BOTname "airmeterbot"
#define BOTusername "airmeterbot"
TelegramBOT bot(BOTtoken, BOTname, BOTusername);
ESP8266WiFiMulti wifiMulti;
WiFiUDP UDP;
IPAddress timeServerIP;
const char* NTPServerName = "0.id.pool.ntp.org";
const int NTP_PACKET_SIZE = 48;
byte NTPBuffer[NTP_PACKET_SIZE];
int Bot_mtbs = 250;
long Bot_lasttime;
bool Start = false;
unsigned long intervalNTP = 30000;
unsigned long prevNTP = 0;
unsigned long lastNTPResponse = millis();
unsigned long prevActualTime = 0;
uint32_t timeUNIX = 0;
int temp1, temp2, humidity1, ppm1, timeH0;
String timeM, timeH;
String dateTime, dateRaw, timeRaw;
String dataIn;
String dt[10];
int i;
boolean parsing=false;
char inChar;
String tempActual, humidityActual, ppmActual, timeString;
String ssid, ipaddr;
String AP1 = "LABTIK";

void setup() {
  Wire.begin(D1, D2);
  Serial.begin(115200);
  delay(10);
  startWiFi();
  startUDP();
  if ( !WiFi.hostByName(NTPServerName, timeServerIP) ) {
    Serial.println("DNS lookup failed. Reboot");
    Serial.flush();
    ESP.reset();
  }
  Serial.print("NTP Server : ");
  Serial.println(timeServerIP);
  sendNTPpacket(timeServerIP);
  dataIn="";
  bot.begin();
  Wire.beginTransmission(8);
  Wire.write("Ready");
  Wire.endTransmission();
}

void loop() {
  if ( millis() > Bot_lasttime + Bot_mtbs ) {
    bot.getUpdates(bot.message[0][1]);
    Bot_ExecMessages();
    Bot_lasttime = millis();
  }
  unsigned long currentMillis = millis();
  if ( currentMillis - prevNTP > intervalNTP ) {
    prevNTP = currentMillis;
    sendNTPpacket(timeServerIP);
  }
  uint32_t time = getTime();
  if (time) {
    timeUNIX = time;
    lastNTPResponse = currentMillis;
  } else if ((currentMillis - lastNTPResponse) > 36000000) {
    Serial.println("No NTP Response. Reboot...");
    Serial.flush();
    ESP.reset();
  }
  uint32_t actualTime = timeUNIX + (currentMillis - lastNTPResponse) / 1000;
  if (actualTime != prevActualTime && timeUNIX != 0) {
    prevActualTime = actualTime;
    if ( (getHours(actualTime) + 7) > 24 ){
      timeH0 = (getHours(actualTime) + 7) - 24;
    } else{
      timeH0 = getHours(actualTime) + 7;
    }
    if ( timeH0 < 10 ){
      timeH = "0" + String(timeH0);
    }else{
      timeH = String(timeH0);
    }
    if( getMinutes(actualTime) < 10 ){
      timeM = "0" + String(getMinutes(actualTime));
    }else{
      timeM = String(getMinutes(actualTime));
    }
    timeRaw = timeH + ":" + timeM;
    timeString = "Data diambil pada waktu " + timeRaw;
  }
  Wire.requestFrom(8, 14);
  while(Wire.available()){
      inChar = (char)Wire.read();
      dataIn += inChar;
      if (inChar == '#') {
      parsing = true;
    }
    if(parsing){
        parsingData();
        parsing=false;
        dataIn="";
    }
  }
  tempActual = "Suhu saat ini adalah " + String(temp2) + "C";
  humidityActual = "Kelembapan saat ini adalah " + String(humidity1) +"%";
  //ppmActual = "Kualitas udara " + String(ppm1) + "PPM";
}

void startWiFi() {
//  wifiMulti.addAP(AP1, "");
  wifiMulti.addAP("LABTIK", "l4bt1k2019");
//  wifiMulti.addAP(AP3),"2019ujianmtcna");
  Serial.println("Connecting");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("Connected ! ");
  Serial.print("IP Address : ");
  Serial.print(WiFi.localIP());
}

void Bot_ExecMessages() {
  for ( int i = 1; i < bot.message[0][0].toInt() + 1; i++){
    bot.message[i][5] = bot.message[i][5].substring(1, bot.message[i][5].length());
    if ( bot.message[i][5] == "get" ) {
      bot.sendMessage(bot.message[i][4], tempActual, "");
      bot.sendMessage(bot.message[i][4], humidityActual, "");
      //bot.sendMessage(bot.message[i][4], ppmActual, "");
      bot.sendMessage(bot.message[i][4], timeString, "");
    }
    if ( bot.message[i][5] == "wifistatus" ) {
      bot.sendMessage(bot.message[i][4], "Modul terkoneksi dengan " + AP1, "");
      bot.sendMessage(bot.message[i][4], "Alamat IP = " + WiFi.localIP(), "");
    }
    if ( bot.message[i][5] == "start" ) {
      bot.sendMessage(bot.message[i][4], "Bot ini merupakan sebuah klien untuk menerima data dari sensor.", "");
      bot.sendMessage(bot.message[i][4], "/get untuk menarik data sensor.", "");
      bot.sendMessage(bot.message[i][4], "/wifistatus untuk melihat status koneksi internet.", "");
      bot.sendMessage(bot.message[i][4], "/start untuk memulai bot dan menampilkan pesan ini.", "");
      Start = true;
    }
  }
  bot.message[0][0] = "";
}

void startUDP() {
  Serial.println("Starting UDP");
  UDP.begin(123);                          // Start listening for UDP messages on port 123
  Serial.print("Local port:\t");
  Serial.println(UDP.localPort());
  Serial.println();
}

void sendNTPpacket(IPAddress& address) {
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

uint32_t getTime() {
  if (UDP.parsePacket() == 0) { // If there's no response (yet)
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}

inline int getSeconds(uint32_t UNIXTime) {
  return UNIXTime % 60;
}

inline int getMinutes(uint32_t UNIXTime) {
  return UNIXTime / 60 % 60;
}

inline int getHours(uint32_t UNIXTime) {
  return UNIXTime / 3600 % 24;
}

void parsingData(){
  int j=0;
  //inisialisasi variabel, (reset isi variabel)
  dt[j]="";
  //proses parsing data
    for(i=1;i<dataIn.length();i++){
    //pengecekan tiap karakter dengan karakter (#) dan (,)
    if ((dataIn[i] == '#') || (dataIn[i] == ',')) {
    //increment variabel j, digunakan untuk merubah index array penampung
      j++;
      dt[j]="";       //inisialisasi variabel array dt[j]
    }
    else {
      dt[j] = dt[j] + dataIn[i];
    }
  }
  temp1 = dt[0].toInt();
  temp2 = dt[1].toInt();
  humidity1 = dt[2].toInt();
}
