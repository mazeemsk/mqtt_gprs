#define TINY_GSM_MODEM_SIM800

#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

SoftwareSerial SerialAT(16,17); // RX, TX

#define LOCKPIN 4

//Network details
const char apn[]  = "";
const char user[] = "";
const char password[] = "";


// MQTT details

const char* MQTT_HOST = "MQTT_Broker";
const int   MQTT_PORT = 1883;

const char MQTT_SUB_TOPIC[] = "your_topic_to_subscribe";
const char MQTT_PUB_TOPIC[] = "your_topic_to_publish";


TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);



void setup()
{
  Serial.begin(9600);
  SerialAT.begin(9600);
  pinMode(LOCKPIN, OUTPUT);
  
  Serial.println("System start.");
  modem.restart();
  Serial.println("Modem: " + modem.getModemInfo());
  Serial.println("Searching for telco provider.");
  if(!modem.waitForNetwork())
  {
    Serial.println("fail");
    while(true);
  }
  Serial.println("Connected to telco.");
  Serial.println("Signal Quality: " + String(modem.getSignalQuality()));

  Serial.println("Connecting to GPRS network.");
  if (!modem.gprsConnect(apn, user, password))
  {
    Serial.println("fail");
    while(true);
  }
  Serial.println("Connected to GPRS: " + String(apn));
  
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  Serial.println("Connecting to MQTT Broker: " + String(MQTT_HOST));
  while(mqttConnect()==false) continue;
  Serial.println();
    
}

void loop()
{
  if(Serial.available())
  {
    delay(10);
    String message="";
    while(Serial.available()) message+=(char)Serial.read();
    mqtt.publish(MQTT_PUB_TOPIC, message.c_str());
  }
  
  if(mqtt.connected())
  {
    mqtt.loop();
  }
}

boolean mqttConnect()
{
  if(!mqtt.connect("GsmClientTest"))
  {
    Serial.print(".");
    return false;
  }
  Serial.println("Connected to broker.");
  mqtt.subscribe(MQTT_SUB_TOPIC);
  return mqtt.connected();
}

void mqttCallback(char* topic, byte* payload, unsigned int len)
{
  
  Serial.print("Message receive: ");
  Serial.write(payload, len);
  Serial.println();

      if( !strncmp((char *)payload, "unlocked", len) )
      {
        Serial.print("Unlocked");
        digitalWrite(LOCKPIN, LOW); 
        mqtt.publish(MQTT_PUB_TOPIC, "Unlocked");
       } 
      else if( !strncmp((char *)payload, "locked", len)  )
      {
        Serial.print("Locked");
        digitalWrite(LOCKPIN, HIGH); 
        mqtt.publish(MQTT_PUB_TOPIC, "Locked");

       }                  
}
