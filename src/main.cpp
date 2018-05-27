#include <Arduino.h>
#include <EasyDDNS.h>
#include <ESP8266WiFi.h>

//sensitive data
const char *ssid = "your wifi SSID";
const char *password = "your wifi password";
String DNSdomain = "duck dns subdomain";
String DNStoken = "duck dns token";
int serverPort = 80;
String krakenCommand = "your http url command for activation (in GET)";
//sensitive data

WiFiServer server(serverPort);
WiFiClient client;
String krakenResponse =
    String("HTTP/1.1 200 OK\r\n") +
    "Content-Type: text/html\r\n" +
    "Connection: close\r\n" +
    "\r\n" +
    "<!DOCTYPE HTML>" +
    "<html>" +
    "Kraken is released! <br/> <br/> ----<br/>Roomba edition" +
    "</html>" +
    "\r\n";

bool getFound = false;
const int frequency = 50;
uint8_t servoPin = D1;
int pos0 = 16;
int pos1 = 30;
long timespan = 1000;
long lastBlink  = 0;
bool blinkState = false;
void setup()
{
    Serial.begin(9600);
    
    pinMode(BUILTIN_LED, OUTPUT);
    pinMode(servoPin, OUTPUT);
    digitalWrite(BUILTIN_LED, blinkState);

    analogWriteRange(255);
    analogWriteFreq(frequency);
    analogWrite(servoPin, pos0);
    
    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    delay(500);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        Serial.print(WiFi.status());
    }
    Serial.println(WiFi.localIP());
    server.begin();

    EasyDDNS.service("duckdns");
    EasyDDNS.client(DNSdomain, DNStoken);
    Serial.printf("Kraken Rommba ready...");
}

void releaseTheKraken()
{
    Serial.println("Releasing the kraken...");
    for (int i = pos0; i < pos1; i++)
    {
        analogWrite(servoPin, i);
        delay(30);
    }
    for (int i = pos1; i > pos0; i--)
    {
        analogWrite(servoPin, i);
        delay(30);
    }
}

void blinkOnce()
{
    if((millis()- lastBlink) > timespan )
    {
        lastBlink = millis();
        blinkState = !blinkState;
        digitalWrite(BUILTIN_LED, blinkState);
    }
}

void checkServer()
{
    client = server.available();
    if (client)
    {
        getFound = false;
        Serial.println("Client connected");
        while (client.connected())
        {
            if (client.available())
            {
                String line = client.readStringUntil('\r');
                Serial.print(line);
               
               if(!getFound )
               {
                    getFound = true;
                    if (line.indexOf(krakenCommand) > 0)
                    {
                        releaseTheKraken();
                    }
               }
                if (line.length() == 1 && line[0] == '\n')
                {
                    client.println(krakenResponse);
                    break;
                }
            }
        }
        delay(100);
        client.stop();
        Serial.println("Client disonnected");
    }
}

void loop()
{
    blinkOnce();
    EasyDDNS.update(10000);
    checkServer();
}