#include <ESP8266WebServer.h>
#include <FS.h>
//#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include "time.h"
#include <Ticker.h>  //Ticker Library

ESP8266WiFiMulti wifiMulti;
MDNSResponder mdns;

#define COUNT_FILE_MAX 100
#define LENGTH_PATH_MAX 10
#define SBUFF_COUNT_MAX 128

#define RED_LED 4
#define GREEN_LED 5
#define SwithPin 12

#define TZ              1       // (utc+) TZ in hours
#define DST_MN          60      // use 60mn for summer time in some countries

#define RTC_TEST     1510592825 // 1510592825 = Monday 13 November 2017 17:07:05 UTC

////////////////////////////////////////////////////////

#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

const char* ssid = "Dose.IO";
const char* password = "12345678";

const char* www_username = "admin";
const char* www_password = "admin";

const String OwnHost ="192.168.4.1";

String host = OwnHost;

int LedNotification = 0;
int SoundNotification = 0;
int FlagNotificationAlarm = 0;
int FlagOpenCase = 0;
time_t NextNotification = 0;
time_t NextNotification_last = 0;
int CurrentSwith = 0;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer  server(80);

IPAddress gateway(192, 168, 4, 1);

WiFiUDP Udp;
unsigned int localUdpPort = 4210;
char incomingPacket[255];
char  replyPacekt[] = "Hi there! Got the message :-)";

HTTPClient http;

enum ESP_CODE{
  ESP_NULL=0,
  ESP_OK,
  ESP_NEW_IP,
  ESP_CONNECTING,
  ESP_CONNCECT_OK,
  ESP_CONNECT_ERROR,
  ESP_STA,
  ESP_AP,
  ESP_IP,
  ESP_READ_FILE,
  ESP_WRITE_FILE,
  ESP_NEW_CODE,
  STM_NEW_CODE, 
  SD_READ_ROOT,
  SD_READ_FOLDER,
  SD_LAST_CODE,
  SD_NEW_CODE
};

// Р вЂ”Р Т‘Р ВµРЎРѓРЎРЉ РЎвЂћРЎС“Р Р…Р С”РЎвЂ Р С‘Р С‘ Р Т‘Р В»РЎРЏ РЎР‚Р В°Р В±Р С•РЎвЂљРЎвЂ№ РЎРѓ РЎвЂћР В°Р в„–Р В»Р С•Р Р†Р С•Р в„– РЎРѓР С‘РЎРѓРЎвЂљР ВµР С�Р С•Р в„–
String getContentType(String filename) {
 if (server.hasArg("download")) return "application/octet-stream";
 else if (filename.endsWith(".htm")) return "text/html";
 else if (filename.endsWith(".html")) return "text/html";
 else if (filename.endsWith(".css")) return "text/css";
 else if (filename.endsWith(".js")) return "application/javascript";
 else if (filename.endsWith(".png")) return "image/png";
 else if (filename.endsWith(".gif")) return "image/gif";
 else if (filename.endsWith(".jpg")) return "image/jpeg";
 else if (filename.endsWith(".ico")) return "image/x-icon";
 else if (filename.endsWith(".xml")) return "text/xml";
 else if (filename.endsWith(".pdf")) return "application/x-pdf";
 else if (filename.endsWith(".zip")) return "application/x-zip";
 else if (filename.endsWith(".gz")) return "application/x-gzip";
 return "text/plain";
}

struct serial{
  char SBUFF[SBUFF_COUNT_MAX]={0};
  int SBUFF_COUNT=0;
  bool START=false;
  int RX_CODE=ESP_NULL;
  int TX_CODE=ESP_NULL;
  int COUNT_PACKAGE=0;
  int COUNT_DATA;
};

struct flash{
char SD_LIST_FILE[COUNT_FILE_MAX][15]={{0}};
int COUNT_WRITE=0;
int COUNT_WRITE_MAX=0;
String PATCH="/Log";
char Patch[6][13]={{0}};
bool PATCH_EMPTY=false;
};

#define NamePill_MAX 10

struct s_SettigsDoseIO
{
  union{
        uint8_t All[NamePill_MAX+4+1];
        struct{
        char  NamePill[NamePill_MAX];   //Part number
        int   FreqOfTakingThePill;      //frequency of taking the pill
        bool  SoundNotification;        //Enable sound notification
        bool  LedNotification;          //Enable led notification
    };
  };
};

serial USART;
flash MicroSd;
s_SettigsDoseIO SettigsDoseIO;
Ticker blinker;

void HTML_WIFI()
{
  String Str = "<!DOCTYPE html>\n\
              <html>\n\
                <head>\n\
                  <meta charset=\"utf-8\">\n\
                  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge,chrome=1\">\n\
                  <title>ESP8266</title>\n\
                  <link href=\"\" rel=\"stylesheet\">\n\
                  <style>#navbar ul{display: none;background-color: #f90;position: absolute;top: 100%;}#navbar li:hover ul { display: block; }#navbar, #navbar ul{margin: 0;padding: 0;list-style-type: none;}#navbar {height: 30px;background-color: #666;padding-left: 25px;min-width: 470px;}#navbar li {float: left;position: relative;height: 100%;}#navbar li a {display: block;padding: 6px;width: 100px;color: #fff;text-decoration: none;text-align: center;}#navbar ul li { float: none; }#navbar li:hover { background-color: #f90; }#navbar ul li:hover { background-color: #666; }</style>\n\
                </head>\n\
                <body>\n\
                  <ul id=\"navbar\">\n\
                    <li><a href=\"http://"+host+"/index\">Главная</a></li>\n\
                    <li><a href=\"http://"+host+"/graph\">Графики</a></li>\n\
                    <li><a href=\"#3\">Таблица</a>\n\
                      <ul>\n\
                        <li><a href=\"http://"+host+"/table\">Текущий</a></li>\n\
                        <li><a href=\"#32\">Телефон</a></li>\n\
                        <li><a href=\"#33\">Email</a></li>\n\
                      </ul>\n\
                    </li>\n\
                    <li><a href=\"http://"+host+"/wifi\">WIFI</a></li>\n\
                  </ul>\n\
                   <form action=\"reconnect\">\n\
                    <p><strong>Название WIFI сети:</strong>\n\ 
                    <input maxlength=\"25\" size=\"40\" name=\"login\"></p>\n\
                    <p><strong>Пароль:</strong>\n\ 
                    <input type=\"password\" maxlength=\"25\" size=\"40\" name=\"password\"></p>\n\
                    <input type=\"submit\" name=\"submit\" value=\"Подключиться\">\n\
                  </form>\n\
                </body>\n\
            </html>";
 server.send(200, "text/html", Str);
}

void COMPIL_PATCH(){
  MicroSd.PATCH="";
  for(int i=0;i<MicroSd.Patch[0][0];i++)MicroSd.PATCH+="/"+String(MicroSd.Patch[i+1]);
}

void WIFI_SET_AP ()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(gateway, gateway, IPAddress(255,255,255,0));
  WiFi.softAP(ssid, password);

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "dose.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("dose")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  //Serial.println("AP");
  delay(5000);

  // Connect to WiFi network
  Serial.println();
  
  Serial.print("Connecting to ");
  Serial.println(ssid);

  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Start TCP (HTTP) server
  server.begin();

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void WIFI_SET_STA(const char* Ssid,const char* Password) {
  int count=0;
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  delay(100);

  MDNS.begin("Dose");
  MDNS.addService("http", "tcp", 80);
  
  WiFi.begin(Ssid, Password);
  Serial.print("Connecting to ");
  Serial.println(Ssid);
  Serial.println(Password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(count<20)count++;
    else {
      Serial.println("Error connect!");
      WIFI_SET_AP ();
      break;
    }
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
  if(WiFi.localIP()){
    host="";
    host+=String(WiFi.localIP()[0]) + ".";
    host+=String(WiFi.localIP()[1]) + ".";
    host+=String(WiFi.localIP()[2]) + ".";
    host+=String(WiFi.localIP()[3]);
    //Serial.println("[0]"+host);
  }
}

void Reconnect()
{
  String message, sid, pas;
  for (uint8_t i = 0; i < server.args(); i++) 
  {
    if(server.argName(i).indexOf("login")!=-1)sid=server.arg(i);
    if(server.argName(i).indexOf("password")!=-1)pas=server.arg(i);
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  Serial.println(message);
  char s[sid.length()];
  char p[pas.length()];
  for(int j=0;j<sid.length();j++) s[j]=sid[j];
  for(int j=0;j<pas.length();j++) p[j]=pas[j];
  server.send(200, "text/plain","Connecting to "+sid +"\n");
  if(sid!=0 && pas!=0)WIFI_SET_STA(s,p);
}


#define PTM(w) \
  Serial.print(":" #w "="); \
  Serial.print(tm->tm_##w);

void printTm(const char* what, const tm* tm) {
  Serial.print(what);
  PTM(isdst); PTM(yday); PTM(wday);
  PTM(year);  PTM(mon);  PTM(mday);
  PTM(hour);  PTM(min);  PTM(sec);
}

time_t now;
struct tm * timeinfo;

void changeState()
{
  if(LedNotification)
  {
//    digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));  //Invert Current State of LED_BUILTIN  
    digitalWrite(RED_LED, !(digitalRead(RED_LED)));  //Invert Current State of LED_BUILTIN  
    digitalWrite(GREEN_LED, !(digitalRead(RED_LED)));  //Invert Current State of LED_BUILTIN  
  }
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  
  pinMode(GREEN_LED,OUTPUT); 
  pinMode(RED_LED,OUTPUT); 

  pinMode(SwithPin,INPUT); 
  
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH 
  
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  time_t rtc = RTC_TEST;
  timeval tv = { rtc, 0 };
  timezone tz = { TZ_MN + DST_MN, 0 };
  settimeofday(&tv, &tz);

  now = time(nullptr);
  timeinfo = localtime(&now);

  char buffer[80];
  strftime (buffer,80,"%D %I:%M:%S %p",timeinfo);
  Serial.println(buffer);
//  Serial.println();
//  printTm("localtime", localtime(&now));
//  Serial.println();
//  printTm("gmtime   ", gmtime(&now));
//  Serial.println();

  COMPIL_PATCH();
  
  Serial.begin(115200);
  
  //Initialize File System
  if(SPIFFS.begin())
  {
    Serial.println("SPIFFS Initialize....ok");
  }
  else
  {
    Serial.println("SPIFFS Initialization...failed");
  }
  
  delay(10);

  WIFI_SET_AP();

  server.on("/index", HTTP_GET, HTML_INDEX);
//  server.on("/wifi", HTTP_GET, HTML_WIFI);
//  server.on("/reconnect", HTTP_GET, Reconnect);
  server.on("/ledon", HTTP_GET, [](){
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  });
  server.on("/ledoff", HTTP_GET, [](){
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage HIGH
  });

  server.on("/", HTTP_GET, HTML_INDEX);
  
  server.onNotFound([](){
    server.send(404, "text/plain", "NotFound Uri "+ server.uri()+"\n");
  });

  Udp.begin(localUdpPort);

   // Start the server
//  server.begin();

  blinker.attach(0.25, changeState);

//  File f_copy = SPIFFS.open("/auth.txt", "w+");
//  if (f_copy) {
//    while(f_copy.available()) {
//      //Lets read line by line from the file
//      String line = f_copy.readStringUntil('\n');
//      if(line)
//      {
//        time_t CurrentTime = atol((const char*)&line);
//        char lineChar[100];
//        if(NextNotification <= 1 || NextNotification > CurrentTime) NextNotification = CurrentTime;
//        sprintf(lineChar,"%lld", (long long int)CurrentTime);
//        f.println(lineChar);
//        Serial.println(lineChar);
//      }
//    }
//    f.close();
}

void loop() {

  MDNS.update();
  
 server.handleClient();

 int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    if(incomingPacket == strstr(incomingPacket,"LedOn"))
    {
      LedNotification = 1;
    }
    else if(incomingPacket == strstr(incomingPacket,"LedOff"))
    {
      LedNotification = 0;
      digitalWrite(RED_LED, LOW);  //Invert Current State of LED_BUILTIN  
      digitalWrite(GREEN_LED, LOW);  //Invert Current State of LED_BUILTIN  
    }
    else if(incomingPacket == strstr(incomingPacket,"UpdateTime"))
    {
      time_t SetNow = 0;
      char* PointerData = &incomingPacket[0] + 11;

      for(int i = 11; i < len; i++)
      {
        SetNow *= 10;
        SetNow += incomingPacket[i] - '0';
      }
      
      timeval tv = { SetNow, 0 };
      timezone tz = { 0, 0 };
      settimeofday(&tv, &tz);
      
      timeinfo = localtime(&SetNow);
    
      char buffer[80];
      strftime (buffer,80,"%D %I:%M:%S %p",timeinfo);
      Serial.println(buffer);
    }
    else if(incomingPacket == strstr(incomingPacket,"SetNextNotification"))
    {
      char* PointerData = &incomingPacket[0] + 20;
      time_t SetNow = 0;

      SetNow = atol((const char*)&incomingPacket[20]);

      Serial.print("SetNow = ");
      Serial.println(SetNow);

      if(SetNow)
      {
        int FlagNoWrite = 0;
        if(SetNow > now && ((SetNow < NextNotification && NextNotification > 1) || NextNotification <= 1))
        {
          NextNotification = SetNow;
        }
        File f = SPIFFS.open("/NextNotification.txt", "a+");
        if (!f) {
          Serial.println("file creation failed");
        }
        while(f.available() && FlagNoWrite == 0) {
          //Lets read line by line from the file
          String line = f.readStringUntil('\n');
          time_t CurrentTime = atol((const char*)&line);
          if(CurrentTime == SetNow || SetNow < now) FlagNoWrite = 1;
        }

        if(SetNow < now) FlagNoWrite = 1;
        
        if(FlagNoWrite == 0)f.println(&incomingPacket[20]);
        f.close();
        
        f = SPIFFS.open("/NextNotification.txt", "r");

        Serial.println("/NextNotification.txt");
        while(f.available()) {
          //Lets read line by line from the file
          String line = f.readStringUntil('\n');
          Serial.println(line);
        }
        
        f.close();
      }
      
      File f = SPIFFS.open("/Journal.txt", "a");
      if (!f) {
        Serial.println("file creation failed");
      }

      now = time(nullptr);
      timeinfo = localtime(&now);
    
      char buffer[80];
      strftime (buffer,80,"%D %I:%M:%S %p|",timeinfo);
      f.print(buffer);
      
      // now write two lines in key/value style with  end-of-line characters
      f.println(incomingPacket);
      f.close();
    }
    else if(incomingPacket == strstr(incomingPacket,"updateStruct"))
    {
      char* PointerStruct = &incomingPacket[0] + 13;

      if(PointerStruct)
      {
        for(int i = 0; i < sizeof(s_SettigsDoseIO); i++)
        {
          SettigsDoseIO.All[i] = 0;
          if((*(PointerStruct + (2*i))) >= '0' && (*(PointerStruct + (2*i))) <= '9')
          {
            SettigsDoseIO.All[i] += ((*(PointerStruct + (2*i)))-'0')*10;
          }
          else if((*(PointerStruct + (2*i))) >= 'A' && (*(PointerStruct + (2*i))) <= 'F')
          {
            SettigsDoseIO.All[i] += ((*(PointerStruct + (2*i)))-'A')*10;
          }

          if((*(PointerStruct + (2*i+1))) >= '0' && (*(PointerStruct + (2*i+1))) <= '9')
          {
            SettigsDoseIO.All[i] += (*(PointerStruct + (2*i)))-'0';
          }
          else if((*(PointerStruct + (2*i+1))) >= 'A' && (*(PointerStruct + (2*i+1))) <= 'F')
          {
            SettigsDoseIO.All[i] += (*(PointerStruct + (2*i)))-'A';
          }
        }
      }
      
      // open file for writing
      File f = SPIFFS.open("/settings.txt", "w+");
      if (!f) {
          Serial.println("file open failed");
      }
      Serial.println("====== Writing to SPIFFS file =========");
      // write strings to file
      if(PointerStruct) f.println(PointerStruct);
      f.close();

      f = SPIFFS.open("/settings.txt", "r");
      String line = f.readStringUntil('\n');
      Serial.println(line);
      f.close();
    }
    else if(incomingPacket == strstr(incomingPacket,"UpdateEvents"))
    {
      // this opens the file "f.txt" in read-mode
      File f = SPIFFS.open("/Journal.txt", "r");
      
      if (f) {
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        // we could open the file
        while(f.available()) {
          //Lets read line by line from the file
          char lineChar[100];
          String line = f.readStringUntil('\n');
          Serial.println(line);
          line.toCharArray(lineChar,line.length());
          lineChar[line.length()] = '\n';
          lineChar[line.length()+1] = '\0';
          Udp.write(lineChar,line.length()+2);
        }
        Udp.endPacket();
      }

      f.close();

      SPIFFS.remove("/Journal.txt");
      Serial.println("remove file Journal.txt");
    }
    else if(incomingPacket == strstr(incomingPacket,"OpenCase"))
    {
      FlagOpenCase = 1;
    }
    else if(incomingPacket == strstr(incomingPacket,"DelAllNotification"))
    {
      SPIFFS.remove("/NextNotification.txt");
      Serial.println("remove file Journal.txt");
      NextNotification = 1;
    }
    else if(incomingPacket == strstr(incomingPacket,"DelNotification"))
    {
      char* PointerData = &incomingPacket[0] + 16;
      time_t DelNotif = 0;

      DelNotif = atol((const char*)&incomingPacket[16]);

      Serial.print("DelNotif = ");
      Serial.println(DelNotif);

      NextNotification = 0;

      if(DelNotif)
      {
        File f = SPIFFS.open("/NextNotification.txt", "r");
        
        if (f) {
          File f_copy = SPIFFS.open("/NextNotification_copy.txt", "w");
          if (f_copy) {
            Serial.println("/NextNotification_copy.txt");
            while(f.available()) {
              //Lets read line by line from the file
              String line = f.readStringUntil('\n');
              if(line)
              {
                time_t CurrentTime = atol((const char*)&line);
                if(CurrentTime != DelNotif)
                {
                  char lineChar[100];
                  sprintf(lineChar,"%lld", (long long int)CurrentTime);
                  f_copy.println(lineChar);
                  Serial.println(lineChar);

                  if(CurrentTime > now && CurrentTime < NextNotification) NextNotification = CurrentTime;
                }
              }
            }
            f.close();
            SPIFFS.remove("/NextNotification.txt");
          }
          f_copy.close();
        }
    
        File f_copy = SPIFFS.open("/NextNotification_copy.txt", "r");
        if (f_copy) {
          f = SPIFFS.open("/NextNotification.txt", "w");
          Serial.println("/NextNotification.txt");
          while(f_copy.available()) {
            //Lets read line by line from the file
            String line = f_copy.readStringUntil('\n');
            if(line)
            {
              time_t CurrentTime = atol((const char*)&line);
              char lineChar[100];
              if(NextNotification <= 1 || NextNotification > CurrentTime) NextNotification = CurrentTime;
              sprintf(lineChar,"%lld", (long long int)CurrentTime);
              f.println(lineChar);
              Serial.println(lineChar);
            }
          }
          f.close();
        }
    
        f_copy.close();
        SPIFFS.remove("/NextNotification_copy.txt");
      }
    }
    else if(incomingPacket == strstr(incomingPacket,"ReadAllNotification"))
    {
      // this opens the file "f.txt" in read-mode
      File f = SPIFFS.open("/NextNotification.txt", "r");
      
      if (f) {
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        // we could open the file
        while(f.available()) {
          //Lets read line by line from the file
          char lineChar[100];
          String line = f.readStringUntil('\n');
          Serial.println(line);
          sprintf(lineChar,"%s\n",&line);
          Udp.write(lineChar,strlen(lineChar));
        }
        Udp.endPacket();
      }

      f.close();
    }
    else if(incomingPacket == strstr(incomingPacket,"ReadNextNotification"))
    {
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      char lineChar[100];
      sprintf(lineChar, "%lld",NextNotification);
      Udp.write(lineChar,strlen(lineChar));
      Udp.endPacket();
      Serial.println(lineChar);
    }
    else
    {
      Serial.println("-------No Parse Packet-------");
    }

    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
//    char Str[(sizeof(s_SettigsDoseIO) * 2) + 1] = {0};
//    SettigsDoseIO.FreqOfTakingThePill = 5;
//    SettigsDoseIO.SoundNotification = 1;
//    SettigsDoseIO.LedNotification = 1;
//    memcpy(SettigsDoseIO.NamePill,"0123456789",10);
//    for(int i = 0; i < sizeof(s_SettigsDoseIO); i++)
//    {
//      sprintf(&Str[i*2],"%x",((uint8_t*)(&SettigsDoseIO.All))[i]);
//    }
////    sprintf(Str,"%x",SettigsDoseIO.All);
    Udp.write("Echo:");
    Udp.write(incomingPacket,len);
    Udp.endPacket();
  }

  now = time(nullptr);
  if(NextNotification == 0)
  {
    File f = SPIFFS.open("/NextNotification.txt", "r");
    if (f) 
    {
      while(f.available() && NextNotification <= 1) {
        //Lets read line by line from the file
        String line = f.readStringUntil('\n');
        if(line)
        {
          time_t CurrentTime = atol((const char*)&line);
          if(CurrentTime > now)
          {
            NextNotification = CurrentTime;
            Serial.print("NextNotification = ");
            Serial.println(CurrentTime);
          }
        }
      }
    }
    else
    {
      NextNotification = 1;
      Serial.println("file \"NextNotification\" read failed");
    }
    f.close();
  }
  else if(now >= NextNotification && (now - NextNotification) <= 3600)
  {
    if(FlagNotificationAlarm == 0)
    {
      FlagNotificationAlarm = 1;
      
      Serial.print("NextNotification now=");
      Serial.print(now);
      Serial.print(" NextNotification=");
      Serial.println(NextNotification);

      File f = SPIFFS.open("/Journal.txt", "a+");
      if (!f) {
        Serial.println("file creation failed");
      }

      timeinfo = localtime(&now);
    
      char buffer[80];
      strftime (buffer,80,"%D %I:%M:%S %p|",timeinfo);
      f.print(buffer);
      
      // now write two lines in key/value style with  end-of-line characters
      f.println("Time for pill");
      f.close();
    }
  }

 if(FlagNotificationAlarm)
 {
    LedNotification = 1;
 }

  if(digitalRead(SwithPin) == HIGH)
 {
  if(CurrentSwith == LOW) FlagOpenCase = 1;
  CurrentSwith = HIGH;
  digitalWrite(LED_BUILTIN, LOW);  //Invert Current State of LED_BUILTIN 
 }
 else
 {
  CurrentSwith = LOW;
  digitalWrite(LED_BUILTIN, HIGH);  //Invert Current State of LED_BUILTIN 
 }

 if(FlagOpenCase)
 {
    if(FlagNotificationAlarm == 1)
    {
      File f = SPIFFS.open("/NextNotification.txt", "r");
      
      if (f) {
        File f_copy = SPIFFS.open("/NextNotification_copy.txt", "w");
        if (f_copy) {
          Serial.println("/NextNotification_copy.txt");
          while(f.available()) {
            //Lets read line by line from the file
            String line = f.readStringUntil('\n');
            if(line)
            {
              time_t CurrentTime = atol((const char*)&line);
              if(CurrentTime != NextNotification && CurrentTime > now)
              {
                char lineChar[100];
                sprintf(lineChar,"%lld", (long long int)CurrentTime);
                f_copy.println(lineChar);
                Serial.println(lineChar);
              }
            }
          }
          f.close();
          SPIFFS.remove("/NextNotification.txt");
        }
        f_copy.close();
      }
  
      NextNotification = 0;
  
      File f_copy = SPIFFS.open("/NextNotification_copy.txt", "r");
      if (f_copy) {
        f = SPIFFS.open("/NextNotification.txt", "w");
        Serial.println("/NextNotification.txt");
        while(f_copy.available()) {
          //Lets read line by line from the file
          String line = f_copy.readStringUntil('\n');
          if(line)
          {
            time_t CurrentTime = atol((const char*)&line);
            char lineChar[100];
            if(NextNotification <= 1 || NextNotification > CurrentTime) NextNotification = CurrentTime;
            sprintf(lineChar,"%lld", (long long int)CurrentTime);
            f.println(lineChar);
            Serial.println(lineChar);
          }
        }
        f.close();
      }
  
      f_copy.close();
      SPIFFS.remove("/NextNotification_copy.txt");
      
      FlagNotificationAlarm = 0;
      LedNotification = 0;
      SoundNotification = 0;
      if(NextNotification == 0)NextNotification = 1;

      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
    }

    File f = SPIFFS.open("/Journal.txt", "a+");
    if (f) {
      char buffer[80];
      timeinfo = localtime(&now);
      strftime (buffer,80,"%D %I:%M:%S %p|",timeinfo);
      f.print(buffer);
      f.println("OpenCase");
    }
    f.close();
    
    FlagOpenCase = 0;
 }

  if(NextNotification_last != NextNotification)
  {
    NextNotification_last = NextNotification;
    Serial.print("NextNotification = ");
    Serial.println(NextNotification);
  }

// if(Serial.available()>0)
// {
//      size_t len = Serial.available();
//      char sbuf[len];
//      Serial.readBytes(sbuf, len);
//      
// }
}


