ed th#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include "DebugMacros.h"
int Gas_value_dec, Gas_value_per;
int light_value_dec, light_value_per;
// Multiplexer and NodeMCU Increased Input Analog Values.

#define MUX_A D4
#define MUX_B D3
#define MUX_C D2
#define ANALOG_INPUT A0

int light_value = 0;
int Gas_value = 0;
String sheetlight = "";

String sheetgas = "";

String sheetTemp = "";

String sheetlightalarm = "";

String sheetgasalarm = "";


String sheettempalarm = "";

const char* ssid = "Abhishek_Sebastian";
const char* password = "Abby@1234";


const char* host = "script.google.com";
const char* GScriptId = "AKfycby6vrTEsc7fha0Cyy7SmzlWa3GoXUtjvXBSInZzNOt3ukxRoJ4UiNPRATqk8K5RAx3O";
const int httpsPort = 443;



String url = String("/macros/s/") + GScriptId + "/exec?value=Light";
String url2 = String("/macros/s/") + GScriptId + "/exec?cal";

String payload_base =  "{\"command\": \"appendRow\", \
                    \"sheet_name\": \"Light Sheet\", \
                       \"values\": ";

String payload = "";



HTTPSRedirect* client = nullptr;




void setup()
{
  delay(1000);
  Serial.begin(115200);
  pinMode(ANALOG_INPUT, INPUT);

  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  Serial.print("Connecting to ");
  Serial.println(host);          //try to connect with "script.google.com"

  // Try to connect for a maximum of 5 times then exit
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }




  Serial.println("\nWrite into cell 'A1'");
  Serial.println("------>");
  // fetch spreadsheet data
  client->GET(url, host);


  Serial.println("\nGET: Fetch Google Calendar Data:");
  Serial.println("------>");
  // fetch spreadsheet data
  client->GET(url2, host);



  Serial.println("\nStart Sending Sensor Data to Google Spreadsheet");



  // delete HTTPSRedirect object
  delete client;
  client = nullptr;


}



void loop() {



  float value;
// Light Sensor Value 
  changeMux(LOW, LOW, LOW);
  value = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 0 pin of Mux
  Serial.println("Light value:");
  value = value / 1024; // 0~1024    212/1024=0.207
  value = value * 100;  //20.7
  Serial.println(value);
  sheetlight = String(value) + String("%");
  sheetlightalarm = String("Stable Light!!");
  if (value > 55)
  {
    sheetlightalarm = String("Abnormal light detected!!");
  }
  if (value > 80)
  {
    sheetlightalarm = String("Possible Fire detected!!");
  }

  delay(2000);





// Gas Sensor Value


  changeMux(LOW, LOW, HIGH);
  value = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 1 pin of Mux
  Serial.println("Gas value:");
  value = value / 1024;
  value = value * 100;
  Serial.println(value);
  sheetgasalarm = String("Stable Atmosphere!!");
  sheetgas = String(value) + String("%");
  if (value > 55)
  {
    sheetgasalarm = String("Gas Detected!!");
  }
  if (value > 70)
  {
    sheetgasalarm = String("Very High Conc. Gas Detected!!");
  }

  delay(2000);





// Temperature

  value = 25;
  if (value > 50)
  {
    sheettempalarm = ("Dangerous Temperature Detected");

  }
  if (value > 80)
  {
    sheettempalarm = ("Possible Fire Detected");

  }

  if (value < 0)
  {
    sheettempalarm = ("Sub Zero Temperature Detected");

  }
  sheettempalarm = ("Safe Temp!");
  sheetTemp = String("25");

  delay(2000);


  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  static bool flag = false;

  //payload = payload_base + "\"" + sheetTemp + "," + sheetHumid + "\"}";

  payload = payload_base + "\"" + sheetlight + "," + sheetgas + "," + sheetTemp + "," + sheetlightalarm + "," + sheetgasalarm + "," + sheettempalarm + "\"}";

  if (!flag) {
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }


  if (client != nullptr) {
    if (!client->connected()) {
      client->connect(host, httpsPort);
      client->POST(url2, host, payload, false);
      Serial.print("Sent : ");  Serial.println("Light");
    }



  }
  else {
    DPRINTLN("Error creating client object!");
    error_count = 5;
  }



  if (connect_count > MAX_CONNECT) {
    connect_count = 0;
    flag = false;
    delete client;
    return;
  }

  Serial.println("POST or SEND Sensor data to Google Spreadsheet:");
  if (client->POST(url2, host, payload)) {
    ;
  }
  else {
    ++error_count;
    DPRINT("Error-count while connecting: ");
    DPRINTLN(error_count);
  }
  if (error_count > 3) {
    Serial.println("Halting processor...");
    delete client;
    client = nullptr;
    Serial.printf("Final free heap: %u\n", ESP.getFreeHeap());
    Serial.printf("Final stack: %u\n", ESP.getFreeContStack());
    Serial.flush();
    ESP.deepSleep(0);
  }

  delay(100);
}


void changeMux(int c, int b, int a) {
  digitalWrite(MUX_A, a);
  digitalWrite(MUX_B, b);
  digitalWrite(MUX_C, c);
}
