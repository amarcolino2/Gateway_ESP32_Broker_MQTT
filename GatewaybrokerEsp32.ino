#include<sMQTTBroker.h>
#include <ArduinoJson.h>
#include <ThingsBoard.h>

// to understand how to obtain an access token
#define TOKEN              "your_access_token"
#define clientId      	   "your_clientId"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define thingsboardServer   "demo.thingsboard.io"

const char* ssid =         "your_SSID";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password =     "your_PASSWORD_WIFI"; // The password of the Wi-Fi network
bool  subscribed =         false;

WiFiClient wifiClient;

ThingsBoard client(wifiClient);
//PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

class MyBroker:public sMQTTBroker
{
public:
    bool onConnect(sMQTTClient *client, const std::string &username, const std::string &password)
    {
        // check username and password, if ok return true
        return true;
    };
	  void onRemove(sMQTTClient*)
    {
    };
    void onPublish(sMQTTClient *cliId,const std::string &topic, const std::string &payload)
    {
        // client publish a message to the topic
        // you can parse the message, save it to a database, or whatever, based on your goals
        char settings_data[100];
        Serial.println(cliId->getClientId().c_str());
        Serial.println(topic.c_str());
        Serial.println(payload.c_str());    
        strcpy(settings_data, "v1/devices/me/");
        strcat(settings_data, topic.c_str());

        long rssi = WiFi.RSSI();
        char buffer[3];
        char* strptr = ltoa(rssi,buffer,10);        
        
        String pay = "{";
        pay += "\"rssi\":"; pay += strptr;
        pay += "}";
            
        client.sendTelemetryString(topic.c_str(), payload.c_str());    
        client.loop();        
    }
    bool onEvent(sMQTTEvent *event)
    {
        switch(event->Type())
        {
        case NewClient_sMQTTEventType:
            {
                sMQTTNewClientEvent *e=(sMQTTNewClientEvent*)event;
                e->Login();
                e->Password();
            }
            break;
        case LostConnect_sMQTTEventType:
            WiFi.reconnect();
            break;
        }
        return true;
    }
};

MyBroker broker;


// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response processGet(const RPC_Data &data)
{
  boolean getvalue = data;  
  Serial.println("Received the get value method");
  Serial.println(getvalue);
  
  return RPC_Response(NULL, getvalue);
};

// RPC handlers
RPC_Callback callbacks[] = {
  { "getValue",         processGet },
};

void setup()
{
    Serial.begin(115200);
      
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(1000);
    }
    Serial.println("Connection established!");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
          
    const unsigned short mqttPort=1883;
    broker.init(mqttPort);
    
    tr// all done
    // your magic code

};

void loop()
{
    if ( !client.connected() ) {
      reconnect();
      subscribed = false;
    }
    
    broker.update();

    if (!subscribed) {  
      Serial.println("Subscribing for RPC...");     
      if (!client.RPC_Subscribe(callbacks, COUNT_OF(callbacks))) {
        Serial.println("Failed to subscribe for RPC");
        return;
      }   
      Serial.println("Subscribe done");
      subscribed = true;
    } 
    
    // your magic code
    
};

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
     
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
};
