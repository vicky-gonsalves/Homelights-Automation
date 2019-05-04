#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/*----------------Custom Init----------------------*/
#include <ArduinoJson.h>
#include <SocketIoClient.h>
/*-------------------------------------------------*/

#ifndef STASSID
#define STASSID "XXXX"
#define STAPSK  "XXXX"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;


/*----------------Custom Init----------------------*/
#define USE_SERIAL Serial
SocketIoClient webSocket;
const int light1Pin = 12;
const int light2Pin = 14;
const int light3Pin = 4;
const int light4Pin = 5;
long ms1 = 0;
int cutOff = 30;  //Minutes
/*-------------------------------------------------*/

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  /*----------------Custom Setup----------------------*/
  pinMode(light1Pin, OUTPUT);
  pinMode(light2Pin, OUTPUT);
  pinMode(light3Pin, OUTPUT);
  pinMode(light4Pin, OUTPUT);
  digitalWrite(light1Pin, HIGH);
  digitalWrite(light2Pin, HIGH);
  digitalWrite(light3Pin, HIGH);
  digitalWrite(light4Pin, HIGH);
  webSocket.on("connect", connectedEV);
  webSocket.on("disconnect", disconnected);
  webSocket.on("welcome", event);
  webSocket.on("get-light:init", event);
  webSocket.on("get-light:save", event);
  webSocket.on("get-light:remove", event);
  webSocket.begin("somedomain.com");
  // use HTTP Basic Authorization this is optional remove if not needed
  //    webSocket.setAuthorization("username", "password");
  /*----------------Custom Setup----------------------*/
}

void loop() {
  ArduinoOTA.handle();
  webSocket.loop();
  if (ms1 > 0) {
    if ((millis() - ms1) >= (60000 * cutOff)) {
      ms1 = 0;
      digitalWrite(light1Pin, HIGH);
      digitalWrite(light2Pin, HIGH);
      digitalWrite(light3Pin, HIGH);
      digitalWrite(light4Pin, HIGH);
      webSocket.emit("get-light:put", "{\"light1\":false,\"light2\":false,\"light3\":false,\"light4\":false}");
    }
  }
}

void event(const char * payload, size_t length) {
  USE_SERIAL.printf("got message: %s\n", payload);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    return;
  }
  String id = doc["identifier"];
  if (id && id == "light") {
    bool flag = doc["flag"];
    bool light1 = doc["light1"];
    bool light2 = doc["light2"];
    bool light3 = doc["light3"];
    bool light4 = doc["light4"];

    if (light1 == true) {
      digitalWrite(light1Pin, LOW);
      ms1 = millis();
    } else  if (light1 == false) {
      digitalWrite(light1Pin, HIGH);
    }
    if (light2 == true) {
      digitalWrite(light2Pin, LOW);
      ms1 = millis();
    } if (light2 == false) {
      digitalWrite(light2Pin, HIGH);
    }
    if (light3 == true) {
      digitalWrite(light3Pin, LOW);
      ms1 = millis();
    } if (light3 == false) {
      digitalWrite(light3Pin, HIGH);
    }
    if (light4 == true) {
      digitalWrite(light4Pin, LOW);
      ms1 = millis();
    } if (light4 == false) {
      digitalWrite(light4Pin, HIGH);
    }
  }
}

void disconnected(const char * payload, size_t length) {
  webSocket.emit("get-light:put", "{\"websocket\":\"disconnected\",\"light1\":false,\"light2\":false,\"light3\":false,\"light4\":false}");
}

void connectedEV(const char * payload, size_t length) {
  webSocket.emit("get-light:put", "{\"websocket\":\"connected\",\"light1\":false,\"light2\":false,\"light3\":false,\"light4\":false}");
}
