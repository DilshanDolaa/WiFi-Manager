#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>

//Variables
int i = 0;
int statusCode;
const char* ssid = "Dilshan-WiFi";
const char* password = "dolaa@1234";


String st;
String content;
String esid;
String epass = "";


//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80
WebServer server(80);

void setup()
{

  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  Serial.println();
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  pinMode(15, INPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");


  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);


  WiFi.begin(esid.c_str(), epass.c_str());
}
void loop() {

  if ((WiFi.status() == WL_CONNECTED))
  {

    for (int i = 0; i < 10; i++)
    {
      Serial.print("Connected to ");
      Serial.print(esid);
      Serial.println(" Successfully");
      delay(100);
    }

  }
  else
  {
  }

  if (testWifi() && (digitalRead(15) != 1))
  {
    Serial.println(" connection status positive");
    return;
  }
  else
  {
    Serial.println("Connection Status Negative / D15 HIGH");
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
  delay(1000);
}


//Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void)
{
  int c = 0;
  //Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
    
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
 WiFi.softAP("SEnergy ConnectIO", "");
  //Adding PW
 Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");
  
  Serial.println("Initializing Softap for WiFi Credentials Modification");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])+ '.';
      content = "<!DOCTYPE HTML>\r\n<html><head><style><#ali {text-align: center;}><#label{display:inline-block;width: 160px;text-align: right;}><#body{margin: 0 auto;width: 360px;padding: 1em;border: 1px solid #CCC;border-radius: 1em; background-color: #6e34db;}></style><h1>SEnergy ConnectIO</h1></head>";
      content +="<body>";
      content +="<h2>Setup WiFi Credentials </h2>";
      content += "<p> <= Available Networks =>";
      content += st;
      content += "</p><form method='get' id='fm' action='setting'><label id='label'>Model No:</label><input name='model' length=64><br><br><label>Serial No:</label><input name='serial' length=64><br><br><label>IP Address:</label><input name='ipads' length=64><br><br><label>WiFi SSID: </label><input name='ssid' length=32><br><br><label>WiFi Password:</label><input name='pass' length=64><br><br><label>Description:</label></form>";
      content += "<br><br><button id='ali' onclick="">Logout</button>";
      content += "</body></html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String modelq = server.arg("model");
      String serialq = server.arg("serial");
      String ipadsq = server.arg("ipads");
      String disq = server.arg("dis");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
    
        Serial.print("IP Address :");
        Serial.println(ipadsq);
        Serial.println("");
        Serial.print("WiFi SSID :");
        Serial.println(qsid);
        Serial.println("");
        Serial.print("WiFi PASSWORD :");
        Serial.println(qpass);
        Serial.println("");
        Serial.print("Description :");
        Serial.println(disq);
      
      
        
        

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}
