#include <HardwareSerial.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h> //NEED THIS TO COMPILE
#include <SimpleMap.h>  


HardwareSerial MySerial(1);
String aruco_data[11];
SimpleMap<String, String>* mac_map;

struct __attribute__((packed)) DataStruct {
    char text[32];
    unsigned int time;
};

DataStruct myData;

#define NB_SLAVES 5
uint8_t remoteMac[][6] = {
  {0x36, 0x33, 0x33, 0x33, 0x33, 0x34},
  {0x36, 0x33, 0x33, 0x33, 0x33, 0x35},
  {0x36, 0x33, 0x33, 0x33, 0x33, 0x36},
  {0x36, 0x33, 0x33, 0x33, 0x33, 0x37},
  {0x36, 0x33, 0x33, 0x33, 0x33, 0x38}
};
float circlepoints[NB_SLAVES][2] = {{0,0},{0,0},{0,0},{0,0},{0,0}};


esp_now_peer_info_t slave[NB_SLAVES]; // ?????


void sendData(const uint8_t *dest_mac) {
        myData.time = millis();
        uint8_t bs[sizeof(myData)];
        memcpy(bs, &myData, sizeof(myData));
        esp_now_send((uint8_t *)dest_mac, bs, sizeof(myData)); // NULL means send to all peers
        Serial.print("sent data : ");
        Serial.println(myData.text);
        delay(10);
}


void sendCallBackFunction(const uint8_t* destination_mac_addr, esp_now_send_status_t uploadStatus) {
    // what to do when the master send a message
      Serial.println(uploadStatus == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");

}

void setup() {
    Serial.begin(115200);
    MySerial.begin(115200, SERIAL_8N1, 16, 17);
   MySerial.setTimeout(10);
     randomSeed(analogRead(0));

   
    delay(100);
    Serial.println("ok go");
    MySerial.println("setmapping2 YUYV 640 480 20.0 JeVois DemoArUco");
    delay(50);
    MySerial.println("setpar serlog None");
    delay(50);
    MySerial.println("setpar serout Hard");
    delay(50);
    MySerial.println("setpar serstyle Detail");
    delay(50);
    MySerial.println("setpar serprec 5");
    delay(50);
    MySerial.println("setpar dopose true");
    delay(50);
    MySerial.println("streamon");
    delay(50);
    Serial.println("is aruco running ???");

    mac_map = new SimpleMap<String, String>([](String& a, String& b) -> int {
        if (a == b) return 0;

        if (a > b) return 1;

        /*if (a < b) */ return -1;
    });

    /*circlepoints_map = new SimpleMap<String, String>([](String& a, String& b) -> int {
        if (a == b) return 0;

        if (a > b) return 1;

        /*if (a < b) *!/ return -1;
    }); */

    mac_map->put("U49","0");
    
    mac_map->put("U48","1");
    
    mac_map->put("U47","2");
    
    mac_map->put("U46","3");
    
    mac_map->put("U45","4");

    
    Serial.printf("setting wifi station mode..\n");
    WiFi.mode(WIFI_STA); // Station mode for esp-now controller
    WiFi.disconnect();
    Serial.printf("done.\n");
    
    Serial.printf("master's mac: %s, ", WiFi.macAddress().c_str());

    while(esp_now_init()!=ESP_OK) {
        Serial.println("*** ESP_Now init failed");
        delay(500);
    }
    Serial.printf("pairing with slaves..\n");
    for(int y = 0; y < NB_SLAVES; y++) {
       // esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
        for (int ii = 0; ii < 6; ++ii ) {
                slave[y].peer_addr[ii] = (uint8_t) remoteMac[y][ii];
              }
        esp_now_add_peer(&slave[y]);
        Serial.println((char *)slave[y].peer_addr);
    }
    Serial.printf("done.\n");
    
    Serial.printf("Setting Callback func..\n");
    esp_now_register_send_cb(sendCallBackFunction);
    Serial.printf("done.\n");

   strcpy(myData.text, "dc:1");
   sendData((uint8_t*)NULL);
          
    
}


// Buffer for received serial port bytes:
#define INLEN 128
char instr[INLEN + 1];
char *arr_t[12] = {};

void loop() {
  
  byte len = MySerial.readBytesUntil('\n', instr, INLEN);
  instr[len] = 0;
  
      Serial.println(instr);
      char * tok = strtok(instr, " \r\n");
      int nb = 0;
      while(tok) {
        arr_t[nb] = tok;
        tok = strtok(0, " \r\n");
        nb++;
      }
      if(mac_map->has(arr_t[1])) {

          float x1 = atof(arr_t[2]); // center of the aruco tag
          float y1 = atof(arr_t[3]); // center of the aruco tag
          int nb_bot = atoi(mac_map->get(arr_t[1]).c_str());
          
          for(int i = 0; i < NB_SLAVES-1; i++) {
            
            if(nb != i) {
              int r1 = 350;
              int r2 = 350;
              float x2 = circlepoints[i][0];
              float y2 = circlepoints[i][1];
              float distSq = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
              float radSumSq = (r1 + r2) * (r1 + r2);

              

              if (distSq == radSumSq){
                  //strcpy(myData.text, "sv:65");
                  sprintf_P(myData.text, "sv:%i", (rand()%170 + 10));
              }else if (distSq > radSumSq){
                  strcpy(myData.text, "dc:1");
              }else{
                  //strcpy(myData.text, "sv:140");
                  sprintf_P(myData.text, "sv:%i", (rand()%170 + 10));
              }

              Serial.print(distSq);
              Serial.print(radSumSq);
              sendData((uint8_t*)remoteMac[atoi(mac_map->get(arr_t[1]).c_str())]);
              //delay(250);
          
            }
                
          }
          
          //strcpy(myData.text, "sv:0");
          //sendData((uint8_t*)remoteMac[atoi(mac_map->get(arr_t[1]).c_str())]);
          
      }
      memset(instr,0,INLEN);
}
   
