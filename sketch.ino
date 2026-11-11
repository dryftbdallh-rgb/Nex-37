#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h> // مكتبة الواي فاي

// إعدادات الشاشة
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// تعريف الأطراف
const int relayPin = 4;  // طرف الريلاي متوصل بـ 4
const int buttonPin = 5; // طرف الزر متوصل بـ 5

// متغير لحفظ حالة الريلاي
bool relayState = false; 

// بيانات شبكة الواي فاي (غيرها لو حبيت)
const char* ssid = "Wokwi-GUEST"; // دي شبكة المحاكي الجاهزة
const char* password = "";

// إنشاء سيرفر ويب على منفذ 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  // تشغيل الشاشة على أطراف 8 و 9
  Wire.begin(8, 9);
  
  // إعداد الأطراف
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 
  
  // إطفاء الريلاي في البداية
  digitalWrite(relayPin, HIGH); 

  // تشغيل الشاشة
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  updateDisplay();

  // --- الاتصال بالواي فاي ---
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // ده الـ IP اللي هتفتحه من المتصفح
  
  server.begin(); // بدء السيرفر
}

void loop() {
  // --- 1. التحكم بالزر الفيزيائي ---
  if (digitalRead(buttonPin) == LOW) {
    relayState = !relayState; 
    digitalWrite(relayPin, relayState ? LOW : HIGH);
    updateDisplay(); 
    delay(500); 
  }

  // --- 2. التحكم عن طريق الويب (النت) ---
  WiFiClient client = server.available(); 
  if (client) {
    String request = client.readStringUntil('\r');
    
    // لو الطلب فيه /ON
    if (request.indexOf("/ON") != -1) {
      relayState = true;
      digitalWrite(relayPin, LOW); 
      updateDisplay();
    }
    // لو الطلب فيه /OFF
    else if (request.indexOf("/OFF") != -1) {
      relayState = false;
      digitalWrite(relayPin, HIGH); 
      updateDisplay();
    }

    // إرسال صفحة الويب للمتصفح
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head>");
    client.println("<body><h1>Nex 37 - Web Control</h1>");
    
    if (relayState) {
      client.println("<p>Device is currently: <b>ON</b></p>");
    } else {
      client.println("<p>Device is currently: <b>OFF</b></p>");
    }
    
    client.println("<p><a href=\"/ON\"><button style=\"padding:20px; font-size:30px;\">TURN ON</button></a></p>");
    client.println("<p><a href=\"/OFF\"><button style=\"padding:20px; font-size:30px;\">TURN OFF</button></a></p>");
    client.println("</body></html>");
    
    delay(1);
    client.stop(); 
  }
}

// دالة تحديث الشاشة
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("NEX 37");
  
  display.setTextSize(1);
  display.setCursor(10, 40);
  display.print("Status: ");
  
  if (relayState) {
    display.println("ON");
    display.println("Device is active");
  } else {
    display.println("OFF");
    display.println("Device is idle");
  }
  
  display.display();
}
