#include <ESP8266WiFi.h>       // ESP8266WiFi.h library
#include <SoftwareSerial.h> // SoftwareSerial library (사용 안함) 
#include <dht11.h>           
#define DHT11PIN  // D1 PIN NUMBER : 3

dht11 DHT11; // DHT 11을 사용
Unsigned char  pms[32]; // 32바이트 값을 넣기 위해 사용 , pms7003은 32바이트 값을 받아온다.
//WIFI 통신을 위한 접속 단계
const char* ssid     = "iptime1304";  // 공유기 ID      
const char* password = "---password---"; // PASSWORD
const char* host = "api.thingspeak.com"; // Thingspeak address
const char* writeAPIKey = "3H5GBLY4DZZ6DNEE"; // Private key
int cooler = 2;  // NOT USED
int Relay = 16;  // D1 PIN NUMBER : 2
int PM10 = 0;
int led1 = 14;  //B  // D1 PIN NUMBER : 5
int led2 = 12;  //G  // D1 PIN NUMBER : 6
int led3 = 13;  //R // D1 PIN NUMBER : 7
int check = 100;
int ex[1];
void setup() {   
  // normal Setting
  // 기본 셋팅입니다. digitalWrite를 많이 사용하였고 초기 단계입니다.
  Serial.begin(9600);     
  pinMode(cooler, OUTPUT); 
  pinMode(Relay, OUTPUT);
pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  //  Connect to WiFi network
  WiFi.begin(ssid, password); // Wifi를 시작하기 위해, id와 비밀번호 
  digitalWrite(led1, LOW);  // 초기에는 LED를 점등
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  while (WiFi.status() != WL_CONNECTED) {  // 연결 상태 여부를 파악합니다. 0.5 간에 센싱
    delay(500);
  }
} // SETUP 끝
void loop() {
  int chk = DHT11.read(DHT11PIN);  // ckh 변수 선언입니다.

  if (Serial.available() >= 32) {    // 들어온 값이 32 btye 값이 들어오면 사용합니다.

    for (int j = 0; j < 32 ; j++) {    // pms 에 값들을 입력한다.
      pms[j] = Serial.read();
    }
    // 저희가 사용하는 PM_10 값은 15 16 번째 ! 기본 PMS7003 책자 확인하시면 32바이트에 무엇이 들어가는지 다나와있으실겁니다.
    if (pms[0] == 0x42) {   // 오류 검출, 첫 값이 0x42 라면
      PM10 = (pms[14] << 8)  | pms[15]; // Shift 연산으로 15번째 값이 왼쪽으로 8칸이동 
      // PM_10 값은 15번째와 16번째로 들어옵니다. 15 16 형태를 만들기 위해서 8번 쉬프트연산 해준겁니다.

      delay(5000);
    }
     else {                  // 오류 검출 , 처음 들어오는 값이 0x42가 아니면, error
      Serial.println("error");
    }
  }
  while (Serial.available())  //D1의 스택에러 방지용으로 만들었습니다.
  {                             
    Serial.read();
  }
  Serial.println("PM10 =");  // PM 10 값출력
  Serial.println(PM10);
  Serial.println("-----------");
 int dust1 = 30;   // 임의의 값 보내기 105호 dust 
  int temp1 = 25;   // 임의의 값 보내기 105호 temp
  // make TCP connections
  WiFiClient client; // wificlient  변수 명시
   const int httpPort = 80;  // 포트 80  http 일겁니다.
  if (!client.connect(host, httpPort)) {
    return;
  }
  String url = "/update?key=";  // 일반적으로 저희가 사용하는 Thingspeak는 필드가 나뉘어져 있습니다.
  url += writeAPIKey;
  url += "&field1=";
  url += String(dust1);   // field1 에는 임의의 105호 미세먼지 값    
  url += "&field2=";      // field2 에는 임의의 105호 온도 값 
  url += String(temp1);
 url += "&field3=";
  url += String(PM10); // field3 에는 측정된 107호 미세먼지 값 
  url += "&field4=";     
  url += String(DHT11.temperature); // field4 에는 측정된 107호 온도 값 
  url += "\r\n";
   // Request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +  // Client 연결 방식입니다. 
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  if (PM10 > 80) // 나쁨 상태입니다. 150 부터는 매우 안좋은 상태입니다.
  {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);// LED 색 RED
    digitalWrite(Relay, HIGH); // Cooler 작동
  }
  else if (PM10 > 30)  //보통 단계입니다.
  {
    digitalWrite(led1, LOW); 
    digitalWrite(led2, HIGH); // LED 색 GREEN
    digitalWrite(led3, LOW);
    digitalWrite(Relay, HIGH); // Cooler 작동
  }
Serial.print("Temperature (°C): ");
  Serial.println((float)DHT11.temperature, 2);
  delay(20000);

}



