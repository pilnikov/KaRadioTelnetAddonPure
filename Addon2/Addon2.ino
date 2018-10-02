#include "conf.h"

void setup()
{
  //------------------------------------------------------  Определяем консоль
  DBG_OUT_PORT.begin(115200);

  Serial.setDebugOutput(true);

  //-------------------------------------------------------- Запускаем WiFi

  start_wifi();

  DBG_OUT_PORT.println("WiFi started");

  //------------------------------------------------------ Подключаем OTA
  OTA_init();

  //------------------------------------------------------ Запускаем парсер
  telnet_time = millis() + 31000;
  radio_snd("cli.info", true);
  info();
}


void loop()
{
 ArduinoOTA.handle();
}


