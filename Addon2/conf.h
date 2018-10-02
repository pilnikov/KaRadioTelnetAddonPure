#ifndef _wc_h
#define _wc_h

#define FW_Ver 1.0 //16.05.17 added udp debug console

// ------------------------------------------------------------- Include
#include <WiFiManager.h>

#include <WiFi.h>
#include <WiFiClient.h>

#include <TimeLib.h>
#include <ArduinoOTA.h>

#define DBG_OUT_PORT Serial


// ----------------------------------- Force define func name

void radio_snd (String, bool);
void parse_k(char*);
void removeUtf8(byte*);
void askTime();
void info();
void configModeCallback (WiFiManager*);
void start_wifi();
void OTA_init();

unsigned long   telnet_time  = millis();

//----------------------------------------------------- Karadio specific data


#define BUFLEN  180

char line[BUFLEN]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN]; // received title
char nameset[BUFLEN]; // the local name of the station
char genre[BUFLEN]; // the genre of the station
int16_t volume;
uint8_t _index = 0;
bool askDraw = false, syncTime = false, itAskTime = false;


//------------------------------------------------ Wifi
const char* radio_addr = "192.168.111.178";

#endif /* _wc_h */


