
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// conectado al puerto D4 de Wemos Lite
//#define ONE_WIRE_BUS 2
//OneWire oneWire(ONE_WIRE_BUS); // inicia una instancia de OneWire
#define DHTPIN 2    // Digital pin connected to the DHT sensor  D4
#define DHTTYPE    DHT22     // DHT 22 (AM2302)


// Pass our oneWire reference to Dallas Temperature. 
//DallasTemperature sensors(&oneWire); 


DHT_Unified dht(DHTPIN, DHTTYPE);
// Update these with values suitable for your network.


// Defenir Sensor de CO2
#define MQ135 0  //A0  HW080_VCC

const char* ssid = "WiFi 2.4Ghz";       //Credenciales de Red
const char* password = "williams145";
//const char* mqtt_server = "201.253.135.158"; 
const char* mqtt_server = "192.168.0.67"; 

String horas = "";      // Variable donde se guardara la hora recibida por MQTT
String minutos = "";    // Variable donde se guardara los minutos recibida por MQTT
String co2 = "";    // Variable donde se guardara los minutos recibida por MQTT
bool flag = 0;


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)  // tamaño del array de caractares que sera enviado como payload del mensaje MQTT
char msg[MSG_BUFFER_SIZE];  // array de caractares que sera enviado como payload del mensaje MQTT
int value = 0;

// Varialbe para guardar temporalmente las lecturas de los sensores 
// Convertir int --> String
String mensaje = "";        

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode (MQ135, OUTPUT);//HW080_VCC


}
void callback(char* topic, byte* payload, unsigned int length) {
// Variable en las que ser guarda Tema MQTT recibido 
  String tema = ""; 
  horas = "";
  minutos = "";
  co2 = "";
  tema = String(topic);
//  Serial.println("minutos recibido");
  Serial.print ("Tema: ");
  Serial.println(tema);
// Si el tema es CASA/HORA/HORA; guarda el payload en la variable horas
  if (tema == "CASA/HORA/HORA")
  {
    for (int i = 0; i < length; i++) {
      horas = horas + (char)payload[i];
      }
    }
// Si el tema es CASA/HORA/MINUTOS; guarda el payload en la variable minutos

  if (tema == "CASA/HORA/MINUTOS")
  {
    for (int i = 0; i < length; i++) {
      minutos = minutos + (char)payload[i];
      }
    }
// Si el tema es CASA/JARDIN/CO2; guarda el payload en la variable co2
    if (tema == "CASA/JARDIN/CO2")
  {
    for (int i = 0; i < length; i++) {
      co2 = co2 + (char)payload[i];
      }
    }

    // Setea la bandera que marca que recibimos un mensaje de tema CASA/HORA
    flag = 1;   
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");

      // call sensors.requestTemperatures() to issue a global temperature 
      // request to all devices on the bus
      Serial.print("Requesting temperatures...");

//      sensors.requestTemperatures(); // Send the command to get temperatures
      Serial.println("DONE");
      // After we got the temperatures, we can print them here.
      // We use the function ByIndex, and as an example get the temperature from the first sensor only.
    //  float tempC = sensors.getTempCByIndex(0);
      //int TEMP = analogRead(A0);
      //int tempC = ((TEMP * 5000.0) / 1023) / 10;
       
      int tempC = random(0, 100);
      // Check if reading was successful
      if(tempC != DEVICE_DISCONNECTED_C) 
        {
        Serial.print("Temperature for the device 1 (index 0) is: ");
        Serial.println(tempC);
        //snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        //snprintf (msg, MSG_BUFFER_SIZE, "%ld", (char)tempC);
        mensaje =  String(tempC);
        mensaje.toCharArray(msg, MSG_BUFFER_SIZE);
        //msg = (char)tempC;
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("CASA/JARDIN/TEMPERATURA", msg);
        //    client.publish("CASA/JARDIN/TEMPERATURA", msg);

      } 
      else
      {
        Serial.println("Error: Could not read temperature data");
      }

      digitalWrite (MQ135, LOW);
      delay (500);
      
//      int humedad = (1024-lectura)*100/1024;
        int humedad = random(0, 128);

      Serial.print ("El porcentaje de humedad es: ");
      Serial.println (humedad);
      mensaje =  String(humedad);
      mensaje.toCharArray(msg, MSG_BUFFER_SIZE);      
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("CASA/JARDIN/HUMEDAD", msg);
      digitalWrite (MQ135, HIGH);
//      client.subscribe("CASA/HORA/MINUTOS");
      client.subscribe("CASA/HORA/#");
      //--------------------------------------------
      
      int sensorValue = analogRead(0);       // read analog input pin 0
      Serial.print("AirQua CO2= ");
      Serial.print(sensorValue, DEC);  // prints the value read
      Serial.println(" PPM");
      delay(100);                        // wait 100ms for next reading
      mensaje =  String(sensorValue);
      mensaje.toCharArray(msg, MSG_BUFFER_SIZE);      
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("CASA/JARDIN/CO2", msg);
      digitalWrite (MQ135, HIGH);

      //-----------------------------------------

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
   
  // Start up DS18b20 the library
  //sensors.begin();
}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (flag == 1){
    Serial.print ("HORA: ");
    Serial.println( horas);
    Serial.print ("MINUTOS: ");
    Serial.println( minutos);
    flag = 0;
   // if (minutos == "0" || minutos == "10" || minutos =="20" || minutos == "30" || minutos == "40" || minutos == "50"){
   if (1){
      // call sensors.requestTemperatures() to issue a global temperature 
      // request to all devices on the bus
      Serial.print("Requesting temperatures...");
      //sensors.requestTemperatures(); // Send the command to get temperatures
      Serial.println("DONE");
      // After we got the temperatures, we can print them here.
      // We use the function ByIndex, and as an example get the temperature from the first sensor only.
      //float tempC = sensors.getTempCByIndex(0);
      //-----------------------

  
 
//----------------------------------------------
     //----11-08-24-------------------------------------
      int tempC = random(0, 100);
      
     //      int TEMP = analogRead(A0);
      //int tempC = ((TEMP * 5000.0) / 1023) / 10;
      Serial.println(tempC);
      // Check if reading was successful
      if(tempC != DEVICE_DISCONNECTED_C) 
        {
        Serial.print("Temperature for the device 1 (index 0) is: ");
        Serial.println(tempC);
        //snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        //snprintf (msg, MSG_BUFFER_SIZE, "%ld", (char)tempC);
        mensaje =  String(tempC);
        mensaje.toCharArray(msg, MSG_BUFFER_SIZE);
        //msg = (char)tempC;
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("CASA/JARDIN/TEMPERATURA", msg);
        //    client.publish("CASA/JARDIN/TEMPERATURA", msg);
        
      } 
      else
      {
        Serial.println("Error: Could not read temperature data");
      }
      
      digitalWrite (MQ135, LOW);
      delay (500);
      //int lectura = analogRead(A0);
       
      int humedad = random(0, 128);
      humedad = (1024-lectura)*100/1024;
//      int humedad =event.relative_humidity;
      Serial.print ("El porcentaje de humedad es: ");
      Serial.println (humedad);
      mensaje =  String(humedad);
      mensaje.toCharArray(msg, MSG_BUFFER_SIZE);      
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("CASA/JARDIN/HUMEDAD", msg);
        //    client.publish("CASA/JARDIN/TEMPERATURA", msg);
      digitalWrite (MQ135, HIGH);

       //--------------------------------------------
       
      
      int sensorValue = analogRead(0);       // read analog input pin 0
      Serial.print("AirQua CO2= ");
      Serial.print(sensorValue, DEC);  // prints the value read
      Serial.println(" PPM");
      delay(100);                        // wait 100ms for next reading
      mensaje =  String(sensorValue);
      mensaje.toCharArray(msg, MSG_BUFFER_SIZE);      
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("CASA/JARDIN/CO2", msg);
      digitalWrite (MQ135, HIGH);


      //-----------------------------------------



    }  
  }
}


/*
  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("CASA/JARDIN/TEMPERATURA", msg);
  }
*/