/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *  http://creativecommons.tankerkoenig.de/json/list.php?lat=48.782307&lng=9.209079&rad=7&sort=price&type=e10&apikey=YOUR-API-KEY
 */

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

WiFiClientSecure client;

const char* ssid     = "WIFI-SSID";
const char* password = "WIFI PASSWORT";

const unsigned long BAUD_RATE = 9600;                 // serial connection speed
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response

// Key für den Zugriff auf die freie Tankerkönig-Spritpreis-API
// Für eigenen Key bitte hier https://creativecommons.tankerkoenig.de
// registrieren.
const char* server = "creativecommons.tankerkoenig.de";
const char* resource = "/json/list.php?lat=48.782307&lng=8.903507&rad=7&sort=price&type=e10&apikey=YOUR-API-KEY";

/* Wenn ich mal variabler werden will!!
const char* lat = "51.498668";    // 'lat': Breitengrad, Dezimalform
const char* lng = "8.903507";     // 'lng': Längengrad, Dezimalform
const char* rad = "7";            // Suchradius in km; erlaubter Bereich: 1 bis 25, ganze Zahl oder Dezimalzahl
const char* sort = "price";       // Sortierung; Werte 'price' oder 'dist'
const char* type = "e10";         // Spritsorte; erlaubte Werte; Werte: 'e5', 'e10', 'diesel' oder 'all' (*)
//const char* tankerkoenig_api_key = 'xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx'; // TODO: Key ersetzen
const char* tankerkoenig_api_key = "YOUR-API-KEY"; // API-Key
*/

// The type of data that we want to extract from the page
struct UserData {
  char name1[50];
  char brand1[50];
  char price1[10];
  //char isOpen1[5];
  char name2[50];
  char brand2[50];
  char price2[10];
  //char isOpen2[5];
};

elapsedMillis timeElapsed = 1200000;

String data = "";   // String with json data

void setup() {
  initSerial();
  initDisplay();
  initWifi();
}

void loop() {
  if(timeElapsed > 600000){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Aktualisiere E10");
    lcd.setCursor(0,1);
    lcd.print("Abfrage laeuft..");
    delay(2000);
    timeElapsed = 0;
    if (connect(server)) {
      if (sendRequest(server, resource) && skipResponseHeaders()) {
        UserData userData;
        if (readReponseContent(&userData)) {
          printUserData(&userData);
          printUserDataOnLCD(&userData);
        }
      }
      disconnect();
    }
  }

}

// Initialize Serial port
void initSerial() {
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    ;  // wait for serial port to initialize
  }
  Serial.println("Serial ready");
}

void initDisplay() {
  lcd.begin();
  lcd.print("Wird hochgefahren!");
}

// Initialize Wifi library
void initWifi() {
// We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  Verbinde mit");
  lcd.setCursor(0,1);
  lcd.print(ssid);

  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Verbunden! IP:");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  
  delay(1000);
  // Use WiFiClient class to create TCP connections
}



// Open connection to the HTTP server
bool connect(const char* hostName) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();
  delay(100);
  Serial.print("Connect to ");
  Serial.println(hostName);

  bool ok = client.connect(hostName, 443);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, const char* resource) {
  Serial.print("GET ");
  Serial.println(resource);
  
/*
  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.print(host);
  client.println("\r\n");
  client.print("Connection: close");
  client.println();
*/
  client.println("GET /json/list.php?lat=48.782307&lng=9.209079&rad=7&sort=price&type=e10&apikey=b53e1dff-49b4-880a-732e-f3cece8aeecf HTTP/1.1");
  client.println("Host: creativecommons.tankerkoenig.de");
  client.println("Connection: close");
  client.println();
  /*
  // Read all the lines of the reply from server and print them to Serial
  while (client.connected()) {
    data = client.readStringUntil('\r');
    Serial.print(data);
  }
*/
  return true;
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    Serial.println("No response or invalid response!");
  }

  return ok;
}

bool readReponseContent(struct UserData* userData) {
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // This is only required if you use StaticJsonBuffer.
  
  const size_t BUFFER_SIZE =
      JSON_OBJECT_SIZE(5)    // the root object has 5 elements
      + JSON_OBJECT_SIZE(72)   //stations has 6 elements
      + MAX_CONTENT_SIZE;    // additional space for strings
      
  // Allocate a temporary memory pool
  DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);

/*
  // Read all the lines of the reply from server and print them to Serial
  while (client.connected()) {
    data = client.readStringUntil('\r');
    Serial.print(data);
  }*/
  data = client.readStringUntil('\r');
  Serial.println(data);
  //StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(client);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    return false;
  }

  // Here were copy the strings we're interested in
  strcpy(userData->name1, root["stations"][0]["name"]);
  strcpy(userData->brand1, root["stations"][0]["brand"]);
  strcpy(userData->price1, root["stations"][0]["price"]);
  //strcpy(userData->isOpen1, root["stations"][0]["isOpen"]);
  strcpy(userData->name2, root["stations"][1]["name"]);
  strcpy(userData->brand2, root["stations"][1]["brand"]);
  strcpy(userData->price2, root["stations"][1]["price"]);
  //strcpy(userData->isOpen2, root["stations"][1]["isOpen"]);

  // It's not mandatory to make a copy, you could just use the pointers
  // Since, they are pointing inside the "content" buffer, so you need to make
  // sure it's still in memory when you read the string

  return true;
}

// Print the data extracted from the JSON
void printUserData(const struct UserData* userData) {
  Serial.print("Name1 = ");
  Serial.println(userData->name1);
  Serial.print("Company = ");
  Serial.println(userData->brand1);
  Serial.print("Preis = ");
  Serial.println(userData->price1);
  //Serial.print("Geoeffnet? ");
  //Serial.println(userData->isOpen1);
  Serial.println("JOOOOO");
  Serial.print("Name2 = ");
  Serial.println(userData->name2);
  Serial.print("Company = ");
  Serial.println(userData->brand2);
  Serial.print("Preis = ");
  Serial.println(userData->price2);
  //Serial.print("Geoeffnet? ");
  //Serial.println(userData->isOpen2);
}

void printUserDataOnLCD(const struct UserData* userData) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String(userData->name1).substring(0,10));
  lcd.print(">");
  lcd.print(userData->price1);
  lcd.setCursor(0,1);
  lcd.print(String(userData->name2).substring(0,10));  
  lcd.print(">");
  lcd.print(userData->price2);
}


// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
}

// Pause for a 1 minute
void wait() {
  Serial.println("Wait 1200 seconds aka 20min");
  delay(1200000);
}
