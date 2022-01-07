#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdio.h>
#define SSID_LENGTH 40

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#define WIFI_SET_PIN 21
int inPin = 33;

const char* mqtt_server = "test.mosquitto.org";
String resultString;
String myArray[] = {"\"Ada_Kotipathi\":[\"2021-03-25\",\"17,32,33,71,X\",\"2021-03-24\",\"17,32,33,71,A\",\"2021-03-23\",\"17,32,33,71,B\",\"2021-03-22\",\"17,32,33,71,z\",\"2021-03-21\",\"17,32,33,71,z\"]","\"Development_Fortune\":[\"2021-03-25\",\"17,32,33,71,z\",\"2021-03-24\",\"17,32,33,71,z\",\"2021-03-23\",\"17,32,33,71,z\",\"2021-03-22\",\"17,32,33,71,z\",\"2021-03-21\",\"17,32,33,71,z\"]","\"Jayoda\":[\"2021-03-25\",\"17,32,33,71,z\",\"2021-03-24\",\"17,32,33,71,z\",\"2021-03-23\",\"17,32,33,71,z\",\"2021-03-22\",\"17,32,33,71,z\",\"2021-03-21\",\"17,32,33,71,z\"]","\"Kotipathi_Kapruka\":[\"2021-03-25\",\"17,32,33,71,z\",\"2021-03-24\",\"17,32,33,71,z\",\"2021-03-23\",\"17,32,33,71,z\",\"2021-03-22\",\"17,32,33,71,z\",\"2021-03-21\",\"17,32,33,71,z\"]","\"Saturday_Fortune\":[\"2021-03-25\",\"17,32,33,71,z\",\"2021-03-24\",\"17,32,33,71,z\",\"2021-03-23\",\"17,32,33,71,z\",\"2021-03-22\",\"17,32,33,71,z\",\"2021-03-21\",\"17,32,33,71,z\"]","\"Super_Ball\":[\"2021-03-25\",\"17,32,33,71,z\",\"2021-03-24\",\"17,32,33,71,z\",\"2021-03-23\",\"17,32,33,71,z\",\"2021-03-22\",\"17,32,33,71,z\",\"2021-03-21\",\"17,32,33,71,z\"]"};
String User;
String Date;
String Lottery;
String obj_history1;
String obj_history2;


WiFiClient client;
PubSubClient mqttClient(client);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

WiFiServer wifiServer(81);
WebServer server(80);

int client_count = 0;


int record_rst_time()
{
    int rst_time = 0;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
      
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle my_handle;                                 
    err = nvs_open("storage", NVS_READWRITE, &my_handle); 
    if (err != ESP_OK)
    
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");

        // Read
        printf("Reading restart counter from NVS ... ");
        int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(my_handle, "restart_counter", &restart_counter);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("Restart counter = %d\n", restart_counter);
            rst_time = restart_counter;
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Write
        printf("Updating restart counter in NVS ... ");
        restart_counter++;
        err = nvs_set_i32(my_handle, "restart_counter", restart_counter);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

    printf("\n");
    return rst_time;
}

void record_wifi(char *ssid, char *password)
{

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    printf("\n");
    printf("Opening Non-Volatile Wifi (NVS) handle... ");
    nvs_handle my_handle;                             
    err = nvs_open("Wifi", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");

        // Write
        printf("Updating ssid in NVS ... ");
        err = nvs_set_str(my_handle, "ssid", ssid);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        printf("Updating password in NVS ... ");
        err = nvs_set_str(my_handle, "password", password);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }

    printf("\n");
}


void check_wifi(char *ssid, char *password)
{
    char saved_ssid[SSID_LENGTH];
    size_t ssid_length = SSID_LENGTH;
    char saved_password[SSID_LENGTH];
    size_t password_length = SSID_LENGTH;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    printf("\n");
    printf("Opening Non-Volatile Wifi (NVS) handle... \n");
    nvs_handle my_handle;                             
    err = nvs_open("Wifi", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");

        // Read
        printf("Reading ssid and password from NVS ... \n");

        err = nvs_get_str(my_handle, "ssid", saved_ssid, &ssid_length);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("ssid: %s\n", saved_ssid);
            printf("ssid length= %d\n", ssid_length);
            strcpy(ssid, saved_ssid);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        err = nvs_get_str(my_handle, "password", saved_password, &password_length);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("password: %s\n", saved_password);
            printf("password length= %d\n", password_length);
            strcpy(password, saved_password);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Close
        nvs_close(my_handle);
    }

    printf("\n");
    return;
}

void ap_init()
{
    //WiFi.softAP(ssid, password);
    WiFi.softAP("ESP_AP");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    wifiServer.begin();
}

int wifi_config_server()
{

    WiFiClient client = wifiServer.available(); // listen for incoming clients

    if (client) // if you get a client,
    {
        Serial.println("---------------------------------------------------");
        Serial.printf("Index:%d\n", client_count);
        client_count++;
        Serial.println("New Client."); // print a message out the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                if (c == '\n')
                { // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        // the content of the HTTP response follows the header:
                        client.print("<h1>Lottery-Draw</h1><br><h2>ESP32 WIFI CONFIG</h2><br>");
                        client.print("Click <a href=\"/wifi_set\">here</a> to set WIFI.<br>");

                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
                //show wifiset page
                if (currentLine.endsWith("GET /wifi_set"))
                {
                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                    // and a content-type so the client knows what's coming, then a blank line:
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();

                    client.print("<h1>Lottery-Draw</h1><br><h2>ESP32 WIFI CONFIG</h2><br>");
                    client.print("<form action=\"/set_over\">SSID:<br><input type=\"text\" name=\"ssid\"><br>PASSWORD:<br><input type=\"text\" name=\"password\"><br><br>");
                    client.print("<input type=\"submit\" value=\"Set\"></form>");
                    // The HTTP response ends with another blank line:
                    client.println();
                    // break out of the while loop:
                    break;
                }

                if (currentLine.endsWith("GET /set_over"))
                {
                    String get_request = "";
                    //read GET next line
                    while (1)
                    {
                        char c_get = client.read();
                        Serial.write(c_get);
                        if (c_get == '\n')
                        {
                            break;
                        }
                        else
                        {
                            get_request += c_get;
                        }
                    }

                    //set_wifi_from_url(wifiServer.uri());
                    set_wifi_from_url(get_request);

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();

                    client.print("<h1>Lottery-Draw</h1><br><h2>ESP32 WIFI CONFIG</h2><br>");
                    client.print("Set Successful<br>");
                    client.println();

                    client.stop();
                    Serial.println("Client Disconnected.");

                    return 0;
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
    return 1;
}

void set_wifi_from_url(String get_url)
{
    //get_url = "http://192.168.4.1/set_over?ssid=Lottery-Draw&password=20160704"
    int str_len = 0;
    int ssid_add = 0;
    int pwd_add = 0;
    int end_add = 0;

    String ssid = "";
    String pwd = "";

    str_len = get_url.length();
    ssid_add = get_url.indexOf('?');
    pwd_add = get_url.indexOf('&');
    end_add = get_url.indexOf(' ');

    ssid = get_url.substring(ssid_add + 6, pwd_add);
    pwd = get_url.substring(pwd_add + 10, end_add);

    Serial.println();
    Serial.println("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");
    Serial.println("Get ssid and password from url:");
    Serial.println(get_url);
    Serial.println(ssid);
    Serial.println(pwd);
    Serial.println("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");

    record_wifi((char *)ssid.c_str(), (char *)pwd.c_str());
}

int wifi_set_main()
{
    char ssid[SSID_LENGTH];
    char password[SSID_LENGTH];
    pinMode(WIFI_SET_PIN, INPUT_PULLUP);

    check_wifi(ssid, password);

    Serial.println("Check WIFI_SET_PIN");
    int runtime = millis();
    int starttime = runtime;
    while ((runtime - starttime) < 3000)
    {
        if (digitalRead(WIFI_SET_PIN) == LOW)
        {

            Serial.println("Please connect ESP AP");
            Serial.println("And visit 192.168.4.1 to set WIFI.");
            Serial.println("");
            ap_init();
            while (wifi_config_server())
                ;
            delay(3000);
            esp_restart();
            return 0;
        }
        Serial.print(".");
        delay(100);
        runtime = millis();
    }
    Serial.println();

    //Connect wifi
    Serial.println("Connecting WIFI");
    WiFi.begin(ssid, password);

    int connect_count = 0;


    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        connect_count++;
        if (connect_count > 10)
            return 0;
    }


    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    return 1;
}

void nvs_test()
{
    char ssid[SSID_LENGTH] = "";
    char password[SSID_LENGTH] = "";
    int rst_time = 0;

    check_wifi(ssid, password);
    rst_time = record_rst_time();

    sprintf(ssid, "ssid_%d", rst_time);
    sprintf(password, "password_%d", rst_time);

    record_wifi(ssid, password);

    // Restart module
    for (int i = 10; i >= 0; i--)
    {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

void callback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic,"nnsaNODEToESP1")==0){
      String test1="";
      for (int i = 0; i < length; i++) {
        test1=test1+String((char)payload[i]);
      }
      test1=test1.substring(1,test1.length()-1);
      myArray[0] = test1;
      }
      if (strcmp(topic,"nnsaNODEToESP2")==0){
      String test2="";
      for (int i = 0; i < length; i++) {
        test2=test2+String((char)payload[i]);
      }
      test2=test2.substring(1,test2.length()-1);
      myArray[1] = test2;
      }
      if (strcmp(topic,"nnsaNODEToESP3")==0){
      String test3="";
      for (int i = 0; i < length; i++) {
        test3=test3+String((char)payload[i]);
      }
      test3=test3.substring(1,test3.length()-1);
      myArray[2] = test3;
      }
      if (strcmp(topic,"nnsaNODEToESP4")==0){
      String test4="";
      for (int i = 0; i < length; i++) {
        test4=test4+String((char)payload[i]);
      }
      test4=test4.substring(1,test4.length()-1);
      myArray[3] = test4;
      }
      if (strcmp(topic,"nnsaNODEToESP5")==0){
      String test5="";
      for (int i = 0; i < length; i++) {
        test5=test5+String((char)payload[i]);
      }
      test5=test5.substring(1,test5.length()-1);
      myArray[4] = test5;
      }
      if (strcmp(topic,"nnsaNODEToESP6")==0){
      String test6="";
      for (int i = 0; i < length; i++) {
        test6=test6+String((char)payload[i]);
      }
      test6=test6.substring(1,test6.length()-1);
      myArray[5] = test6;
      }

      if (strcmp(topic,"nnspSLEEP")==0){
          if ((char)payload[0] == '1') {
            Serial.println("Going to sleep now");
            esp_deep_sleep_start();}
      }
   }

  void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random mqttClient ID
    String mqttClientId = "ESP32mqttClient-";
    mqttClientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(mqttClientId.c_str())) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      mqttClient.publish("nnsaWAKEUP", "hELLO!");
      // ... and resubscribe
      mqttClient.subscribe("nnsaNODEToESP1");
      mqttClient.subscribe("nnsaNODEToESP2");
      mqttClient.subscribe("nnsaNODEToESP3");
      mqttClient.subscribe("nnsaNODEToESP4");
      mqttClient.subscribe("nnsaNODEToESP5");
      mqttClient.subscribe("nnsaNODEToESP6");
      mqttClient.subscribe("nnspSLEEP");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void handle_OnConnect() {
  server.send(200, "text/html", connectHTML()); 
}
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
  }
void handle_data(){
  Date = server.arg("date");
  Lottery = server.arg("lottery");
  resultString = "{"+myArray[0]+","+myArray[1]+","+myArray[2]+","+myArray[3]+","+myArray[4]+","+myArray[5]+"}";
  server.send(200, "application/json", resultString);
  }
void handle_results(){
  if (server.arg("User") != ""){
      User = server.arg("User");
      Serial.println(User);
  }
  server.send(200, "text/html", SendHTML());
  }

String connectHTML(){
  String ptr="<!DOCTYPE html>\n";
ptr+="<html>\n";
  ptr+="<head>\n";
    ptr+="<meta charset=\"UTF-8\">\n";
    ptr+="<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    ptr+="<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js\"></script>\n";
    ptr+="<title>User</title>\n";
      ptr+="<style>\n";
           ptr+="body {color: #434343;font-family: \"Helvetica Neue\",Helvetica,Arial,sans-serif;font-size: 14px;background-color: #eeeeee; margin-top: 100px;}\n";
           ptr+=".container {margin: 0 auto;max-width: 400px;padding: 30px;box-shadow: 0 10px 20px rgba(0,0,0,0.19), 0 6px 6px rgba(0,0,0,0.23);background-color: #ffffff;}\n";
           ptr+=" h1 {text-align: center;margin-bottom: 40px;margin-top: 0px;color: #939393;font-size: 35px;}\n";
           ptr+=" form .field-group {box-sizing: border-box;clear: both;padding: 4px 0;position: relative;margin: 1px 0;width: 100%;}\n";
           ptr+="form .field-group > label {color: #757575;display: block;margin: 0 0 5px 0;padding: 5px 0 0;position: relative;word-wrap: break-word;}\n";
           ptr+="input[type=text] {font-size: 15px;margin-bottom: 4%;-webkit-appearance: none;display: block;background: #fafafa;color: #636363;width: 100%;border: none;border-radius: 0;border-top: none;border-left: none;border-right: none;border-bottom: 1px solid #00bcd4;background-color: transparent;}\n";
           ptr+="input[type=text]:focus {border-color: #4C669F;outline: 0;}\n";
           ptr+=".button-container {box-sizing: border-box;clear: both;margin: 1px 0 0;padding: 4px 0;position: relative;width: 100%;}\n";
           ptr+="button[type=submit] {background: #00bcd4;border: none;border-radius: 30px;color: #ffffff;cursor: pointer;display: block;font-weight: bold;font-size: 16px;padding: 15px 0;margin-top: 12%;text-align: center;text-transform: uppercase;width: 100%;-webkit-transition: background 250ms ease;-moz-transition: background 250ms ease;-o-transition: background 250ms ease;transition: background 250ms ease;}\n";
        ptr+="</style>\n";
    ptr+="</head>\n";
    ptr+="<body>\n";
        ptr+="<div class=\"container\">\n";
            ptr+="<h1 style=\"text-align: center;\">User</h1>\n";
                ptr+="<div class=\"field-group\">\n";
                    ptr+="<input id=\"First\" type=\"text\" length=32 placeholder=\"First Name\">\n";
                ptr+="</div>\n";
                ptr+="<div class=\"field-group\">\n";
                    ptr+="<input id=\"Last\" type=\"text\" length=32 placeholder=\"Last Name\">\n";
                ptr+="</div>\n";
                ptr+="<div  class=\"button-container\">\n";
                    ptr+="<button id=\"b1\" type=\"submit\" onclick=\"window.location.href='/results';\">Connect</button>\n";
                ptr+="</div>\n";
                ptr+="</div>\n";
        ptr+="<script>\n";
            ptr+="$(\"#b1\").click(function() {\n";
                ptr+="var first = $(\"#First\").val();\n";
                ptr+="var last = $(\"#Last\").val();\n";
                ptr+="var url = \"/results?User=\"+first+\"_\"+last\n";
                ptr+="var xhttp = new XMLHttpRequest();\n";
                ptr+="xhttp.onreadystatechange = function() {\n";
                
                ptr+="}\n";
                ptr+="xhttp.open(\"GET\",url, true);\n";
                ptr+="xhttp.send();\n";
  
                ptr+="});\n";
            
        ptr+="</script>\n";
    ptr+="</body>\n";
ptr+="</html>\n";

  return ptr;
  }



  
String SendHTML(){
  String ptr= "<!DOCTYPE html>\n";
ptr+="<html>\n";
  ptr+="<head>\n";
    ptr+="<meta charset=\"UTF-8\">\n";
    ptr+="<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    ptr+="<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js\"></script>\n";
    ptr+="<title>Lottery Results</title>\n";
      ptr+="<style>\n";
          ptr+=" body {color: #434343;font-family: \"Helvetica Neue\",Helvetica,Arial,sans-serif;font-size: 14px;background-color: #eeeeee;}\n";
          ptr+=".container {margin: 0 auto;max-width: 400px;padding: 5px;box-shadow: 0 10px 20px rgba(0,0,0,0.19), 0 6px 6px rgba(0,0,0,0.23);background-color: #ffffff;}\n";
          ptr+=" h1 {text-align: center;margin-bottom: 20px;margin-top: 0px;color: #1f1fb8;font-size: 25px;}\n";
          ptr+=".lottery_box{border: 3px solid royalblue;padding: 10px 10px 0px 10px;}\n";
          ptr+=".lottery_name{color: #292826;padding-left: 10px;padding-right: 10px;font-size: 20px;display: block;margin: 0 auto;border: 0;outline: 0;text-align: center;background-color:#F9D342 ;}\n";
          ptr+=".results{margin-top: 10px;margin-bottom: 20px;display: flex;justify-content: center;}\n"; 
          ptr+=".number{padding: 6px;min-width: 20px;border-radius: 5px;margin-right: 10px;background-color: #f05151;color: white;font-weight: bold;font-size: 20px;}\n";
          ptr+=".letter{padding:6px 8px 6px 8px;min-width: 14px;border-radius: 5px;margin-left: 15px;background-color: #383834;color: white;font-weight: bold;font-size: 20px;}\n";
          ptr+=".date_dropdown{margin-left: 5px;border: 0;outline: 0;background-color: lightblue;padding-left: 10px;padding-right: 10px;}\n";
          ptr+=".date{display: flex;justify-content: center;margin-top: 20px;font-size: 15px;}\n";
       ptr+="</style>\n";
  ptr+="</head>\n"; 

  ptr+="<body>\n";
    ptr+="<div class=\"container\">\n";

      ptr+="<h1>Lottery Results</h1>\n";

      ptr+="<div class=\"lottery_box\">\n";
            ptr+="<select id=\"lottery_name\" class=\"lottery_name\">\n";
              ptr+="<option selected disabled hidden>Choose Here</option>\n";
              ptr+="<option>Ada Kotipathi</option>\n";
              ptr+="<option>Development Fortune</option>\n";
              ptr+="<option>Jayoda</option>\n";
              ptr+="<option>Kotipathi Kapruka</option>\n";
              ptr+="<option>Saturday Fortune</option>\n";
              ptr+="<option>Super Ball</option>\n";
            ptr+="</select>\n";
            
            ptr+="<div class=\"date\"> Select Date:\n";
              ptr+="<select id=\"date_dropdown\" class=\"date_dropdown\">\n"; 
                  ptr+="<option id=\"d1\">Date</option>\n";
                  ptr+="<option id=\"d2\">Date</option>\n"; 
                  ptr+="<option id=\"d3\">Date</option>\n";
                  ptr+="<option id=\"d4\">Date</option>\n";
                  ptr+="<option id=\"d5\">Date</option>\n";
              ptr+="</select>\n";
            ptr+="</div>\n";

            ptr+="<div class=\"results\">\n";
              ptr+="<span id=\"n1\" class=\"number\">N1</span>\n";
              ptr+="<span id=\"n2\" class=\"number\">N2</span>\n";
              ptr+="<span id=\"n3\" class=\"number\">N3</span>\n";
              ptr+="<span id=\"n4\" class=\"number\">N4</span>\n";
              ptr+="<span id=\"L1\" class=\"letter\">L</span>\n";
            ptr+="</div>\n";
      ptr+="</div>\n";

    ptr+="</div>\n";
    ptr+="<script>\n";
      ptr+="var str_obj;\n";
       
      ptr+="var date;\n";
      ptr+="var lottery;\n";
      
      ptr+="setInterval(function ( ) {\n"; 
        ptr+="var xhttp = new XMLHttpRequest();\n";
        ptr+="xhttp.onreadystatechange = function() {\n";
          ptr+="if (this.readyState == 4 && this.status == 200) {\n";
            ptr+="var ai = this.responseText;\n";
            //ptr+="console.log(ai);\n";
            ptr+="str_obj = JSON.parse(ai);\n";
            
          ptr+="}\n";
        ptr+="}\n";
       ptr+="var url2 = \"/data?date=\"+date+\"&lottery=\"+lottery\n";
       ptr+="xhttp.open(\"GET\",url2, true);\n";
       ptr+="xhttp.send();\n";
      ptr+="}, 1000);\n";
 

      ptr+="$(function() {\n";

        ptr+="function DateUpdate(key){\n";
          ptr+="var i;\n";
          ptr+="for (i = 1; i < 6; i++) {\n";
            ptr+="var x1 = \"d\"+ i.toString();\n";
            ptr+="document.getElementById(x1).innerHTML = str_obj[key][2*(i-1)];\n";
          ptr+="}\n";
        ptr+="}\n";

        ptr+="function ValueUpdate(key,id1){\n";
          ptr+="var all_dates = [str_obj[key][1],str_obj[key][3],str_obj[key][5],str_obj[key][7],str_obj[key][9]];\n";
          ptr+="var num_array = all_dates[id1-1].split(\",\");\n";
         // ptr+="console.log(all_dates);\n";
         ptr+="document.getElementById(\"n1\").innerHTML = num_array[0]\n";;
          ptr+="document.getElementById(\"n2\").innerHTML = num_array[1];\n";
          ptr+="document.getElementById(\"n3\").innerHTML = num_array[2];\n";
          ptr+="document.getElementById(\"n4\").innerHTML = num_array[3];\n";
          ptr+="document.getElementById(\"L1\").innerHTML = num_array[4];\n";
        ptr+="}\n";

        ptr+="$(\"#lottery_name\").change(function(){\n";
           ptr+="lottery = $(\"#lottery_name option:selected\").text()\n";
        
          ptr+="if ($(\"#lottery_name option:selected\").text() == \"Ada Kotipathi\"){\n";
            ptr+="var key1 = \"Ada_Kotipathi\"\n";
            ptr+="DateUpdate(\"Ada_Kotipathi\")\n";
            ptr+="$(\"#n4\").css(\"background-color\",\"#f05151\")\n";
          ptr+="}else if ($(\"#lottery_name option:selected\").text() == \"Development Fortune\") {\n";
            ptr+="key1 = \"Development_Fortune\"\n";
            ptr+=" DateUpdate(\"Development_Fortune\")\n";
            ptr+="$(\"#n4\").css(\"background-color\",\"purple\")\n";
          ptr+="}else if ($(\"#lottery_name option:selected\").text() == \"Jayoda\") {\n";
            ptr+="key1 = \"Jayoda\"\n";
            ptr+=" DateUpdate(\"Jayoda\")\n";
            ptr+="$(\"#n4\").css(\"background-color\",\"#f05151\")\n";
          ptr+="}else if ($(\"#lottery_name option:selected\").text() == \"Kotipathi Kapruka\") {\n";
            ptr+="key1 = \"Kotipathi_Kapruka\"\n";
            ptr+="DateUpdate(\"Kotipathi_Kapruka\")\n";
            ptr+="$(\"#n4\").css(\"background-color\",\"#f05151\")\n";
          ptr+="}else if ($(\"#lottery_name option:selected\").text() == \"Saturday Fortune\") {\n";
            ptr+="key1 = \"Saturday_Fortune\"\n";
            ptr+="DateUpdate(\"Saturday_Fortune\")\n";
            ptr+="$(\"#n4\").css(\"background-color\",\"#f05151\")\n";
          ptr+="}else{\n";
            ptr+="key1 = \"Super_Ball\"\n";
            ptr+="DateUpdate(\"Super_Ball\")\n";
            ptr+="$(\"#n4\").css(\"background-color\",\"#f05151\")\n";
          ptr+="}\n";
          ptr+="var id2 = $(\"#date_dropdown\").children(\":selected\").attr(\"id\");\n";
          ptr+="date = $(\"#date_dropdown option:selected\").text()\n";
          ptr+="id2 = id2[1];\n";
          ptr+="ValueUpdate(key1,id2);\n";

        ptr+="})\n";
      
      ptr+="$(\"#date_dropdown\").change(function(){\n";
        ptr+="var id1 = $(this).children(\":selected\").attr(\"id\");\n";
        ptr+="id1 = id1[1];\n";
        ptr+="date = $(\"#date_dropdown option:selected\").text()\n";
        
        ptr+="if ($(\"#lottery_name option:selected\").text() == \"Ada Kotipathi\"){\n";
            ptr+="ValueUpdate (\"Ada_Kotipathi\",id1);\n";
          ptr+="}else if ($(\"#lottery_name option:selected\").text() == \"Development Fortune\") {\n";
            ptr+="ValueUpdate (\"Development_Fortune\",id1)\n";
          ptr+="}else if ($(\"#lottery_name option:selected\").text() == \"Jayoda\") {\n";
            ptr+="ValueUpdate(\"Jayoda\",id1)\n";
          ptr+="}else if ($(\"#lottery_name option:selected\").text() == \"Kotipathi Kapruka\") {\n";
            ptr+="ValueUpdate(\"Kotipathi_Kapruka\",id1)\n";
          ptr+="}else if ($(\"#lottery_name option:selected\").text() == \"Saturday Fortune\") {\n";
            ptr+="ValueUpdate(\"Saturday_Fortune\",id1)\n";
          ptr+="}else{\n";
            ptr+="ValueUpdate(\"Super_Ball\",id1)\n";
          ptr+="}\n";
      ptr+="})\n";
      ptr+="})\n";

    ptr+="</script>\n";
  ptr+="</body>\n";
ptr+="</html>\n";
  return ptr;
  }

  
void setup()
{
    Serial.begin(115200);
    delay(100);
    
    pinMode(inPin, INPUT);
    
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);

    if (wifi_set_main())
    {
        Serial.println("Connect WIFI SUCCESS");
        server.begin();
        Serial.println("HTTP server started");
      
        mqttClient.setServer(mqtt_server, 1883);
        mqttClient.setCallback(callback);
      
        server.on("/", handle_OnConnect);
        server.onNotFound(handle_NotFound);
        server.on("/data",handle_data);
        server.on("/results",handle_results);
    }
    else
    {
        Serial.println("Connect WIFI FAULT");
    }
}

void loop()
{
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  server.handleClient();
  //Serial.println(User);

 
  obj_history2 = "{\"H\":[\""+User+"\",\""+Date+"\",\""+Lottery+"\"]}";

  if ((obj_history2 != obj_history1) && (User != "")&&(Date != "")&&(Lottery != "")&&(Date != "undefined")&&(Lottery != "undefined")){
    Serial.println(obj_history2);
    mqttClient.publish("nnsaESPtoNODE", (char*) obj_history2.c_str());
    }
  obj_history1 = obj_history2;
  
}
