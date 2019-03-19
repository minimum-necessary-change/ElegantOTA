#ifndef ElegantOTA_h
#define ElegantOTA_h

#include <functional>
using namespace std;
using namespace std::placeholders;

#if defined(ESP8266) || defined(ESP32)
    #include "Arduino.h"
    #include "stdlib_noniso.h"
    #include "elegantWebpage.h"
#endif

#if defined(ESP8266)
    #define HARDWARE "ESP8266"
    #include "ESP8266WiFi.h"
    #include "WiFiClient.h"
    #include "ESP8266WebServer.h"
    #include "ESP8266HTTPUpdateServer.h"
#elif defined(ESP32)
    #define HARDWARE "ESP32"
    #include "WiFi.h"
    #include "WiFiClient.h"
    #include "WebServer.h"
    #include "Update.h"
#endif

class ElegantOtaClass{
    public:
        #if defined(ESP8266)
            void begin(ESP8266WebServer *server){
                _server = server;

                _server->on("/update", HTTP_GET, [&](){
                    _server->sendHeader("Content-Encoding", "gzip");
                    _server->send_P(200, "text/html", (const char*)ELEGANT_HTML, ELEGANT_HTML_SIZE);
                });

                _httpUpdater.setup(server);
            }
        #elif defined(ESP32)
            void begin(WebServer *server){
                _server = server; 

                _server->on("/update", HTTP_GET, [&](){
                    _server->sendHeader("Content-Encoding", "gzip");
                    _server->send_P(200, "text/html", (const char*)ELEGANT_HTML, ELEGANT_HTML_SIZE);
                });

                _server->on("/update", HTTP_POST, [&]() {
                    _server->sendHeader("Connection", "close");
                    _server->send((Update.hasError()) ? 500 : 200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
                    ESP.restart();
                }, [&]() {
                    HTTPUpload& upload = _server->upload();
                    if (upload.status == UPLOAD_FILE_START) {
                    Serial.printf("Update: %s\n", upload.filename.c_str());
                    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                        Update.printError(Serial);
                    }
                    } else if (upload.status == UPLOAD_FILE_WRITE) {
                    /* flashing firmware to ESP*/
                    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                        Update.printError(Serial);
                    }
                    } else if (upload.status == UPLOAD_FILE_END) {
                    if (Update.end(true)) { //true to set the size to the current progress
                        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                    } else {
                        Update.printError(Serial);
                    }
                    }
                });
            }
        #endif

    private:
        #if defined(ESP8266)
            ESP8266WebServer *_server;
            ESP8266HTTPUpdateServer _httpUpdater;
        #elif defined(ESP32)
            WebServer *_server;
        #endif
};

ElegantOtaClass ElegantOTA;
#endif