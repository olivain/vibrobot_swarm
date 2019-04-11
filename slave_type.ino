//slave sketch (27 03 2019)

#include <Servo.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h> //NEED THIS TO COMPILE

uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x38};


int DC_SPEED = 0;
int SERVO_POS = 90;
int TIP120pin = 12;
int servopin = 13;
Servo servomotor;

struct __attribute__((packed)) DataStruct {
    char text[32];
    unsigned int time;
};

DataStruct myData;


// it seems that the mac address needs to be set before setup() is called
//      and the inclusion of user_interface.h facilitates that
//      presumably there is a hidden call to the function initVariant()
void initVariant() {
  WiFi.mode(WIFI_AP);
  //wifi_set_macaddr(SOFTAP_IF, &mac[0]);
    esp_wifi_set_mac(ESP_IF_WIFI_AP, &mac[0]); // esp32 code

}

// callback function linked to the "message received" trigger :
void receiveCallBackFunction(const uint8_t *senderMac, const uint8_t *incomingData, int len) {
  
  char *ptr;
  char *i;
  memcpy(&myData, incomingData, sizeof(myData));
  
  ptr = strtok_r(myData.text, ":", &i);


  if(strcmp("dc",ptr) == 0) { // dc motor speed
      char *speed_nb = strtok_r(NULL, ":", &i);
      DC_SPEED = atoi(speed_nb);
      Serial.print("DC = ");
      Serial.println(speed_nb);
      
  } else if(strcmp("sv",ptr) == 0) { // servo position
      char *pos_nb = strtok_r(NULL,":",&i);
      SERVO_POS = atoi(pos_nb);
     Serial.print("SERVO = ");
     Serial.println(pos_nb);
      
  }
  
}


// setup function (executed only once)
void setup() {
    Serial.begin(115200);
    Serial.setTimeout(10); // 10ms serial timeout
    randomSeed(analogRead(0));

    pinMode(TIP120pin, OUTPUT); // Set pin for output to control TIP120 Base pin
    servomotor.attach(servopin);
    digitalWrite(TIP120pin, 0); // By changing values from 0 to 255 you can control motor speed
    servomotor.write(SERVO_POS);
    delay(1000);

    Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
    Serial.print("This node STA mac: "); Serial.println(WiFi.macAddress());

    while(esp_now_init()!=ESP_OK) {
        Serial.println("*** ESP_Now init failed");
        delay(500);
    }

    esp_now_register_recv_cb(receiveCallBackFunction);
    Serial.println("End of setup - waiting for messages");

    Serial.println("test motors");
    digitalWrite(TIP120pin, 1); // By changing values from 0 to 255 you can control motor speed
    delay(500);
    servomotor.write(rand()%140);
    delay(1000);
    digitalWrite(TIP120pin, 0); // By changing values from 0 to 255 you can control motor speed
Serial.println("End of test motors");

DC_SPEED = 0;
SERVO_POS = 0;

}


void loop() {
  digitalWrite(TIP120pin, DC_SPEED); // By changing values from 0 to 255 you can control motor speed
  servomotor.write(SERVO_POS);
}
