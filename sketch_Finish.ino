#include <SPI.h>
#include <Ethernet.h>
#define RHT01_PIN 0  //온,습도 비교를 할때 사용되는 온습도센서는 A0에 설정

int FAN, PUMP, BLIND, LED;  
int in1 = 7;    
int in2 = 6;    
int in3 = 5;    
int in4 = 4;      // DC모터A는 4+5번,DC모터B는 6+7번으로 설정  
int switch1 = 3;  // 스위치1을 D3번 핀에 꼽을것
int switch2 = 2;  // 스위치2를 D2번 핀에 꼽을것

int case1 = 1;    // 스위치1 high+스위치2 low   
int case2 = 3;    // 스위치1 low+스위치2 high
float temperture = 0; 
float humidity = 0;  
int signal[1] = {0};
                

byte mac[] = { 
  0xD0, 0xAF, 0x0C, 0xAF, 0xEF, 0x0F }; 
IPAddress ip(192,168,0,15);              //맥주소랑 ip 주소 시연전 확인!
EthernetServer server(80);

//온습도 센서 데이터값 입력 함수
String readString;
byte read_rht01_dat() 
{
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++)
  {
    while(!(PINF & _BV(RHT01_PIN)));  
    delayMicroseconds(30);
    if(PINF & _BV(RHT01_PIN)) 
      result |=(1<<(7-i));
    while((PINF & _BV(RHT01_PIN))); 
   }
   return result;
}
void setup() {
  pinMode(in1, OUTPUT);   
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);    // 각각의 in1, in2, in3, in4를 출력으로 설정
  pinMode(switch1, INPUT);  // 스위치1을 입력으로 설정
  pinMode(switch2, INPUT);  // 스위치2를 입력으로 설정
  pinMode(Fan, OUTPUT);
  pinMode(Pump, OUTPUT);
  pinMode(Led, OUTPUT);  //Fan,Pump,Led를 출력으로 설정

  DDRF |= _BV(RHT01_PIN);     
  PORTF |= _BV(RHT01_PIN); // 설정된 포트에 온습도 센서로 받은 데이터를 출력해주도록 설정
  Serial.begin(9600);
  
  Serial.println("Ready");
  delay(1000);
   while (!Serial) {
    ; 
  }
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
}

//이더넷 쉴드 오픈 소스
void loop() {
  
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
  
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) readString.concat(c);
        Serial.write(c);
        if (c == '\n') {
          if(readString.indexOf("W1=1")!=-1) { 
            signal[1] = 1;
            }
            else if(readString.indexOf("W1=0")!=-1) { 
            signal[1] = 0;
            }
          
          
       
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html;charset=utf-8");
          client.println("Connection: close");  
         
    
          client.println("<!DOCTYPE HTML>");
          client.println("<html><body>");
          
               
          client.println("</body>");  
          client.println("</html>");                    
          
          readString=" ";         
          break;
        }
      }
    }

    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
    
  }

  //온습도 센서 데이터에 필요한 변수
  byte rht01_dat[5];
  byte rht01_in;
  byte i;
  
  PORTF &= ~_BV(RHT01_PIN);
  delay(18);
  PORTF |= _BV(RHT01_PIN);
  delayMicroseconds(40);
  DDRF &= ~_BV(RHT01_PIN);
  delayMicroseconds(40);
  
  rht01_in = PINF & _BV(RHT01_PIN);
  if(rht01_in)
  {
    Serial.println("start condition 1 not met");
    return;
  }
  delayMicroseconds(80);
  rht01_in = PINF & _BV(RHT01_PIN);
  if(!rht01_in)
  {
    Serial.println("start condition 2 not met");
    return;
  }
  
  delayMicroseconds(80);
  for (i=0; i<5; i++)  
  rht01_dat[i] = read_rht01_dat();
  DDRF |= _BV(RHT01_PIN);
  PORTF |= _BV(RHT01_PIN);
  byte rht01_check_sum = rht01_dat[0]+rht01_dat[1]+rht01_dat[2]+rht01_dat[3];

  if(rht01_dat[4]!= rht01_check_sum) 
  {
    Serial.println("RHT01 checksum error");
  }
  
  delay(250);

  
  int S1;    // 스위치1의 입력값을 저장할 변수
  int S2;    // 스위치2의 입력값을 저장할 변수
  S1 = digitalRead(switch1);  // 스위치1로부터 받은 0 혹은 1의 값을 저장
  S2 = digitalRead(switch2);  // 스위치2로부터 받은 0 혹은 1의 값을 저장
  
  if(case1 == 1) {
    if(temperture<=st_temperture) {
   digitalWrite(Fan, LOW);
   digitalWrite(Led, HIGH);
  if(S1 == LOW && S2 == HIGH)
  { digitalWrite(in1, HIGH);
    digitalWrite(in3, LOW);               
    digitalWrite(in2, LOW);
    digitalWrite(in4, HIGH);      // 하강   
 
  }
  else if(S1 == HIGH && S2 == LOW)
  {     
    digitalWrite(in1, HIGH);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);      // 모터 정지    
  }
   
    }
    
    case1 = case1 + 1;
     }
    
  if(case1 == 2) {
    if(temperture>st_temperture) {
    digitalWrite(Fan, HIGH);
    digitalWrite(Led, LOW);
    if(S1 == HIGH && S2 == LOW)
    { digitalWrite(in2, HIGH);
      digitalWrite(in4, LOW);      
      digitalWrite(in1, LOW);            
      digitalWrite(in3, HIGH);     // 상승
  }
  else if(S1 == LOW && S2 == HIGH)
  {
    digitalWrite(in2, HIGH);
    digitalWrite(in4, LOW); 
    digitalWrite(in1, HIGH);
    digitalWrite(in3, LOW);      // 모터 정지
    
  }
  
}
  case1 = case1 - 1;
  }
  if(case2 == 3) {
    if(humidity<=st_humidity) 
    {
     digitalWrite(Pump, HIGH);
     b = 1;
    }
    case2 = case2 + 1;
  }
  if(case2 == 4) {
    if(humidity>st_humidity)
    {
     digitalWrite(Pump, LOW);  
     b = 2;
    }    
    case2 = case2 - 1;
  }
}

 
