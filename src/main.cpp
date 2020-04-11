/* Pour pouvoir flasher le code il faut éditer les droits sur le port USB à chaque reémarrage du PC
 *
 * sudo chmod -R 777 /dev/ttyUSB0 
 * 
*/


#include <pgmspace.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

Adafruit_AMG88xx amg;

const char *ssid = "90Drogou"; //nom du wifi auquel se connecter
const char *password = "Apoxes29"; //son mot de passe

const int pin_led = D8; //led en D8

ESP8266WebServer server(80); //Port 80
WebSocketsServer webSocket(81); //Por 81

String &valeur_cam(String &ret)
{
  float tableau_pxls[AMG88xx_PIXEL_ARRAY_SIZE];      //déclaration d'un tableau qui contiendra les valeurs de températures par pixel
  amg.readPixels(tableau_pxls);                      //lecture sur la camera
  ret = "[";                                         //pour affichage
  for (int i = 0; i < AMG88xx_PIXEL_ARRAY_SIZE; i++) //pour print le tableau  de manière compréhensible
  {
    if (i % 8 == 0)  //tout les 8 pixels
      ret += "\r\n"; // retour à la ligne
    ret += tableau_pxls[i];
    if (i != AMG88xx_PIXEL_ARRAY_SIZE - 1) // entre chaque valeur
      ret += ", ";
  }
  ret += "\r\n]\r\n"; //en fin de tableau on le ferme et retour à la ligne
  return ret;         //tableau complet et agencé
}



const __FlashStringHelper *ws_html_1() //Page html
{
  return F("<!DOCTYPE html>\n"
           "<html>\n"
           "<head>\n"
           "<title>thermo</title>\n"
           "<style>\n"
           "body {\n"
           "    background-color: #667;\n"
           "}\n"
           "table#tbl td {\n"
           "    width: 64px;\n"
           "    height: 64px;\n"
           "    border: solid 1px grey;\n"
           "    text-align: center;\n"
           "}\n"
           "</style>\n"
           "</head>\n"
           "<body>\n"
           "<table border id=\"tbl\"></table>\n"
           "<script>\n"
           "function bgcolor(t) {\n"
           "    if (t < 0) t = 0;\n"
           "    if (t > 30) t = 30;\n"
           "    return \"hsl(\" + (360 - t * 12) + \", 100%, 80%)\";\n"
           "}\n"
           "\n"
           "var t = document.getElementById('tbl');\n"
           "var tds = [];\n"
           "for (var i = 0; i < 8; i++) {\n"
           "    var tr = document.createElement('tr');\n"
           "    for (var j = 0; j < 8; j++) {\n"
           "        var td = tds[i*8 + 7 - j] = document.createElement('td');\n"
           "        tr.appendChild(td);\n"
           "    }\n"
           "    t.appendChild(tr);\n"
           "}\n"
           "var connection = new WebSocket('ws://");
}

const __FlashStringHelper *ws_html_2()
{
  return F(":81/');\n"
           "connection.onmessage = function(e) {\n"
           "    const data = JSON.parse(e.data);\n"
           "    for (var i = 0; i < 64; i++) {\n"
           "        tds[i].innerHTML = data[i].toFixed(2);\n"
           "        tds[i].style.backgroundColor = bgcolor(data[i]);\n"
           "    }\n"
           "};\n"
           "</script>\n"
           "</body>\n"
           "</html>\n");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED:
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    break;
  }
}

void enableOTA()
{
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void toggle()
{
  static bool last_led = false;
  last_led = !last_led;
  digitalWrite(pin_led, last_led);
}

void handleRoot()
{
  auto ip = WiFi.localIP();
  String ip_str = String(ip[0]) + "." + ip[1] + "." + ip[2] + "." + ip[3];
  server.send(200, "text/html", String(ws_html_1()) + ip_str + ws_html_2());
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {

  case WIFI_EVENT_STAMODE_DISCONNECTED:
    digitalWrite(pin_led, LOW);
    Serial.println("WiFi lost connection: reconnecting...");
    WiFi.begin();
    break;
  case WIFI_EVENT_STAMODE_CONNECTED:
    Serial.print("Connected to ");
    Serial.println(ssid);
    break;
  case WIFI_EVENT_STAMODE_GOT_IP:
    digitalWrite(pin_led, HIGH);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    if (MDNS.begin("esp8266-amg8833"))
    {
      Serial.println("MDNS responder started");
    }
    enableOTA();
    break;
  }
}

void setup(void) //initialisation
{
  pinMode(pin_led, OUTPUT); //écriture
  //Serial.begin(115200); //baud rate pour init du wifi
  Serial.begin(9600);   //baud rate pour amg

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, password); 
  

  server.on("/", handleRoot);
  server.on("/current", []() {
    String str;
    server.send(200, "text/plain", valeur_cam(str)); //envoi des premières valeurs
  });

  server.onNotFound(handleNotFound); //si erreurs wifi

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);


  amg.begin(0x68); //position dans la pile

  delay(5000); // pendant chauffe de la cam & pour voir l'affichage des init
}

void loop(void) //main
{
  ArduinoOTA.handle();
  server.handleClient();
  webSocket.loop();
  //delay(1000); //entre chaque mise à jour -- voir jusqu'ou c'est diminuable

  // Wait for connection
  if (WiFi.status() != WL_CONNECTED) //si perte de la connection wifi
  {
    static unsigned long temps_avant;
    unsigned long t = millis();
    if (t - temps_avant > 500) //après 500ms
    {
      Serial.print("."); // suite de points
      toggle(); //clignotement led
      temps_avant = t; //reset du temps d'attente
    }
  }
  else
  {
    digitalWrite(pin_led, millis() % 3000 < 200); // clignotement led long si tvb
  }

  static unsigned long temps_lecture_precedente = millis();
  unsigned long now = millis();
  if (now - temps_lecture_precedente > 100) //si pas de maj depuis 100ms
  {
    temps_lecture_precedente += 100;
    String str;
    valeur_cam(str); //lecture des valeurs
    Serial.println(str); //afichage des valeurs
    webSocket.broadcastTXT(str); //envoie des valeurs au serveur
  }
}