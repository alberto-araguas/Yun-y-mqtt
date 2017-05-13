#include <Bridge.h>
#include <YunClient.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 


IPAddress server(192, 168, 1, 55); //Ip de Servidor donde se levanto Mosquitto
/*PINES PLACA RELES*/
int rele1 = 8;            //CALDERA
int rele2 = 9;            //SIRENA
int rele3 = 10; 
int rele4 = 12;
int Builtin_Red_Led = 13;
const int oneWirePin = 11;

// Topic con el que trabajamos
const char* topic_OnOffCaldera = "OnOffCaldera";
const char* topic_EstadoCaldera = "EstadoCaldera";
const char* topic_Temp_Lavadora = "Temperatura_Lavadora";

//variables globales
String Estado_caldera = "Desconocido";
float temperatura=0;
//char message_buff[50];


void byteToChar(byte* bytes, char* chars, unsigned int count){
    for(unsigned int i = 0; i < count; i++)
       chars[i] = (char)bytes[i];
}
char payload_buffer[20];
void callback(char* topic, byte* payload, unsigned int length) {

    if (strcmp (topic,"OnOffCaldera") == 0) {
        byteToChar(payload, payload_buffer, length);
        Serial.println(payload_buffer);
        if (strcmp(payload_buffer, "Encender") == 0) {
           EncenderCaldera();
        } else if (strcmp(payload_buffer, "Apagar") == 0){
           ApagarCaldera();
        }
    }
   for( int i = 0; i < 19;  ++i ){
    payload_buffer[i] = (char)0;
   }
}
/*INSTANCIAS*/
YunClient briClient;
PubSubClient client(server,1883,callback,briClient);
OneWire oneWireBus(oneWirePin);
DallasTemperature sensors(&oneWireBus);

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT conectando...");
    if (client.connect("YUN")) {
      Serial.println("conectado a mqtt");
      client.subscribe("OnOffCaldera");
    } else {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup(){
  Serial.println("Configurando");
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
  pinMode(rele3, OUTPUT);
  pinMode(rele4, OUTPUT);
  pinMode(Builtin_Red_Led, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.begin(115200);
  Bridge.begin();
  sensors.begin(); 
  delay(100);
}

void loop(){
  Serial.println("loop");
  if (!client.connected()) {
    reconnect();
  }
    digitalWrite(Builtin_Red_Led, HIGH);
    client.loop();
    Serial.println("Enviando mqtt");
    publicar();
    sensors.requestTemperatures();
    temperatura =  sensors.getTempCByIndex(0);
    delay(1000);
    Serial.println(temperatura);
    digitalWrite(Builtin_Red_Led, LOW);
    delay(1000);
}

void EncenderCaldera(){
  digitalWrite(rele1,HIGH);
  Estado_caldera = "Encendida";
}
void ApagarCaldera(){
  digitalWrite(rele1,LOW);
  Estado_caldera = "Apagada";
}
void publicar(){
  int longitud=Estado_caldera.length();
  char msgbuffer[longitud];
  Estado_caldera.toCharArray(msgbuffer,longitud+1);
 client.publish("EstadoCaldera",msgbuffer);
  char buff[8];
  dtostrf(temperatura, 4, 2, buff);
 client.publish("Temperatura_Lavadora",buff);
}
