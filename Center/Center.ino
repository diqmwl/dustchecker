#include "ThingSpeak.h" //클라우드 연동 include
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //I2C LCD
#include "image.h" //bmp이미지를 image.h에 배열로 저장해놓고 include함
#include <Adafruit_GFX.h>    // 그래픽 라이브러리
#include <Adafruit_ST7735.h> // 이미지 불러오기위한 라이브러리
#include <SPI.h>

#define TFT_CS     15 //TFT방식 LCD사용하기 위한 변수 선언
#define TFT_RST    2 
#define TFT_DC     0

#if defined(ARDUINO_AVR_YUN)
#include "YunClient.h"
YunClient client;
#else
#if defined(USE_WIFI101_SHIELD) || defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
// Use WiFi
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#else
#include <SPI.h>
#include <WiFi101.h>
#endif
char *ssid = "AndroidHotspot2750";    //  사용하고자 하는 와이파이 이름
char *pass = "---password---";   // 와이파이 비밀번호 이름
int status = WL_IDLE_STATUS;
WiFiClient  client;
#elif defined(USE_ETHERNET_SHIELD)
// Use wired ethernet shield
#include <SPI.h>
#include <Ethernet.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient client;
#endif
#endif

unsigned long myChannelNumber = 499591; //thigspeak 채널 번호
unsigned long youChannelNumber = 500008;

String voice = "";
String voice2;

LiquidCrystal_I2C lcd(0x3F, 16, 2); // 0x27 or 0x3F
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {

  Serial.begin(9600);
  delay(100);
#ifdef ARDUINO_AVR_YUN
  Bridge.begin();
#else
#if defined(ARDUINO_ARCH_ESP8266) || defined(USE_WIFI101_SHIELD) || defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_ARCH_ESP32)
  WiFi.begin(ssid, pass);
#else
  Ethernet.begin(mac);
#endif
#endif

  ThingSpeak.begin(client);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  tft.initR(INITR_144GREENTAB);
  tft.setTextWrap(false);
  tft.fillScreen(ST7735_WHITE);
}

void loop() {
  String voice;
  char cmd;
  if (Serial.available()) { //블루투스 통신이 들어오면
    int num = Serial.available(); // 전체길이를 num에 저장한다
    for (int i = 0; i < num; i++) {// 저장후 전체길이만큼 반복한다
      delay(10);
      cmd = (char)Serial.read(); //한바이트씩 cmd에 저장한다
      if (cmd == NULL) {
        break; //cmd값이 NULL일경우 루프문을 빠져나간다
      }
      voice += cmd; //한글값은 1바이트가 아니므로 voice에 cmd값을 추가적으로 더한다
    }
    voice2 = voice;
  }

  Serial.print(voice2);
  if (voice2 == "105호") { //스마트폰을통한 음성인식이 105호 일때
    int dust105 = ThingSpeak.readFloatField(youChannelNumber, 1); //thigspeak 자체 함수로서 채널의 첫번째 필드의 마지막 값을 읽어오는 역할을 한다.
    int temp105 = ThingSpeak.readFloatField(youChannelNumber, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DUST : ");
    lcd.setCursor(7, 0);
    lcd.print(dust105);
    lcd.setCursor(11, 0);
    lcd.setCursor(0, 1);
    lcd.print("TEMP : ");
    lcd.setCursor(7, 1);
    lcd.print(temp105);
    delay(100);
          tft.fillScreen(ST7735_WHITE); //tftLCD화면을 하얀색으로 초기화한다
    if (31 > dust105) {
       tft.setCursor(0, 30);
      tft.setTextColor(ST7735_RED);
      tft.setTextSize(2);
      tft.println("Very Good!");
    }

    else if (80 >= dust105) {
      for (int i = 0; i < 100; i++) { //이미지 출력 하기위해 이중for문을 돌린다
        for (int j = 0; j < 100; j++) {
          tft.drawPixel(j + 14, i + 14, IMG_2[i * 100 + j]);
        }
      }
    }
    else if (150 >= dust105) {
      for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
          tft.drawPixel(j + 14, i + 14, IMG_3[i * 100 + j]);
        }
      }
    }
    else {
      tft.setCursor(0, 30);
      tft.setTextColor(ST7735_RED);
      tft.setTextSize(2);
      tft.println("Very Dangerous");
    }
  }
  else {
    int averagedust = ThingSpeak.readFloatField(myChannelNumber, 1);
    int averagetemp = ThingSpeak.readFloatField(myChannelNumber, 2);
    Serial.print("Dust is: ");
    Serial.println(averagedust);
    Serial.print("Temp is: ");
    Serial.println(averagetemp);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DUST : ");
    lcd.setCursor(7, 0);
    lcd.print(averagedust);
    lcd.setCursor(11, 0);
    lcd.setCursor(0, 1);
    lcd.print("TEMP : ");
    lcd.setCursor(7, 1);
    lcd.print(averagetemp);
    delay(100);
          tft.fillScreen(ST7735_WHITE);
    if (31 > averagedust) {
      tft.setCursor(0, 30);
      tft.setTextColor(ST7735_RED);
      tft.setTextSize(2);
      tft.println("Very Good!");
    }

    else if (80 >= averagedust) { //아두이노 변수선언을 하기위한 메모리 부족으로 인해 그림파일은 2개만 사용하고 나머지는 텍스트로 표현함
      for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
          tft.drawPixel(j + 14, i + 14, IMG_2[i * 100 + j]);
        }
      }
    }
    else if (150 >= averagedust) {
      for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
          tft.drawPixel(j + 14, i + 14, IMG_3[i * 100 + j]);
        }
      }
    }
    else {
      tft.setCursor(0, 30);
      tft.setTextColor(ST7735_RED);
      tft.setTextSize(2);
      tft.println("Very Dangerous");
    }
  }
  voice = ""; //한루프문 돌때마다 voice 초기화
}
