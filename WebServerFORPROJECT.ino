#include <SPI.h>
#include <Ethernet2.h>
#include <SD.h>

#define led1 = 8
#define led2 = 9

File myFile;
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress ip(192, 168, 1, 49);

String HTTP_req; // stores the HTTP request
int HTTP_reqLenght = 0;

int Step = 1;
String code = "";

String filereadstring = "";  //FILE READ STRING POUR DES DONNÉES NON DÉFINIT
String filereadstring1 = ""; //FILE READ STRING POUR LES DONNÉES DU SWITCH CONNECTÉE
String filereadstring2 = ""; //FILE READ STRING POUR LES DONNÉES DU CAPTEUR DE TEMPÉRATURE
String filereadstring3 = ""; //FILE READ STRING POUR LES DONNÉES DU CAPTEUR DE GAZ (AIRQUALITY)
String filereadstring4 = ""; //FILE READ STRING POUR LES DONNÉES DE LA SERRURE CONNECTÉE

char filereadchar = "";  //FILE READ CHAR POUR DES DONNÉES NON DÉFINIT
char filereadchar1 = ""; //FILE READ CHAR POUR LES DONNÉES DU SWITCH CONNECTÉE
char filereadchar2 = ""; //FILE READ CHAR POUR LES DONNÉES DU CAPTEUR DE TEMPÉRATURE
char filereadchar3 = ""; //FILE READ CHAR POUR LES DONNÉES DU CAPTEUR DE GAZ (AIRQUALITY)
char filereadchar4 = ""; //FILE READ CHAR POUR LES DONNÉES DE LA SERRURE CONNECTÉE

uint16_t A1state, A2state, A3state, A4state; //ETAT DES BROCHES ANALOGIQUES

String day = "04";         //VARIABLE LIÉ AU JOURS ACTUEL
String mois = "03";        //VARIABLE LIÉ AU MOIS ACTUEL
String annee = "2019";     //VARIABLE LIÉ A L'ANNE ACTUEL
String datatime = "12:12"; //VARIABLE LIÉ A L'HEURE ACTUEL

String sharedata; //VARIABLE ENVOYER VIA LA PAGE HTML A L'APPLICATION ET A L'IHM

int sensortype;    //TYPE DE CAPTEUR QUI ENVOIE DES DONNÉES
int recupdatatype; //TYPE DE DONNÉE A ENVOYER A L'APPLICATION OU A L'IHM

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Adresse du serveur: ");
  Serial.println(Ethernet.localIP());

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  Serial.print("Initialisation de la carte SD...");

  if (!SD.begin(4))
  {
    Serial.println("Echec de l'initialisation!");
    while (1)
      ;
  }
  Serial.println("Initialisation terminée.");
  /*
    if (!SD.exists("index.htm")) {
      Serial.println("ERROR - Can't find index.htm file!");
      return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.htm file.");
  */
  a();
  //testfile();
}

void a()
{
  if (SD.exists("F5.txt"))
  {
    Serial.println("F5.txt existe.");
    myFile = SD.open("F5.txt", FILE_READ);
    while (myFile.available())
    {
      filereadchar = myFile.read();
      filereadstring = filereadstring + filereadchar;
      //Serial.write(myFile.read());
    }
    Serial.println(filereadstring);
    myFile.close();
  }
  else
  {
    Serial.println("F5.txt est inexistant.");
    myFile = SD.open("F5.txt", FILE_WRITE);
    myFile.close();
    myFile = SD.open("F5.txt", FILE_WRITE);
    myFile.println("fistuse :" + day + "/" + mois + "/" + annee + " à " + datatime);
    myFile.close();
  }
}

void loop()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client)
  {
    Serial.println("Nouveau client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        String pass = "" + c;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        HTTP_req += c; // save the HTTP request 1 char at a time
        HTTP_reqLenght = HTTP_req.length();
        delay(1000);
        Serial.print("\nHTTP REQUEST STRING\n");
        Serial.println(HTTP_req);
        // send a standard http response header
        if (c == '\n' && currentLineIsBlank)
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close"); // the connection will be closed after completion of the response
          client.println("Refresh: 5");        // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<style> p.sharedata {color: rgb(255,0,0);} </style> ");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++)
          {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
            client.println(code);
          }

          client.print("<p class=");
          client.print("datashare");
          client.println(">" + sharedata + "</p>");

          client.println("</html>");
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }

        // Serial.println(HTTP_req.indexOf("elias") > -1);

        // traitementgetTEST();        //A SUPRIMER SI NE FONCTIONNE PAS

        traitementget(); //TRAITEMENT DES REQUETES GET RECUS

        if ((analogRead(A1)) || (analogRead(A2)) || (analogRead(A4)) || (analogRead(A4)))
        {
          if (analogRead(A1))
          {
            sensortype = 1;
          }
          else if (analogRead(A2))
          {
            sensortype = 2;
          }
          else if (analogRead(A3))
          {
            sensortype = 3;
          }
          else if (analogRead(A4))
          {
            sensortype = 4;
          }
          savesensor();
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    HTTP_req = "";
  }

  A1state = analogRead(A1);
  A2state = analogRead(A2);
  A3state = analogRead(A3);
  A4state = analogRead(A4);
}

/*
  void traitementgetTEST () {

  if ( HTTP_req.indexOf("GET /home.html") > -1) {

    webFile = SD.open("home.html");        // open web page file
    if (webFile) {
      while (webFile.available()) {
        client.write(webFile.read()); // send web page to client
      }
      webFile.close();
    }
  }
  }
*/

void traitementget()
{

  if (HTTP_req.indexOf("1234") > -1)
  {

    Serial.println("CODE OK");
    code = "OK";
  }

  if (HTTP_req.indexOf("elias") > -1)
  {

    Serial.println("ELIAS OK");
  }
  if (HTTP_req.indexOf("led1ON") > -1)
  {

    Serial.println("LED1 ON");
    digitalWrite(led1, HIGH);
  }
  else if (HTTP_req.indexOf("led1OFF") > -1)
  {
    Serial.println("LED1 OFF");
    digitalWrite(led1, LOW);
  }

  if (HTTP_req.indexOf("led2ON") > -1)
  {

    Serial.println("LED2 ON");
    digitalWrite(led2, HIGH);
  }
  else if (HTTP_req.indexOf("led2OFF") > -1)
  {
    Serial.println("LED2 OFF");
    digitalWrite(led2, LOW);
  }

  if (HTTP_req.indexOf("recupdata1") > -1)
  {
    recupdatatype = 1;

    myFile = SD.open("F6.txt", FILE_READ);
    while (myFile.available())
    {
      filereadchar1 = myFile.read();
      filereadstring1 = filereadstring1 + filereadchar1;
      //Serial.write(myFile.read());
    }
    Serial.println(filereadstring1);
    myFile.close();

    recupdata();
  }
  else if (HTTP_req.indexOf("recupdata2") > -1)
  {
    recupdatatype = 2;

    myFile = SD.open("F7.txt", FILE_READ);
    while (myFile.available())
    {
      filereadchar2 = myFile.read();
      filereadstring2 = filereadstring2 + filereadchar2;
      //Serial.write(myFile.read());
    }
    Serial.println(filereadstring1);
    myFile.close();

    recupdata();
  }
  else if (HTTP_req.indexOf("recupdata3") > -1)
  {
    recupdatatype = 3;

    myFile = SD.open("F8.txt", FILE_READ);
    while (myFile.available())
    {
      filereadchar3 = myFile.read();
      filereadstring3 = filereadstring3 + filereadchar3;
      //Serial.write(myFile.read());
    }
    Serial.println(filereadstring3);
    myFile.close();

    recupdata();
  }
  else if (HTTP_req.indexOf("recupdata4") > -1)
  {
    recupdatatype = 4;

    myFile = SD.open("F9.txt", FILE_READ);
    while (myFile.available())
    {
      filereadchar4 = myFile.read();
      filereadstring4 = filereadstring4 + filereadchar4;
      //Serial.write(myFile.read());
    }
    Serial.println(filereadstring4);
    myFile.close();

    recupdata();
  }
}

void savesensor()
{
  /*

    F1 : FIRST LOG OF USER

    F2 : DATA

    F3 : DATA

    F4 : DATA

    F5 : DATA

    F6 : SWITCH CONNECTÉ    #ELIAS

    F7 : CAPTEUR DE TEMPÉRATURE     #ALEXIS

    F8 : CAPTEUR DE GAZ (AIRQUALITY)    #MATHIEU

    F9 :SERURE CONNECTÉ                         #JORGE

  */
  switch (sensortype)
  {
  case 1:
    if (SD.exists("F6.txt"))
    {
      Serial.println("F6.txt exists.");

      myFile = SD.open("F6.txt", FILE_WRITE);
      myFile.println("ANALOG 1 :" + A1state);
      myFile.close();
    }
    else
    {
      Serial.println("F6.txt doesn't exist.");
      myFile = SD.open("F6.txt", FILE_WRITE);
      myFile.close();
    }
    break;

  case 2:
    if (SD.exists("F7.txt"))
    {
      Serial.println("F7.txt exists.");

      myFile = SD.open("F7.txt", FILE_WRITE);
      myFile.println("ANALOG 2 :" + A2state);
      myFile.close();
    }
    else
    {
      Serial.println("F7.txt doesn't exist.");
      myFile = SD.open("F7.txt", FILE_WRITE);
      myFile.close();
    }
    break;

  case 3:
    if (SD.exists("F8.txt"))
    {
      Serial.println("F8.txt exists.");

      myFile = SD.open("F8.txt", FILE_WRITE);
      myFile.println("ANALOG 3 :" + A3state);
      myFile.close();
    }
    else
    {
      Serial.println("F8.txt doesn't exist.");
      myFile = SD.open("F8.txt", FILE_WRITE);
      myFile.close();
    }
    break;

  case 4:
    if (SD.exists("F9.txt"))
    {
      Serial.println("F9.txt exists.");

      myFile = SD.open("F9.txt", FILE_WRITE);
      myFile.println("ANALOG 4 :" + A4state);
      myFile.close();
    }
    else
    {
      Serial.println("F9.txt doesn't exist.");
      myFile = SD.open("F9.txt", FILE_WRITE);
      myFile.close();
    }
    break;
  default:
    break;
  }
  sensortype = 0;
}

void recupdata()
{
  // ON ENVOI LES DONNÉES DEMANDER PAR L'APPLICATION OU L'IHM VIA LA PAGE HTML AVEC LA VARIABLE DE TYPE "STRING" sharedata
  switch (recupdatatype)
  {
  case 1:
    sharedata = "DATA1 :" + filereadstring1;
    break;
  case 2:
    sharedata = "DATA2 :" + filereadstring2;
    break;
  case 3:
    sharedata = "DATA3 :" + filereadstring3;
    break;
  case 4:
    sharedata = "DATA4 :" + filereadstring4;
    break;
  default:
    break;
  }
}
