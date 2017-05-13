#include <Bridge.h>
#include <BridgeClient.h>
#include <PubSubClient.h>

IPAddress server(192, 168, 1, 55); //Ip de Servidor donde se levanto Mosquitto

BridgeClient briClient;
PubSubClient client(briClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recivido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  client.publish("YunTopic","Hello Node.js");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT conectando...");
    if (client.connect("")) {
      Serial.println("conectado !");
      client.publish("YunTopic","Hello World from YUN");
      client.subscribe("inTopic");
    } else {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup(){
    pinMode(13, OUTPUT);
     digitalWrite(13, HIGH);
  Serial.begin(115200);
  client.setServer(server, 1883);
  client.setCallback(callback);
  Bridge.begin();
  delay(1500);
    digitalWrite(13, LOW);
}

void loop(){
  if (!client.connected()) {
    reconnect();
  }
    digitalWrite(13, HIGH);
    client.loop();
    Serial.println("Enviando !");
    client.publish("YunTopic","Hello World from YUN");
    delay(1000);
    digitalWrite(13, LOW);
}
