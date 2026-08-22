#include <TFT_eSPI.h>
#include <WiFi.h>
#include "time.h"
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <BleMouse.h> // Library for BLE HID Air Mouse

TFT_eSPI tft = TFT_eSPI();
HardwareSerial GPS(2);
WiFiClientSecure client;

// Initialize BLE Mouse (Device Name, Manufacturer, Battery Level)
BleMouse bleMouse("ESP32 Air Mouse", "Custom Watch", 100);

// MPU-6050 I2C Address
#define MPU6050_ADDR 0x68

// WiFi
const char* ssid     = "realme GT 6T";
const char* password = "n3wzritn";

// Telegram
const char* host = "api.telegram.org";
String botToken = "8221758947:AAETWQKwEX0r6gvvjPgdPWjmhId-iFcz_7Y";
String chatID   = "7326269089";

// Time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;

// Buttons
#define BTN_MENU 32
#define BTN_NEXT 5

// --- GLOBAL BUTTON DEBOUNCE & LOCKOUT CONTROL ---
#define BUTTON_LOCKOUT_MS 300  
unsigned long globalButtonLock = 0;

// Modes
int mode = 0;
int selectedApp = 0;

// Fish
int fx=120, fy=185, dx=2, dy=-1;

// Clock box
int boxX=30, boxY=65, boxW=180, boxH=50;

// Game
int dinoY=160, velocity=0, cactusX=240, score=0;
bool jumping=false;

// Gyro / Accel variables
int16_t ax = 0, ay = 0, az = 0;
int16_t gx = 0, gy = 0, gz = 0;
unsigned long lastGyroUpdate = 0;

// Air Mouse Filtered Variables & Sensitivity Tuning
float filteredGz = 0;
float filteredGy = 0;
#define GYRO_ALPHA 0.2f     // Low-pass filter smoothing coefficient (0.0 to 1.0)
#define GYRO_DEADZONE 150  // Ignore tiny sensor jitter noise

// Caution
bool cautionRunning=false;
int cautionStep=0;
unsigned long cautionTimer=0;

// GPS
String latitude="--";
String longitude="--";

// ---------- MPU-6050 HELPER FUNCTIONS ----------
void initMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // Wake up MPU-6050
  Wire.endTransmission(true);
}

void readMPU6050() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B); // Starting register for Accel data
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU6050_ADDR, (size_t)14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // Skip temp
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}

// ---------- BACKGROUND ----------
void drawBackground(){
  tft.fillScreen(tft.color565(0,80,150));
  tft.fillRect(0,200,240,40,tft.color565(194,178,128));

  for(int i=0;i<6;i++){
    tft.drawLine(20+i*3,200,20+i*3,160,TFT_GREEN);
    tft.drawLine(180+i*3,200,180+i*3,150,TFT_GREEN);
  }

  tft.fillRoundRect(boxX,boxY,boxW,boxH,10,tft.color565(0,80,150));
}

// ---------- FISH ----------
void drawFish(int x,int y,int d){
  uint16_t c=tft.color565(255,165,0);
  tft.fillEllipse(x,y,15,8,c);
  if(d>0) tft.fillTriangle(x-15,y,x-25,y-6,x-25,y+6,c);
  else tft.fillTriangle(x+15,y,x+25,y-6,x+25,y+6,c);
  tft.fillCircle(x+(d>0?8:-8),y-2,2,TFT_BLACK);
}

void eraseFish(int x,int y){
  tft.fillRect(x-30,y-20,60,40,tft.color565(0,80,150));
}

// ---------- WIFI ----------
void connectWiFi(){
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(300);
}

// ---------- TIME ----------
void setupTime(){
  configTime(gmtOffset_sec,0,ntpServer);
}

void showTime(){
  struct tm t;
  if(getLocalTime(&t)){
    char timeStr[10], dateStr[20];
    strftime(timeStr,10,"%H:%M:%S",&t);
    strftime(dateStr,20,"%d %b %Y",&t);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE,tft.color565(0,80,150));

    tft.setTextSize(1);
    tft.drawString(dateStr,120,75);

    tft.setTextSize(3);
    tft.drawString(timeStr,120,95);
  }
}

// ---------- MENU ----------
void drawMenu(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);

  // Updated options array: Replaced "Music" with "AirMouse"
  String apps[5]={"Game","Gyro","Caution","AirMouse","GPS"};

  for(int i=0;i<5;i++){
    int y=50+i*30;
    if(i==selectedApp)
      tft.fillRoundRect(40,y-10,160,25,5,TFT_DARKGREY);
    tft.drawString(apps[i],120,y);
  }
}

// ---------- GAME ----------
void drawGameScreen(){
  tft.fillScreen(TFT_WHITE);
  tft.drawLine(0,180,240,180,TFT_BLACK);
  dinoY=160; velocity=0; cactusX=240; score=0;
}

void updateGame(){
  if(millis() > globalButtonLock && digitalRead(BTN_NEXT)==HIGH && !jumping){
    velocity=-14; 
    jumping=true;
    globalButtonLock = millis() + 150;
  }

  velocity++; dinoY+=velocity;
  if(dinoY>=160){ dinoY=160; jumping=false; }

  cactusX-=6;
  if(cactusX<-10){ cactusX=240; score++; }

  if(cactusX<55 && cactusX>35 && dinoY>140){
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("GAME OVER",120,100);
    delay(1500);
    drawGameScreen();
    return;
  }

  tft.fillScreen(TFT_WHITE);
  tft.drawLine(0,180,240,180,TFT_BLACK);

  tft.fillRect(40,dinoY,15,20,TFT_BLACK);
  tft.fillRect(cactusX,160,10,20,TFT_GREEN);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(String(score),120,25);
}

// ---------- GYRO MODE ----------
void drawGyroScreen(){ 
  tft.fillScreen(TFT_BLACK); 
}

void updateGyro(){
  if(millis() - lastGyroUpdate < 150) return;
  lastGyroUpdate = millis();

  readMPU6050();

  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);

  tft.setTextSize(2);
  tft.drawString("GYRO / ACCEL", 120, 25);

  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("--- Accelerometer ---", 120, 55);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("X: " + String(ax), 120, 75);
  tft.drawString("Y: " + String(ay), 120, 95);
  tft.drawString("Z: " + String(az), 120, 115);

  tft.setTextColor(TFT_CYAN);
  tft.drawString("--- Gyroscope ---", 120, 145);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("X: " + String(gx), 120, 165);
  tft.drawString("Y: " + String(gy), 120, 185);
  tft.drawString("Z: " + String(gz), 120, 205);
}

// ---------- CAUTION ----------
void drawCautionScreen(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CAUTION MODE",120,100);
}

void updateCaution(){
  if(millis() > globalButtonLock && !cautionRunning && digitalRead(BTN_NEXT)==HIGH){
    cautionRunning=true;
    cautionStep=0;
    cautionTimer=millis();
    globalButtonLock = millis() + BUTTON_LOCKOUT_MS;
  }

  if(!cautionRunning) return;

  if(millis()-cautionTimer>=3000){
    cautionTimer=millis();
    digitalWrite(19,!digitalRead(19));
    digitalWrite(26,!digitalRead(26));
    cautionStep++;

    if(cautionStep>=8){
      digitalWrite(19,LOW);
      digitalWrite(26,LOW);
      cautionRunning=false;
    }
  }
}

// ---------- AIR MOUSE MODE ----------
void drawAirMouseScreen(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("AIR MOUSE", 120, 30);
  
  tft.setTextSize(1);
  if(bleMouse.isConnected()){
    tft.setTextColor(TFT_GREEN);
    tft.drawString("Status: Connected", 120, 80);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Tilt watch to move", 120, 120);
    tft.drawString("BTN_NEXT: Left Click", 120, 150);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("Status: Disconnected", 120, 80);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Pair on Laptop/Phone:", 120, 120);
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("\"ESP32 Air Mouse\"", 120, 150);
  }
}

void updateAirMouse(){
  static bool wasConnected = false;
  bool isConnected = bleMouse.isConnected();

  // Redraw UI when connection status changes
  if(isConnected != wasConnected){
    drawAirMouseScreen();
    wasConnected = isConnected;
  }

  if(!isConnected) return;

  // Read MPU6050 Sensor
  readMPU6050();

  // Low-Pass Filter (EMA) to smooth out hand jitter
  filteredGz = (GYRO_ALPHA * gz) + ((1.0f - GYRO_ALPHA) * filteredGz);
  filteredGy = (GYRO_ALPHA * gy) + ((1.0f - GYRO_ALPHA) * filteredGy);

  // Calculate mouse delta based on angular velocity (inverting Gz/Gy depending on orientation)
  float moveX = (abs(filteredGz) > GYRO_DEADZONE) ? -filteredGz / 500.0f : 0;
  float moveY = (abs(filteredGy) > GYRO_DEADZONE) ? -filteredGy / 500.0f : 0;

  // Move BLE Cursor
  if(moveX != 0 || moveY != 0){
    bleMouse.move((int8_t)moveX, (int8_t)moveY);
  }

  // Handle NEXT button as Left Click
  if(millis() > globalButtonLock && digitalRead(BTN_NEXT) == HIGH){
    globalButtonLock = millis() + BUTTON_LOCKOUT_MS;
    bleMouse.click(MOUSE_LEFT);
  }

  delay(10); // Polling delay for smoother cursor trajectory
}

// ---------- GPS ----------
float convertToDecimal(String raw,char dir){
  float val=raw.toFloat();
  int deg=int(val/100);
  float min=val-(deg*100);
  float dec=deg+(min/60);
  if(dir=='S'||dir=='W') dec*=-1;
  return dec;
}

void readGPS(){
  while(GPS.available()){
    String line=GPS.readStringUntil('\n');
    if(line.startsWith("$GNRMC")){
      int idx=line.indexOf(',')+1;
      idx=line.indexOf(',',idx)+1;
      idx=line.indexOf(',',idx)+1;

      String latRaw=line.substring(idx,line.indexOf(',',idx));
      idx=line.indexOf(',',idx)+1;
      char latDir=line.charAt(idx);
      idx=line.indexOf(',',idx)+1;

      String lonRaw=line.substring(idx,line.indexOf(',',idx));
      idx=line.indexOf(',',idx)+1;
      char lonDir=line.charAt(idx);

      latitude=String(convertToDecimal(latRaw,latDir),6);
      longitude=String(convertToDecimal(lonRaw,lonDir),6);
    }
  }
}

void sendToTelegram(){
  String msg=(latitude=="--")?
  "Connecting........":
  "https://maps.google.com/?q="+latitude+","+longitude;

  msg.replace(" ","%20");

  if(client.connect(host,443)){
    String url="/bot"+botToken+"/sendMessage?chat_id="+chatID+"&text="+msg;
    client.println("GET "+url+" HTTP/1.1");
    client.println("Host: api.telegram.org");
    client.println("Connection: close");
    client.println();
  }
}

void updateGPS(){
  readGPS();

  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  tft.drawString("GPS",120,40);

  if(latitude=="--"){
    tft.drawString("Connecting...",120,120);
  } else {
    tft.drawString(latitude,120,110);
    tft.drawString(longitude,120,140);
  }

  if(millis() > globalButtonLock && digitalRead(BTN_NEXT)==HIGH){
    globalButtonLock = millis() + 1000;
    sendToTelegram();
  }
}

// ---------- BUTTONS ----------
void handleButtons(){
  if(millis() < globalButtonLock) return;

  static bool lastMenu = LOW;
  static bool lastNext = LOW;

  bool menu = digitalRead(BTN_MENU);
  bool next = digitalRead(BTN_NEXT);

  if(menu == HIGH && lastMenu == LOW){
    globalButtonLock = millis() + BUTTON_LOCKOUT_MS;

    if(mode == 0){
      mode = 1; 
      drawMenu();
    }
    else if(mode == 1){
      mode = selectedApp + 2;

      if(mode == 2) drawGameScreen();
      else if(mode == 3) drawGyroScreen();
      else if(mode == 4) drawCautionScreen();
      else if(mode == 5) drawAirMouseScreen(); // Mode 5 launches Air Mouse
      else if(mode == 6) tft.fillScreen(TFT_BLACK);
    }
    else{
      mode = 0; 
      drawBackground();
    }
  }

  if(next == HIGH && lastNext == LOW){
    if(mode == 1){
      globalButtonLock = millis() + BUTTON_LOCKOUT_MS;
      selectedApp = (selectedApp + 1) % 5;
      drawMenu();
    }
  }

  lastMenu = menu;
  lastNext = next;
}

// ---------- SETUP ----------
void setup(){
  pinMode(BTN_MENU, INPUT_PULLDOWN);
  pinMode(BTN_NEXT, INPUT_PULLDOWN);

  pinMode(19,OUTPUT);
  pinMode(26,OUTPUT);

  tft.init();
  tft.setRotation(0);

  drawBackground();

  connectWiFi();
  setupTime();

  // Initialize I2C for MPU-6050 (SDA=21, SCL=22)
  Wire.begin(21,22);
  initMPU6050();

  // Start Bluetooth Mouse Service
  bleMouse.begin();

  GPS.begin(9600,SERIAL_8N1,16,17);
  client.setInsecure();
}

// ---------- LOOP ----------
void loop(){

  handleButtons();

  if(mode==1) return;
  if(mode==2){ updateGame(); delay(50); return; }
  if(mode==3){ updateGyro(); return; }
  if(mode==4){ updateCaution(); return; }
  if(mode==5){ updateAirMouse(); return; } // BLE Air Mouse loop
  if(mode==6){ updateGPS(); return; }

  eraseFish(fx,fy);

  fx+=dx;
  fy+=dy;

  if(fx>180||fx<40) dx*=-1;
  if(fy>190||fy<70) dy*=-1;

  if(fx>boxX-10&&fx<boxX+boxW+10 &&
     fy>boxY-10&&fy<boxY+boxH+10){
    dx*=-1;
    dy*=-1;
  }

  drawFish(fx,fy,dx);
  showTime();

  delay(50);
}
