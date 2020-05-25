/* Pour pouvoir flasher le code il faut éditer les droits sur le port USB à chaque redémarrage du PC
 *
 * sudo chmod -R 777 /dev/ttyUSB0 
 * 
*/

/* INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pgmspace.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

<<<<<<< HEAD
//oui

=======
>>>>>>> 68139187f35751bf3ce2de468883fa00ee00ee79
/* CONSTANTES */

const char *ssid = "90Drogou";     //nom du wifi auquel se connecter
const char *password = "Apoxes29"; //son mot de passe

uint16_t Tmin = 15; //valeurs supposées des températures extrèmes
uint16_t Tmax = 40; // ici je mesure la T° de la pièce et de ma main.

// variables pour l'interpolation
int8_t ligne, col, incr;
int8_t av, ap; //pour contenir le pixel suivant et le précedent à chaque calcul
float pixelMilieu, val;

//déclaration d'un tableau qui contiendra les valeurs de températures par pixel
float tableau_pixels[64];

// tableau qui contiendra les valeurs interpolées
float tableau_grandi[80][80];

// create the camara object
Adafruit_AMG88xx amg;

ESP8266WebServer server(80);    //crée un objet "serveur" qui écoute les requêtes HTTP sur le port 80
WebSocketsServer webSocket(81); //

String tab_vide = "[0,0]";

/* Pour Affichage web */
const __FlashStringHelper *ws_html_1() //Page html
{
  return F("<!DOCTYPE html>\n"
           "<html>\n"
           "<head>\n"
           "<meta charset=\"UTF-8\">\n"
           "<title>thermo</title>\n"
           "<style>\n"
           "body {\n"
           "    background-color: #777;\n"
           "}\n"
           "table#tbl td {\n"
           "    width: 25px;\n"
           "    height: 25px;\n"
           "    border: 0px;\n"
           "    cellspadding = 0;\n"
           "    text-align: center;\n"
           "}\n"
          ".deg\n"
          "{\n"
          "  background-image: linear-gradient(to right, blue, green, yellow, red, purple);\n"
          "  width: 80%;\n"
          "  height: 20px;\n"
          " margin: 0 auto;\n"
          "}\n"
          ".gauche\n"
          "{\n"
          "  float: left;\n"
          "  color: blue;\n"
          "}\n"
          ".droite\n"
          "{\n"
          "  float: right;\n"
          "  color: purple;\n"
          "}\n"
           ".tablecenter{\n"
           "  margin-left: auto \n"
           "  margin-right: auto \n"
           "}\n"
           "</style>\n"
           "</head>\n"
           "<body>\n"
           "<table border id=\"tbl\" class=\"tablecenter\"></table>\n"
           "<script>\n"
           "var aff = 0\n"
           "function bgcolor(t) {\n"
           "    if (t < 0) t = 0;\n"
           "    if (t > 30) t = 30;\n"
           "    return \"hsl(\" + (360 - t * 12) + \", 100%, 50%)\";\n"
           "}\n"
           "\n"
           "var t = document.getElementById('tbl');\n"
           "var tds = [];\n"
           "for (var i = 0; i < 15; i++) {\n"
           "    var tr = document.createElement('tr');\n"
           "    for (var j = 0; j < 15; j++) {\n"
           "        var td = tds[i*15 + 14 - j] = document.createElement('td');\n"
           "        tr.appendChild(td);\n"
           "    }\n"
           "    t.appendChild(tr);\n"
           "}\n"
           "var connection = new WebSocket('ws://");
}

/* Pour maj du tableau dans la page web */

const __FlashStringHelper *ws_html_2()
{
  return F(":81/');\n"
           "connection.onmessage = function(e) {\n"
           "    const data = JSON.parse(e.data);\n"
           "    tds[i].style.backgroundColor = bgcolor(data[i]);\n"
           "    for (var i = 0; i < 225; i++) {\n"
           "        tds[i].innerHTML = data[i].toFixed(2);\n"
           "        tds[i].style.backgroundColor = bgcolor(data[i]);\n"
           "    }\n"
           "};\n"
           "function Degres(){\n"
           "    if (aff=0){\n"
           "        aff = 1;\n"
           "    }\n"
           "    else{\n"
           "        aff = 0;\n"
           "        tds[i].innerHTML = data[i].toFixed(2);\n"
           "    }\n"
           "}\n"
           "</script>\n"
           "</br>"
           "<div><p class=\"gauche\">0°C</p><p class=\"droite\">40°C</p></div>\n"
           "<div class=\"deg\"></div>\n"
           "</br>"
           "<button onclick=\"Degres()\">Affichage des degrés</button>\n"
           "</body>\n"
           "</html>\n");
}

/* FONCTIONS */

//  Fonction donnée dans la librairie
//  copiée collée ici et un peu nettoyée de ce qui ne servait pas.
//  ref dans WebSocketServer.ino

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

/*
 * Voir les références pour les fonctions handleRoot & handleNotFound 
 * 
 * Fonction prototypes pour la gestion du http
 */

void handleRoot() // Quand le navgateur fait la requête prévue
{
  auto ip = WiFi.localIP();                                                   // atribution d'une addresse ip
  String addr_IP = String(ip[0]) + "." + ip[1] + "." + ip[2] + "." + ip[3];   // écriture de l'addresse ip de connexion sous un format lisible en html
  server.send(200, "text/html", String(ws_html_1()) + addr_IP + ws_html_2()); //envoie le statut http 200 (ok) et les infos html au navigatuer
}

void handleNotFound()
{ // Quand le navigateur fait une requête imprévue, renvoie d'une erreur vulgarisée
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
  switch (event) //Lors d'un changement ou appel de la fonction
  {

  case WIFI_EVENT_STAMODE_DISCONNECTED:
    Serial.println("WiFi lost connection: reconnecting...");
    WiFi.begin(); // Si perte de connexion, re-init du wifi
    break;

  case WIFI_EVENT_STAMODE_CONNECTED:
    Serial.print("Connected to ");
    Serial.println(ssid); //Quand la connexion est établie, affichage du nom du Wifi
    break;

  case WIFI_EVENT_STAMODE_GOT_IP:
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); //Affiche l'adresse à laquelle se connecter pour voir l'affichage
    if (MDNS.begin("esp8266-amg8833"))
    {
      Serial.println("MDNS responder started");
    }
    // enableOTA(); // si besoin de l'OTA dans le futur
    break;
  }
}

void setup(void) //initialisation
{
  Serial.begin(9600); //baud rate pour amg

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, password);

  // read the camera for initial testing
  amg.readPixels(tableau_pixels);

  server.on("/", handleRoot);
  server.on("/current", []() {
    String str;
    server.send(200, "text/plain", tab_vide); //envoi des premières valeurs
  });

  server.onNotFound(handleNotFound); //

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  amg.begin(0x68); //position dans la pile

  delay(5000); // pendant chauffe de la cam & pour voir l'affichage des init
}

void loop(void) //main
{
  // ArduinoOTA.handle();

  server.handleClient();
  webSocket.loop();

  // Wait for connection
  if (WiFi.status() != WL_CONNECTED) //si perte de la connection wifi
  {
    static unsigned long temps_avant;
    unsigned long t = millis();
    if (t - temps_avant > 500) //après 500ms
    {
      Serial.print("."); // suite de points
      temps_avant = t;   //reset du temps d'attente
    }
  }
  

  // lecture images
  amg.readPixels(tableau_pixels);

      // Pour interpoler 1 valeur entre chaque pixels donnés par la caméra
      // Il faut créer 7 colonnes supplémentaires

      //interpolation lignes
      // interpolation des points entre chaque pixel-image sur les lignes
  for (ligne = 0; ligne < 8; ligne++)
  {
    for (col = 0; col < 15; col++)
    {
      // récupère les pixels-image adjacents
      av = col/2 + (ligne * 8);       // 2 entre chacun des huit
      ap = (col/2) + 1 + (ligne * 8); // point suivant
      pixelMilieu = ((tableau_pixels[ap] - tableau_pixels[av]) / 2.0);
      // incrément (0-1)
      incr = col % 2;
      // Calcul de l'interpolation linéaire
      val = (pixelMilieu  * incr) + tableau_pixels[av];
      // tableau_grandi[ligne][col] = val;
      tableau_grandi[(7 - ligne) * 2][col] = val;
    }
  }

  //interpolation colonnes
  // et maintenant sur les colonnes en utilisant le tableau que l'on vient de remplir
  for (col = 0; col < 15; col++)
  {
    for (ligne = 0; ligne < 15 ; ligne++)
    {
      // récupère la position des pixels-image adjacents
      av = (ligne / 2) * 2;
      ap = av + 2;
      pixelMilieu = ((tableau_grandi[ap][col] - tableau_grandi[av][col]) / 2.0);
      incr = ligne % 2;
      val = (pixelMilieu * incr) + tableau_grandi[av][col];
      tableau_grandi[ligne][col] = val;
    }
  }
  //conversion string
  String interpolation;
  interpolation = "[";
  for (int i = 0; i < 15; i++) //lignes
  {
    if(i!=0 || i!=14)
      interpolation += "\r\n";
    for (int j = 0 ; j<15 ; j++){ //colonnes
        
      interpolation += String(tableau_grandi[i][j]);

      if (i!=14 || j!= 14) //sauf dernière valeur
        interpolation += ", "; //entre chaque pixel
    }
  }
  interpolation += "\r\n]\r\n";
  
  
  static unsigned long temps_lecture_precedente = millis();
  unsigned long now = millis();
  if (now - temps_lecture_precedente > 100) //si pas de maj depuis 100ms
  {
    temps_lecture_precedente += 100;
    // String str;
    // get_current_values_str(str);
    // webSocket.broadcastTXT(str);
    // Serial.println(interpolation);
    webSocket.broadcastTXT(interpolation);
  }
}
