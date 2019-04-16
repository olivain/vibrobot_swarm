#include <HardwareSerial.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h> //NEED THIS TO COMPILE
#include <SimpleMap.h>  
#include "Quaternion.h"


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
     

       Serial.println("wait a bit to be sure cam is on");

   // delay(5000);

   
    Serial.println("ok go");
    MySerial.println("setmapping2 YUYV 640 480 20.0 JeVois DemoArUco");
    delay(50);
    MySerial.println("setpar dopose true");
    delay(50);
    MySerial.println("setpar serlog None");
    delay(50);
    MySerial.println("setpar serout Hard");
    delay(50);
    MySerial.println("setpar serstyle Detail");
    delay(50);
    MySerial.println("setpar serprec 5");
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
Quaternion quat;

void loop() {
  
   strcpy(myData.text, "dc:1");
   sendData((uint8_t*)NULL);
   delay(10);
   
  int target_x = 400; // right : -400 <-> left : +400
  int target_y = 300; // top : -300 <-> bottom : +300
  
  byte len = MySerial.readBytesUntil('\n', instr, INLEN);
  instr[len] = 0;
  
    //Serial.println(instr);
      char * tok = strtok(instr, " \r\n");
      int nb = 0;
      while(tok) {
        arr_t[nb] = tok;
        tok = strtok(0, " \r\n");
        nb++;
      }

    if(nb > 0) {
      //Serial.println(nb);
      if(nb > 9) {

        
        for(int v=0; v < nb; v++) {
        Serial.print(v);
        Serial.print(" = ");
        Serial.println(arr_t[v]);
        
        }

        
       quat.a = atof(arr_t[8]);
      quat.b = atof(arr_t[9]);
      quat.c = atof(arr_t[10]);
      quat.d = atof(arr_t[11]);
      int x = atoi(arr_t[2]);
      int y = atoi(arr_t[3]);

      

      quat.normalize(); 

      Quaternion dir;
      dir.a = 0;
      dir.b = 0;
      dir.c = 1;
      dir.d = 0;
      Quaternion r = quat.rotate(dir);

      float tx = target_x - x;
      float ty = target_y - y;

      float mag = sqrt(tx*tx + ty*ty);

      float ntx = tx/mag;
      float nty = ty/mag;

      float c_alpha = r.b*ntx + r.c*nty;
      float s_alpha = r.b*nty - r.c*ntx;

      float alpha = atan2(s_alpha,c_alpha)*180/PI;

      
      Serial.print("alpha = ");
      Serial.println(alpha);

      if(alpha > -33 && alpha < 33) {
        strcpy(myData.text, "sv:0");
        sendData((uint8_t*)remoteMac[atoi(mac_map->get(arr_t[1]).c_str())]);
        
        // turn right : 150
        // turn left: 35
        
      } else if(alpha < 0) {
        // turn left
        strcpy(myData.text, "sv:35");
        sendData((uint8_t*)remoteMac[atoi(mac_map->get(arr_t[1]).c_str())]);
         
      } else if(alpha > 0) {
        // turn right
        strcpy(myData.text, "sv:150");
        sendData((uint8_t*)remoteMac[atoi(mac_map->get(arr_t[1]).c_str())]);
         
      }
      
/*

        Serial.print("dir : ");
        Serial.println(r.a);
        Serial.println(r.b);
        Serial.println(r.c);
        Serial.println(r.d);
        Serial.print("w : ");
        Serial.println(quat.a);
        Serial.print("x : ");
        Serial.println(quat.b);
        Serial.print("y : ");
        Serial.println(quat.c);
        Serial.print("z : ");
        Serial.println(quat.d);

        double angle = atan2(r.b, r.c)*180/PI;
        Serial.println(angle);
        
         
        double head = atan2(2*quat.c*quat.a-2*quat.b*quat.d , 1 - 2*(quat.c*quat.c) - 2*(quat.d*quat.d))*180/PI;
        Serial.print("atan2 (head) : ");
        Serial.println(head);

        
        float delta_x = 320 - x;
        float delta_y = 240 - y;
        double theta = atan2(delta_y, delta_x)*180/PI;
        Serial.print("theta : ");
        Serial.println(theta);

        double rot_z = atan2(quat.d, quat.a)*180/PI;
        Serial.print("rot_z: ");
        Serial.println(rot_z);
        */
        
        /*if(quat.b > 0 && quat.c < 0) {
          strcpy(myData.text, "sv:60");
          //sendData((uint8_t*)remoteMac[atoi(mac_map->get(arr_t[1]).c_str())]);
          
        }*/
     
      }
    }
     
      /*
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

             // Serial.print(distSq);
             // Serial.print(radSumSq);
              sendData((uint8_t*)remoteMac[atoi(mac_map->get(arr_t[1]).c_str())]);
              //delay(250);
          
            }
                
          }
          
          
          
      } */
      memset(instr,0,INLEN);
}
   
