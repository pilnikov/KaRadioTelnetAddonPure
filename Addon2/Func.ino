
//---------------------------------------Client
void radio_snd (String cmd, bool rcv)
{

  if (millis() - telnet_time > 30000)
  {
    DBG_OUT_PORT.println("\n Start communication over telnet");
    String out = "\n No connect with Radio";
    WiFiClient client;
    const int port = 23;

    if (!client.connect(radio_addr, port))
    {
      client.stop();
      out = "\n connection failed";
    }
    else
    {
      client.print(cmd + "\r\n");
      if (rcv)
      {
        //DBG_OUT_PORT.println("\n Start rcv");
        char tmp = client.read();
        unsigned long st_time = millis();
        while (tmp != '\r' && millis() - st_time < 2000 )
        {
          tmp = client.read();
          switch (tmp)
          {
            case '\n' :
              if (_index == 0) break;

            case '\r' :
              line[_index] = 0; // end of string
              _index = 0;
              parse_k(line);
              break;
            case 0xFF :
              break;

            default : // put the received char in line

              if (tmp < 0xFF ) line[_index++] = tmp;
              if (_index > BUFLEN - 1) // next line
              {
                DBG_OUT_PORT.println(F("overflow"));
                line[_index] = 0;
                parse_k(line);
                _index = 0;
              }
          }
          delay(2);
        }
      }
      //DBG_OUT_PORT.println("\n End rcv");
      client.stop();
      out = "\n Sussecs end communication over telnet";
    }
    DBG_OUT_PORT.println(out);
    telnet_time = millis();
  }
}

////////////////////////////////////////
// parse the karadio received line and do the job
void parse_k(char* line)
{
  //DBG_OUT_PORT.println("\n Start parsing");
  char* ici;
  removeUtf8((byte*)line);

  //DBG_OUT_PORT.printf("\n Inline %s\n", line);

  ////// Meta title
  if ((ici = strstr(line, "META#: ")) != NULL)
  {
    strcpy(title, ici + 7);
    //DBG_OUT_PORT.printf("\n Title ...%s\n", title);
    askDraw = true;
  }
  else
    ////// ICY4 Description
    if ((ici = strstr(line, "ICY4#: ")) != NULL)
    {
      strcpy(genre, ici + 7);
      //DBG_OUT_PORT.printf("\n Genree ...%s\n", genre);
      askDraw = true;
    }
    else
      ////// ICY0 station name
      if ((ici = strstr(line, "ICY0#: ")) != NULL)
      {
        if (strlen(ici + 7) == 0) strcpy (station, nameset);
        else strcpy(station, ici + 7);
        //DBG_OUT_PORT.printf("\n Station name ...%s\n", station);
        askDraw = true;
      }
      else
        ////// STOPPED
        if ((ici = strstr(line, "STOPPED")) != NULL)
        {
          strcpy(title, "STOPPED");
          askDraw = true;
        }
        else
          //////Nameset
          if ((ici = strstr(line, "NAMESET#: ")) != NULL)
          {
            strcpy(nameset, ici + 9);
            //DBG_OUT_PORT.printf("\n Nameset ...%s\n", nameset);
          }
          else
            //////Playing
            if ((ici = strstr(line, "PLAYING#")) != NULL)
            {
              if (strcmp(title, "STOPPED") == 0)
              {
                askDraw = true;
              }
            }
            else
              //////Volume
              if ((ici = strstr(line, "VOL#:")) != NULL)
              {
                volume = atoi(ici+6);
                askDraw = true;;
                //DBG_OUT_PORT.printf("\n Volume ...%03d\n", volume);
              }
              else
                //////Date Time  ##SYS.DATE#: 2017-04-12T21:07:59+01:00
                if ((ici = strstr(line, "SYS.DATE#:")) != NULL)
                {
                  if (*(ici + 11) != '2') //// invalid date. try again later
                  {
                    askDraw = true;
                    return;
                  }
                  char lstr[30];
                  strcpy(lstr, ici + 11);

                  tmElements_t dt;
                  breakTime(now(), dt); //Записываем в структуру dt (содержащую элементы час минута секунда год) текущее время в контроллере (в дурине)
                  int year, month, day, hour, minute, second; //объявляем переменные под год месяц день недели и.т.д
                  sscanf(lstr, "%04d-%02d-%02dT%02d:%02d:%02d", &(year), &(month), &(day), &(hour), &(minute), &(second)); //переносим (разбираем) строчку с датой на отдельные кусочки (день месяц год и.т.д)
                  dt.Year = year - 1970; dt.Month = month; dt.Day = day; //заменяем кусочки структуры dt значениями из нашей принятой и разобранной строки с датой и временем
                  dt.Hour = hour; dt.Minute = minute; dt.Second = second;
                  setTime(makeTime(dt)); //записываем в timestamp(штамп/оттиск времени в формате UNIX time (количество секунд с 1970 года) значение времени сформированное в структуре dt
                  syncTime = true;
                  //DBG_OUT_PORT.printf("\n Current time is %02d:%02d:%02d  %02d-%02d-%04d\n", hour, minute, second, day, month, year);
                }
  //DBG_OUT_PORT.println("\n End parsing");
}

////////////////////////////////////////
void removeUtf8(byte *characters)
{
  int index = 0;
  while (characters[index])
  {
    if ((characters[index] >= 0xc2) && (characters[index] <= 0xc3)) // only 0 to FF ascii char
    {
      //      DBG_OUT_PORT.println((characters[index]));
      characters[index + 1] = ((characters[index] << 6) & 0xFF) | (characters[index + 1] & 0x3F);
      int sind = index + 1;
      while (characters[sind]) {
        characters[sind - 1] = characters[sind];
        sind++;
      }
      characters[sind - 1] = 0;

    }
    index++;
  }
}

////////////////////////////////////////
void askTime()
{
  if (itAskTime) // time to ntp. Don't do that in interrupt.
  {
    radio_snd("sys.date", true);
    itAskTime = false;
  }
}

void info()
{
  DBG_OUT_PORT.printf("\nNameset ...%s\n", nameset);
  DBG_OUT_PORT.printf("Station name ...%s\n", station);
  DBG_OUT_PORT.printf("Genree ...%s\n", genre);
  DBG_OUT_PORT.printf("Title ...%s\n", title);
  DBG_OUT_PORT.printf("Volume ...%03d\n", volume);

  tmElements_t dt;
  breakTime(now(), dt); //Current time (in MC) -> dt
  DBG_OUT_PORT.printf("Current time is %02d:%02d:%02d  %02d-%02d-%04d\n", dt.Hour, dt.Minute, dt.Second, dt.Day, dt.Month, dt.Year + 1970);
}

//////////////////////////////////////////// WiFi
//-------------------------------------------------------------- Callback
void configModeCallback (WiFiManager *myWiFiManager)
{
  DBG_OUT_PORT.print("\n Entered config mode, IP is...");
  DBG_OUT_PORT.println(WiFi.softAPIP());

  DBG_OUT_PORT.print("\n Connected to...");
  DBG_OUT_PORT.println(myWiFiManager->getConfigPortalSSID());
}

//-------------------------------------------------------------- Start_wifi
void start_wifi()
{
  WiFiManager wm;
  //wm.resetSettings();
  //wm.setConfigPortalBlocking(false);
  wm.setAPCallback(configModeCallback);
  wm.setConfigPortalTimeout(60);
  wm.autoConnect("Addon", "12345678");
}

//------------------------------------------------------------- OTA
void OTA_init()
{
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    DBG_OUT_PORT.println("Start updating " + type);
  })
  .onEnd([]() {
    DBG_OUT_PORT.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    DBG_OUT_PORT.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    DBG_OUT_PORT.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DBG_OUT_PORT.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DBG_OUT_PORT.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DBG_OUT_PORT.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DBG_OUT_PORT.println("Receive Failed");
    else if (error == OTA_END_ERROR) DBG_OUT_PORT.println("End Failed");
  });

  ArduinoOTA.begin();

  DBG_OUT_PORT.print("OTA ready with IP address: ");
  DBG_OUT_PORT.println(WiFi.localIP());
}

