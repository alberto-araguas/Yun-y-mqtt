#include <Bridge.h>
#include <YunClient.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h> 


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
String inData; 
long previousMillis = 0;
String Temperatura_A;
String Intensidad;
String Potencia;
String Energia;


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
SoftwareSerial BTserial(7, 6); // RX | TX

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
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
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
   // HC-05 default serial speed for communication mode is 9600
   BTserial.begin(9600);  
   Serial.println("BTserial started at 9600"); 
   digitalWrite(rele1, LOW);
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
  // Keep reading from HC-05 and send to Arduino Serial Monitor
    if (BTserial.available())    {  
      char aChar = BTserial.read();
      inData=inData+aChar;
      if(aChar == '\n') {
       String comp=getValue(inData,' ',0);
       String valor=getValue(inData,' ',1);
       if (comp.equals("Temp:")){
        Temperatura_A=valor;
       } else if (comp.equals("I:")){
        Intensidad=valor;
       } else if (comp.equals("Pot:")){
        Potencia=valor;
       } else if (comp.equals("Energia:")){
        Energia=valor;
       }        
         inData="";
      } 
    }
    if(millis()>previousMillis+30000){ //si ha transcurrido el periodo programado
          EnviarDatosBT();           //cambia el estado del led cada 100ms
          previousMillis = millis();        //guarda el tiempo actual como referencia
          Serial.println("DATOS: T: "+Temperatura_A+" I: "+Intensidad+" P: "+Potencia+" E: "+Energia);
     }     
   
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
  longitud=Temperatura_A.length();
  char msgbufferT[longitud];
  Temperatura_A.toCharArray(msgbufferT,longitud+1);
 client.publish("Temperatura_Analizador",msgbufferT);
  longitud=Potencia.length();
  char msgbufferP[longitud];
  Potencia.toCharArray(msgbufferP,longitud+1);
 client.publish("Potencia",msgbufferP);
  longitud=Intensidad.length();
  char msgbufferI[longitud];
  Intensidad.toCharArray(msgbufferI,longitud+1);
 client.publish("Intensidad",msgbufferI);
  longitud=Energia.length();
  char msgbufferE[longitud];
  Energia.toCharArray(msgbufferE,longitud+1);
 client.publish("Energia",msgbufferE); 
}
void EnviarDatosBT(){
   BTserial.write('H');
  delay(150);
  BTserial.write('T');
  delay(150);
  BTserial.write('I');
  delay(150);
  BTserial.write('P');
  delay(150);
  BTserial.write('E');
}
