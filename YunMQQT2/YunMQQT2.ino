#include <Bridge.h>
#include <YunClient.h>
#include <PubSubClient.h>

IPAddress server(192, 168, 1, 137); //Ip de Servidor donde se levanto Mosquitto
/*PINES PLACA RELES*/
int rele1 = 8;            //CALDERA
int rele2 = 9;            //SIRENA
int rele3 = 10; 
int rele4 = 11;
int Builtin_Red_Led = 13;

// Topic con el que trabajamos
const char* topic_OnOffCaldera = "OnOffCaldera";
const char* topic_EstadoCaldera = "EstadoCaldera";

//variables globales
String Estado_caldera = "Desconocido";
char message_buff[50];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


YunClient briClient;
PubSubClient client(server,1883,callback,briClient);

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
  delay(1000);
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
    digitalWrite(Builtin_Red_Led, LOW);
    delay(10000);
  
}
void EncenderCaldera(){
  digitalWrite(rele1,HIGH);
  Estado_caldera = "Encendida";
  publicar();
}
void ApagarCaldera(){
  digitalWrite(rele1,LOW);
  Estado_caldera = "Apagada";
  publicar();
}
void publicar(){
 int longitud=Estado_caldera.length();
 char msgbuffer[longitud];
 Estado_caldera.toCharArray(msgbuffer,longitud+1);
 client.publish("EstadoCaldera",msgbuffer);
}
