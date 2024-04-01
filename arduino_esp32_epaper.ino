/***
   MQTT connected e-paper

   Board : Lilygo T5_V2.3-2.13
   source : https://github.com/Xinyuan-LilyGO/LilyGo-T5-Epaper-Series
   exemple : https://github.com/Xinyuan-LilyGO/LilyGo-T5-Epaper-Series/blob/master
   product : https://www.aliexpress.com/item/32869729970.html
   schematic : https://github.com/Xinyuan-LilyGO/LilyGo-T5-Epaper-Series/blob/master/schematic/T5V2.3.pdf
   
   Source :
    - 2021 by martin schlatter, schwetzingen, germany
    - barcode : https://github.com/victorheid/arduino_barcode128.git

*/

#include <Arduino_JSON.h>
#include "Barcode.h"
#include <GxEPD.h>
#include <SD.h>
#include <FS.h>

#include <GxDEPG0213BN/GxDEPG0213BN.h>  // 2.13" b/w  form DKE GROUP

#include GxEPD_BitmapExamples

#include "FreeMonoBold9pt7b.h"
#include "Fonts/FreeMono12pt7b.h"

#include "Adafruit_GFX.h"
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <stdio.h>
//#include "M5_ENV.h"

#include <Wire.h>
//include <esp_deep_sleep.h>

#include <WiFi.h>
#include <WiFiClient.h>
//#include <WebServer.h>
//#include <ESPmDNS.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <PubSubClient.h>



/*
  ________          _____.__
  \______ \   _____/ ____\__| ____   ____
   |    |  \_/ __ \   __\|  |/    \_/ __ \
   |    `   \  ___/|  |  |  |   |  \  ___/
  /_______  /\___  >__|  |__|___|  /\___  >
          \/     \/              \/     \/
*/
// According to the board, cancel the corresponding macro definition
#define LILYGO_T5_V213
#define ARDUINOJSON_DECODE_UNICODE 1

//pinout baord for screen
#define EPD_CS 5
#define EPD_DC 17
#define EPD_RSET 16
#define EPD_BUSY 4
#define EPD_SCLK 18
#define EPD_MISO 2l
#define EPD_MOSI 23

// Number de test max
#define TEST_WIFI 1  //20
#define TEST_MQTT 1  //5

//timing
#define PERIOD_SEND_DATA 10000   //10 secondes
#define PERIOD_SEND_STATS 60000  //60 secondes

//debug
#define DEBUG_UART 0

/*
    ________.__        ___.          .__                                              __
   /  _____/|  |   ____\_ |__ _____  |  |   ___________ ____________    _____   _____/  |_  ___________  ______
  /   \  ___|  |  /  _ \| __ \\__  \ |  |   \____ \__  \\_  __ \__  \  /     \_/ __ \   __\/ __ \_  __ \/  ___/
  \    \_\  \  |_(  <_> ) \_\ \/ __ \|  |__ |  |_> > __ \|  | \// __ \|  Y Y  \  ___/|  | \  ___/|  | \/\___ \
   \______  /____/\____/|___  (____  /____/ |   __(____  /__|  (____  /__|_|  /\___  >__|  \___  >__|  /____  >
          \/                \/     \/       |__|       \/           \/      \/     \/          \/           \/
*********************************************************************************************************************
*/
//Global parameters
const char* ssid1 = "";
const char* password1 = "";

const char* ssid2 = "";
const char* password2 = "";

const char* ssid3 = "";
const char* password3 = "";

const char* ssid4 = "";
const char* password4 = "";

const char* ssid5 = "";
const char* password5 = "";

//const char* ssid6       = "";
//const char* password6   = "";

const char* mqtt_server = "";  //ip broker mqtt


// value for MQTT broker.
String string_numeroposte = "003";
String clientIdMqtt = "poste" + string_numeroposte;
String clientIdMqttSensStatus = "poste" + string_numeroposte + "_status";
String clientIdMqttSensMeas = "poste" + string_numeroposte + "_meas";
String subscribe_str = "cmd_poste" + string_numeroposte;
String clientLoginMqtt = "";
String clientPassMqtt = "";


//End global parameter
//******************************************************************************************************************



/*
  ___________                   __  .__
  \_   _____/_ __  ____   _____/  |_|__| ____   ____   ______
   |    __)|  |  \/    \_/ ___\   __\  |/  _ \ /    \ /  ___/
   |     \ |  |  /   |  \  \___|  | |  (  <_> )   |  \\___ \
   \___  / |____/|___|  /\___  >__| |__|\____/|___|  /____  >
       \/             \/     \/                    \/     \/
*/

void print_wakeup_reason();

void reconnect();
void callback_mqtt(char* topic, byte* payload, unsigned int length);
void setup_wifi();
void print_wakeup_reason();
void void_fct_info_uart(unsigned long ulong_interval);

void send_status_mqtt(unsigned long ulong_interval);
void createJsonMessage(void);

//void sendMeasSensor(unsigned long ulong_interval);

void change_led(void);

void callback_mqtt(char* topic, byte* payload, unsigned int length);

void status_wifi(void);
void status_mqtt(void);

//watchdog reset
void IRAM_ATTR resetModule() {
  ets_printf("reboot watchdog\n");
  esp_restart();
}

// ------------------------INTERNAL TEMPERATURE ----------------
#ifdef __cplusplus
extern "C" {
#endif
  uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
// ---------------------FIN INTERNAL TEMPERATURE ----------------



/*
  ____   ____            .__      ___.   .__
  \   \ /   /____ _______|__|____ \_ |__ |  |   ____   ______
   \   Y   /\__  \\_  __ \  \__  \ | __ \|  | _/ __ \ /  ___/
    \     /  / __ \|  | \/  |/ __ \| \_\ \  |_\  ___/ \___ \
     \___/  (____  /__|  |__(____  /___  /____/\___  >____  >
                 \/              \/    \/          \/     \/
*/

GxIO_Class io(SPI, EPD_CS, EPD_DC, EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
SPIClass SDSPI(VSPI);
#endif

RTC_DATA_ATTR int bootCount = 0;
const int wdtTimeout = 60000;  //20000ms=20s time in ms to trigger the watchdog
hw_timer_t* timer = NULL;

int count_lost_mqtt = 0;
int count_lost_wifi = 0;

int intcounter = 0;
int i = 0;
int acq_restart = 1;
int update_screen = 1;

unsigned long ulong_time_now = 0;
unsigned long ulong_time_meas_cycle = 0;
unsigned long ulong_time_uart_cycle = 0;
unsigned long ulong_diff_time_meas_cycle = 0;
unsigned long ulong_time_send_meas_cycle = 0;

long int periodSendData = 0;
long int periodSendStat = 0;

//new variable wifi + MQTT
IPAddress Ip_Node_adress;
WiFiMulti wifiMulti;

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, callback_mqtt, espClient);

String string_value = "   ";
int value = 0;
int intTempCpu = -20;

// buffer envoi et reception message
char msg[150];
char buf[10];
char buffer_tmp[250];
char buffer_cmd[250];

int nb_test_ctl_wifi = 0;
int nb_test_ctl_mqtt = 0;

//variables pour la reception JSON MQTT
String stringCode = "";
String stringChannel = "";
String stringState = "";
int intChannel = 0;
int intState = 0;
int intDemandeCmd = 0;
int intErrorCmdReceive = 0;

//Variables emission trame JSON
String stringJsonEnvoi = "";
JSONVar JsonArrayEnvoi;

//environment sensor
float tmp_sensor = 0.0;
float hum_sensor = 0.0;
float pressure_sensor = 0.0;

int quantity = 0;
int restant = 0;

int j = 0;
int k = 0;

String refSachet = "          ";
String refOf = "          ";

Barcode barcode;
String barcodes = "";


/*
    _________       __
   /   _____/ _____/  |_ __ ________
   \_____  \_/ __ \   __\  |  \____ \
   /        \  ___/|  | |  |  /  |_> >
  /_______  /\___  >__| |____/|   __/
          \/     \/           |__|
*/
void setup() {

  //setup watchdog sur 10 secondes
  timer = timerBegin(0, 80, true);                   //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);   //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false);  //set time in us
  timerAlarmEnable(timer);                           //enable interrupt
  //timerAlarmDisable(timer);                          //disable interrupt

  //réarmement du watchdog (echoue si > 10secondes)
  timerWrite(timer, 0);  //reset timer (feed watchdog)


  Serial.begin(2000000);
  delay(1);
  ++bootCount;
  Serial.println("STARTING PROGRAM");
  Serial.println("***************************************************");
  Serial.println("Boot number: " + String(bootCount));

  //SPI screen
  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  //réarmement du watchdog (echoue si > 10secondes)
  timerWrite(timer, 0);  //reset timer (feed watchdog)

  //Init wifi
  setup_wifi();

  //verif wifi
  status_wifi();

  //réarmement du watchdog (echoue si > 10secondes)
  timerWrite(timer, 0);  //reset timer (feed watchdog)

  // abonnement MQTT
  client.setCallback(callback_mqtt);

  //connect MQTT
  reconnect();

  //init period data stat
  periodSendData = PERIOD_SEND_DATA;
  periodSendStat = PERIOD_SEND_STATS;
  //    display.init();
  //    display.setTextColor(GxEPD_BLACK);
  //    display.setRotation(1);

  if (update_screen == 1) {
    update_screen = 0;
    //init screen
    display.init();
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1);
    //display.fillScreen(GxEPD_WHITE);
    //display.setFont(&FreeMonoBold9pt7b);
    display.setFont(&FreeMono12pt7b);
    //long z = random(0, 20);
    display.setCursor(0, 10);
    display.println();
    display.println("-----------------");
    display.println(clientIdMqtt);
    display.println("Attente commande");
    display.println("-----------------");
    display.update();
    display.powerDown();
  }
}



/*
  .____
  |    |    ____   ____ ______
  |    |   /  _ \ /  _ \\____ \
  |    |__(  <_> |  <_> )  |_> >
  |_______ \____/ \____/|   __/
          \/            |__|
*/
void loop() {

  //period with millis()
  ulong_time_now = millis();

  //réarmement du watchdog (echoue si > 10secondes)
  timerWrite(timer, 0);  //reset timer (feed watchdog)

  //test wifi et reconnexion si probleme
  status_wifi();

  //status MQTT
  status_mqtt();

  client.loop();

  //esp_deep_sleep_start();
  //réarmement du watchdog (echoue si > 10secondes)
  timerWrite(timer, 0);  //reset timer (feed watchdog)


  //envoi status MQTT toute les 20 secondes (en vie)
  send_status_mqtt(periodSendStat);

  //no measure if no wifi
  if ((wifiMulti.run() == WL_CONNECTED) && ((client.connect(clientIdMqtt.c_str(), clientLoginMqtt.c_str(), clientPassMqtt.c_str())))) {
    if (acq_restart == 1) {
      acq_restart = 0;
      //led off
      Serial.println("Connexion MQTT OK");

      //affichage verte
      //setBuff(0x00, 0x44, 0x00);
      //M5.dis.displaybuff(DisBuff);
      //delay(2000);

      //effacement ecran
      //setBuff(0x00, 0x00, 0x00);
      //M5.dis.displaybuff(DisBuff);

      intcounter++;
      createJsonMessage();
      sprintf(msg, "{\"ssid\":\"%s\",\"rssi\":%d,\"counter\":%d,\"tempcpu\":%d}", WiFi.SSID().c_str(), WiFi.RSSI(), intcounter, intTempCpu);

      if (DEBUG_UART == 1) {
        Serial.print("Publish message: ");
        Serial.print(clientIdMqtt.c_str());
        Serial.print(" ");
        Serial.println(msg);
      }
      client.publish(clientIdMqtt.c_str(), msg);

      // init subscribe
      sprintf(buffer_tmp, "%s", subscribe_str.c_str());
      client.subscribe(buffer_tmp);

      //rechargement ecoute MQTT
      client.setCallback(callback_mqtt);
    }
  } else {
    Serial.println("Wifi ou MQTT PB");
    acq_restart = 1;
  }

  //debug uart
  if (DEBUG_UART == 1) {
    void_fct_info_uart(1000);
  }

  //Envoi trame RF si bonne reception MQTT
  if (intDemandeCmd == 1) {
    //to something if MQTT receive valid
    intDemandeCmd = 0;
    Serial.println("TRAME MQTT valid");
  }

  if (update_screen == 1) {
    update_screen = 0;

    if (quantity == 0) {
      //init screen
      display.init();
      display.setTextColor(GxEPD_BLACK);
      display.setRotation(1);
      //display.fillScreen(GxEPD_WHITE);
      //display.setFont(&FreeMonoBold9pt7b);
      display.setFont(&FreeMono12pt7b);
      //long z = random(0, 20);
      display.setCursor(0, 10);
      display.println();
      display.println("-----------------");
      display.println(clientIdMqtt);
      display.println("Attente commande");
      display.println("-----------------");
      display.update();
      display.powerDown();
    } else {
      //init screen
      display.init();
      display.setTextColor(GxEPD_BLACK);
      display.setRotation(0);
      //display.fillScreen(GxEPD_WHITE);
      //display.setFont(&FreeMonoBold9pt7b);
      display.setFont(&FreeMono12pt7b);
      //long z = random(0, 20);

      //Generatebar code
      barcodes = barcode.make(refSachet);
      int barLen = 0;
      int posBar = 0;
      int startBar = 0;
      for (int j = 0; j < barcodes.length(); j++) {
        barLen = barcodes[j] - '0';
        startBar = posBar;
        posBar += barLen;
        if (j % 2 == 0) {
          display.fillRect(0, startBar, 45, posBar, GxEPD_BLACK);
          // Serial.print("B-");
          // Serial.print(startBar);
          // Serial.print("-");
          // Serial.println(posBar);
        } else {
          display.fillRect(0, startBar, 45, posBar, GxEPD_WHITE);
          // Serial.print("W-");
          // Serial.print(startBar);
          // Serial.print("-");
          // Serial.println(posBar);
        }
      }

      display.fillRect(0, posBar, 45, display.height(), GxEPD_WHITE);
      //display.updateWindow(0, 0, 45, display.height(), true);


      display.setRotation(1);
      display.setCursor(0, 0);
      // display.println("-----------------");
      // display.print("OF : ");
      // display.println(refOf);
      display.println();
      display.print("Ref : ");
      display.println(refSachet);
      display.print("Quantite : ");
      display.println(quantity);
      //display.println("-----------------");
      display.update();

      //display.powerDown();
    }  //quantity not null
  }
}



//functions
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}



/*
    _________       __                  __      __.______________.___
   /   _____/ _____/  |_ __ ________   /  \    /  \   \_   _____/|   |
   \_____  \_/ __ \   __\  |  \____ \  \   \/\/   /   ||    __)  |   |
   /        \  ___/|  | |  |  /  |_> >  \        /|   ||     \   |   |
  /_______  /\___  >__| |____/|   __/    \__/\  / |___|\___  /   |___|
          \/     \/           |__|            \/           \/
*/
void setup_wifi() {

  //affichage rouge no wifi
  //setBuff(0x44, 0x00, 0x00);
  //M5.dis.displaybuff(DisBuff);

  //réarmement du watchdog (echoue si > 10secondes)
  timerWrite(timer, 0);  //reset timer (feed watchdog)

  //delai restart
  delay(random(500));  // Delay for a period of time (in milliseconds).

  //réarmement du watchdog (echoue si > 10secondes)
  timerWrite(timer, 0);  //reset timer (feed watchdog)

  wifiMulti.addAP(ssid1, password1);
  wifiMulti.addAP(ssid2, password2);
  wifiMulti.addAP(ssid3, password3);
  wifiMulti.addAP(ssid4, password4);
  wifiMulti.addAP(ssid5, password5);

  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected: ");
    Serial.println(WiFi.SSID());

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    //affichage bleu wifi ok MQTT wait
    //setBuff(0x00, 0x00, 0x44);
    //M5.dis.displaybuff(DisBuff);
    delay(1000);
    count_lost_wifi++;
  }

  Ip_Node_adress = WiFi.localIP();
}



/*
                .__  .__ ___.                  __        _____   ______________________________
    ____ _____  |  | |  |\_ |__ _____    ____ |  | __   /     \  \_____  \__    ___/\__    ___/
  _/ ___\\__  \ |  | |  | | __ \\__  \ _/ ___\|  |/ /  /  \ /  \  /  / \  \|    |     |    |
  \  \___ / __ \|  |_|  |_| \_\ \/ __ \\  \___|    <  /    Y    \/   \_/.  \    |     |    |
   \___  >____  /____/____/___  (____  /\___  >__|_ \ \____|__  /\_____\ \_/____|     |____|
       \/     \/              \/     \/     \/     \/         \/        \__>
*/
void callback_mqtt(char* topic, byte* payload, unsigned int length) {
  intDemandeCmd = 0;
  update_screen = 0;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print(" - long ");
  Serial.print(length);
  Serial.print("] ");
  string_value = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    string_value += (char)payload[i];
  }
  Serial.println();

  //Decodage JSON
  JSONVar myObject = JSON.parse(string_value);
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Echec du Parsing JSON!");
  } else {
    //json : {"qty":1,"sachet":0}
    if (myObject.hasOwnProperty("qty")) {
      intDemandeCmd = 1;
      update_screen = 1;

      //Serial.print("myObject[\"qty\"] = ");
      //Serial.print(myObject["qty"]);
      quantity = (int)myObject["qty"];
      //Serial.print(" - ");
      Serial.print("Qty-");
      Serial.println(quantity);
    }

    if (myObject.hasOwnProperty("sachet")) {
      //Serial.print("myObject[\"sachet\"] = ");
      //Serial.print(myObject["sachet"]);
      const char* refSachetValue = myObject["sachet"];
      refSachet = refSachetValue;
      Serial.print("Sachet - ");
      Serial.println(refSachet);
    }

    if (myObject.hasOwnProperty("numof")) {
      //Serial.print("myObject[\"numof\"] = ");
      //Serial.print(myObject["numof"]);
      const char* numofValue = myObject["numof"];
      refOf = numofValue;
      Serial.print("REF - ");
      Serial.println(refOf);
    }
  }

  value = string_value.toInt();

  if (string_value.compareTo("resetreset") == 0)
  //if (string_value=="resetreset")
  {
    Serial.println("Reset board");
    while (1)
      ;
  }

  /*Serial.print("value [");
    Serial.print(string_value);
    Serial.print(" - ");
    Serial.print(value);
    Serial.print("] ");
    Serial.println();*/
}



/*
  __________                                                 __       _____   ______________________________
  \______   \ ____   ____  ____   ____   ____   ____   _____/  |_    /     \  \_____  \__    ___/\__    ___/
   |       _// __ \_/ ___\/  _ \ /    \ /    \_/ __ \_/ ___\   __\  /  \ /  \  /  / \  \|    |     |    |
   |    |   \  ___/\  \__(  <_> )   |  \   |  \  ___/\  \___|  |   /    Y    \/   \_/.  \    |     |    |
   |____|_  /\___  >\___  >____/|___|  /___|  /\___  >\___  >__|   \____|__  /\_____\ \_/____|     |____|
          \/     \/     \/           \/     \/     \/     \/               \/        \__>
*/
void reconnect() {

  int compteur_mqtt = 0;

  //test wifi connexion
  if (wifiMulti.run() == WL_CONNECTED) {

    // Loop until we're reconnected
    while (!(client.connect(clientIdMqtt.c_str(), clientLoginMqtt.c_str(), clientPassMqtt.c_str()))) {
      //affichage bleu not mqtt
      //setBuff(0x00, 0x00, 0x44);
      //M5.dis.displaybuff(DisBuff);
      delay(2000);

      //réarmement du watchdog (echoue si > 10secondes)
      timerWrite(timer, 0);  //reset timer (feed watchdog)

      compteur_mqtt++;
      if (compteur_mqtt >= TEST_MQTT) break;

      Serial.print("Attente connexion MQTT...");

      //delai reboot
      delay(random(1000));  // Delay for a period of time (in milliseconds).

      // Create a random client ID
      //!!!clientId = "id_random_";
      //!!!clientId += String(random(0xffff), HEX);
      client.setCallback(callback_mqtt);

      // Attempt to connect
      if (client.connect(clientIdMqtt.c_str(), clientLoginMqtt.c_str(), clientPassMqtt.c_str())) {
        Serial.println("Connexion MQTT OK");

        //affichage verte
        //setBuff(0x00, 0x44, 0x00);
        //M5.dis.displaybuff(DisBuff);
        delay(2000);

        //effacement ecran
        //setBuff(0x00, 0x00, 0x00);
        //M5.dis.displaybuff(DisBuff);

        intcounter++;
        createJsonMessage();
        sprintf(msg, "{\"ssid\":\"%s\",\"rssi\":%d,\"counter\":%d,\"tempcpu\":%d}", WiFi.SSID().c_str(), WiFi.RSSI(), intcounter, intTempCpu);

        if (DEBUG_UART == 1) {
          Serial.print("Publish message: ");
          Serial.print(clientIdMqtt.c_str());
          Serial.print(" ");
          Serial.println(msg);
        }
        client.publish(clientIdMqtt.c_str(), msg);

        // init subscribe
        sprintf(buffer_tmp, "%s", subscribe_str.c_str());
        client.subscribe(buffer_tmp);

        //rechargement ecoute MQTT
        client.setCallback(callback_mqtt);
      } else {
        Serial.print("Echec, rc=");
        Serial.print(client.state());
        Serial.println(" Retest dans 5 secondes");
      }
      // Wait 5 seconds before retrying
      delay(5000);
    }

    if (client.connect(clientIdMqtt.c_str(), clientLoginMqtt.c_str(), clientPassMqtt.c_str())) {
      Serial.println("Connexion MQTT OK");
      count_lost_mqtt++;

      //effacement ecran
      //setBuff(0x00, 0x00, 0x00);
      //M5.dis.displaybuff(DisBuff);

      delay(2000);
      //delai reboot
      delay(random(200));  // Delay for a period of time (in milliseconds).

      intcounter++;
      createJsonMessage();
      sprintf(msg, "{\"ssid\":\"%s\",\"rssi\":%d,\"counter\":%d,\"tempcpu\":%d}", WiFi.SSID().c_str(), WiFi.RSSI(), intcounter, intTempCpu);

      if (DEBUG_UART == 1) {
        Serial.print("Publish message: ");
        Serial.print(clientIdMqtt.c_str());
        Serial.print(" ");
        Serial.println(msg);
      }
      client.publish(clientIdMqtt.c_str(), msg);

      // init subscribe
      sprintf(buffer_tmp, "%s", subscribe_str.c_str());
      client.subscribe(buffer_tmp);

      //rechargement ecoute MQTT
      client.setCallback(callback_mqtt);
    }

  }  //end test wifi connexion
}



/*
  .___        _____               ____ ___  _____ _____________________
  |   | _____/ ____\____  ______ |    |   \/  _  \\______   \__    ___/
  |   |/    \   __\/  _ \/  ___/ |    |   /  /_\  \|       _/ |    |
  |   |   |  \  | (  <_> )___ \  |    |  /    |    \    |   \ |    |
  |___|___|  /__|  \____/____  > |______/\____|__  /____|_  / |____|
           \/                \/                  \/       \/
*/
void void_fct_info_uart(unsigned long ulong_interval) {
  if ((ulong_time_now - ulong_time_uart_cycle) >= ulong_interval) {
    ulong_time_uart_cycle = ulong_time_now;

    //Measure internal temperature esp32
    intTempCpu = ((temprature_sens_read() - 32) / 1.8);
    Serial.print(intTempCpu);
    Serial.print(",");

    //period data
    Serial.print(periodSendData);
    Serial.print(",");

    //period stat
    Serial.print(periodSendStat);
    Serial.print(",");
  }
}




/*
    _________                  .___    _____   ______________________________
   /   _____/ ____   ____    __| _/   /     \  \_____  \__    ___/\__    ___/
   \_____  \_/ __ \ /    \  / __ |   /  \ /  \  /  / \  \|    |     |    |
   /        \  ___/|   |  \/ /_/ |  /    Y    \/   \_/.  \    |     |    |
  /_______  /\___  >___|  /\____ |  \____|__  /\_____\ \_/____|     |____|
          \/     \/     \/      \/          \/        \__>
*/
//Information status of led to the MQTT server
void send_status_mqtt(unsigned long ulong_interval) {
  if ((ulong_time_now - ulong_time_send_meas_cycle) >= ulong_interval) {
    ulong_time_send_meas_cycle = ulong_time_now;

    //Abonnement MQTT
    client.setCallback(callback_mqtt);

    if ((client.connect(clientIdMqttSensStatus.c_str(), clientLoginMqtt.c_str(), clientPassMqtt.c_str()))) {
      //envoi status MQTT
      intcounter++;
      createJsonMessage();
      sprintf(buffer_tmp, "{\"ssid\":\"%s\",\"rssi\":%d,\"counter\":%d,\"tempcpu\":%d}", WiFi.SSID().c_str(), WiFi.RSSI(), intcounter, intTempCpu);
      if (DEBUG_UART == 1) {
        Serial.print("Publish message: ");
        Serial.print(clientIdMqttSensStatus.c_str());
        Serial.print(" ");
        Serial.println(buffer_tmp);
      }
      client.publish(clientIdMqttSensStatus.c_str(), buffer_tmp);

      // init subscribe
      sprintf(buffer_tmp, "%s", subscribe_str.c_str());
      //Serial.println(buffer_tmp);
      client.subscribe(buffer_tmp);

      //rechargement ecoute MQTT
      client.setCallback(callback_mqtt);
    }
  }
}




/*
                               __              ____.                       _____
    ___________   ____ _____ _/  |_  ____     |    | __________   ____    /     \   ____   ______ ____
  _/ ___\_  __ \_/ __ \\__  \\   __\/ __ \    |    |/  ___/  _ \ /    \  /  \ /  \_/ __ \ /  ___// ___\
  \  \___|  | \/\  ___/ / __ \|  | \  ___//\__|    |\___ (  <_> )   |  \/    Y    \  ___/ \___ \/ /_/  >
   \___  >__|    \___  >____  /__|  \___  >________/____  >____/|___|  /\____|__  /\___  >____  >___  /
       \/            \/     \/          \/              \/           \/         \/     \/     \/_____/
*/
void createJsonMessage(void) {
  //trame JSON envoi
  JsonArrayEnvoi[0] = "ssid";
  JsonArrayEnvoi[1] = WiFi.SSID().c_str();
  JsonArrayEnvoi[2] = "rssi";
  JsonArrayEnvoi[3] = WiFi.RSSI();
  JsonArrayEnvoi[4] = "counter";
  JsonArrayEnvoi[5] = intcounter;
  JsonArrayEnvoi[6] = "tempcpu";
  JsonArrayEnvoi[7] = intTempCpu;

  stringJsonEnvoi = JSON.stringify(JsonArrayEnvoi);

  if (DEBUG_UART == 1) {
    Serial.print("Message JSON: ");
    Serial.println(stringJsonEnvoi);
  }
}




/*
    _________ __          __                    _____   ______________________________
   /   _____//  |______ _/  |_ __ __  ______   /     \  \_____  \__    ___/\__    ___/
   \_____  \\   __\__  \\   __\  |  \/  ___/  /  \ /  \  /  / \  \|    |     |    |
   /        \|  |  / __ \|  | |  |  /\___ \  /    Y    \/   \_/.  \    |     |    |
  /_______  /|__| (____  /__| |____//____  > \____|__  /\_____\ \_/____|     |____|
          \/           \/                \/          \/        \__>
*/
void status_mqtt(void) {
  //test wifi connexion before test MQTT
  if (wifiMulti.run() == WL_CONNECTED) {
    //test MQTT et reconnexion si probleme
    if (!(client.connect(clientIdMqtt.c_str(), clientLoginMqtt.c_str(), clientPassMqtt.c_str()))) {
      nb_test_ctl_mqtt++;
      if (nb_test_ctl_mqtt > TEST_MQTT) {
        nb_test_ctl_mqtt = 0;
        Serial.println("Deconnexion MQTT (pb MQTT)");
        reconnect();
      }
    } else {
    }
  }
}



/*
    _________ __          __                  __      __.______________.___
   /   _____//  |______ _/  |_ __ __  ______ /  \    /  \   \_   _____/|   |
   \_____  \\   __\__  \\   __\  |  \/  ___/ \   \/\/   /   ||    __)  |   |
   /        \|  |  / __ \|  | |  |  /\___ \   \        /|   ||     \   |   |
  /_______  /|__| (____  /__| |____//____  >   \__/\  / |___|\___  /   |___|
          \/           \/                \/         \/           \/
*/
void status_wifi(void) {

  if (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    nb_test_ctl_wifi++;
    if (nb_test_ctl_wifi > TEST_WIFI) {
      nb_test_ctl_wifi = 0;
      Serial.println("Deconnexion wifi (pb wifi)\r\n");
      setup_wifi();
      Serial.println("Deconnexion MQTT (pb wifi)\r\n");
      reconnect();
    }
  }
}
