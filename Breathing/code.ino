#include "WiFiS3.h"

char ssid[] = "iPhone";
char password[] = "password";

WiFiServer server(80);
int status = WL_IDLE_STATUS;

const int threadPin = A0;
int threshold = 188;

bool wasAbove = false;
bool isBreathing = false;

int breathCount = 0;
int currBpm = 0;
int avgBpm = 0;

unsigned long bpmStartTime = 0;

void setup() {
  Serial.begin(9600);
  delay(2000);
  
  Serial.println("Starting Arduino breathing monitor...");
  Serial.println("Connecting to WiFi...");
  
  while (status != WL_CONNECTED) {
    Serial.print("Trying network: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
    delay(5000);
  }

  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
  bpmStartTime = millis();
}
void loop() {
  int sensorValue = analogRead(threadPin);
  bool isAbove = sensorValue > threshold;

  isBreathing = isAbove;
  if (isAbove && !wasAbove) {
    breathCount++;
    Serial.println("Breath detected");
  }
  wasAbove = isAbove;
  
  if (millis() - bpmStartTime >= 10000) {
    currBpm = breathCount * 6;
    if (avgBpm == 0) {
      avgBpm = currBpm;
    } else {
      avgBpm = (avgBpm + currBpm) / 2;
    }
    breathCount = 0;
    bpmStartTime = millis();
  }
  
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 2000) {  // every 2 seconds
    Serial.print("Sensor: ");
    Serial.print(sensorValue);
    Serial.print(" | BPM: ");
    Serial.print(currBpm);
    Serial.print(" | Breathing: ");
    Serial.println(isBreathing ? "YES" : "NO");
    lastPrint = millis();
  }

  WiFiClient client = server.available();
  if (client) {
    while (client.connected() && !client.available()) {
      delay(1);
    }
    client.readStringUntil('\r');

    client.flush();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.print("<!DOCTYPE html><html><head>");
    client.print("<meta charset='UTF-8'>");
    client.print("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.print("<meta http-equiv='refresh' content='2'>");
    client.print("<title>Breathing Sensor</title>");
    client.print("<style>");
    client.print("body{font-family:Arial;text-align:center;background:#BBE1ED;margin:20px;}");
    client.print(".container{max-width:600px;margin:auto;padding:20px;border-radius:20px;background:white;}");
    client.print(".yes{border:5px solid #2ecc71;}");
    client.print(".no{border:5px solid #e74c3c;}");
    client.print(".value{font-size:1.4rem;margin:15px;}");
    client.print(".green{color:#2ecc71;font-weight:bold;}");
    client.print(".red{color:#e74c3c;font-weight:bold;}");
    client.print("</style>");
    client.print("</head><body>");

    if (isBreathing) {
      client.print("<div class='container yes'>");
    } else {
      client.print("<div class='container no'>");
    }

    client.print("<h1>Patient Monitor</h1>");
    client.print("<h2>Breathing Sensor</h2>");
    client.print("<div class='value'>Raw Sensor Value: ");
    client.print(sensorValue);
    client.print("</div>");
    client.print("<div class='value'>Current BPM: ");
    client.print(currBpm);
    client.print("</div>");
    client.print("<div class='value'>Average BPM: ");
    client.print(avgBpm);
    client.print("</div>");
    client.print("<div class='value'>Breathing: ");

    if (isBreathing) {
      client.print("<span class='green'>YES</span>");
    } else {
      client.print("<span class='red'>NO</span>");
    }

    client.print("</div>");
    client.print("<p>Page refreshes every 2 seconds.</p>");
    client.print("</div></body></html>");
    client.stop();
  }
  delay(100);
}

