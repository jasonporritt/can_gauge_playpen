/* Welcome to the ECU Reader project. This sketch uses the Canbus library.
It requires the CAN-bus shield for the Arduino. This shield contains the MCP2515 CAN controller and the MCP2551 CAN-bus driver.
A connector for an EM406 GPS receiver and an uSDcard holder with 3v level convertor for use in data logging applications.
The output data can be displayed on a serial LCD.

SK Pang Electronics www.skpang.co.uk

v1.0 28-03-10

*/

#include "WProgram.h"
#include <Canbus.h>
#include <NewSoftSerial.h>
#include <watch.h>

int LED2 = 8;
int LED3 = 7;

/* Serial LCD is connected on pin 14 (Analog input 0) */
#define COMMAND 0xFE
#define CLEAR   0x01
NewSoftSerial sLCD =  NewSoftSerial(3, 6); 

void clear_lcd(void)
{
  sLCD.print(COMMAND,BYTE);
  sLCD.print(CLEAR,BYTE);
}
void set_cursor(int xpos, int ypos){  
  uint8_t row_start[] = {0, 64, 20, 84};
  sLCD.print(254, BYTE);
  sLCD.print(row_start[ypos] + xpos + 128, BYTE);               
} 

void setup() {
  
  pinMode(LED2, OUTPUT); 
  pinMode(LED3, OUTPUT); 

  Serial.begin(115200);
  Serial.println("ECU Reader");

  sLCD.begin(9600);
  
  if(Canbus.init(CANSPEED_500))
  {
    Serial.println("CAN Init ok");
    clear_lcd();
    sLCD.print("CAN Init ok");
  } else
  {
    Serial.println("Can't init CAN");
    sLCD.print("CAN init FAILED");
  } 

  Canbus.set_gauge_filter();
  // Canbus.set_loopback_mode();
 
  delay(1000);
  clear_lcd();
}



int toggle = 0;
void simulate(void) {
  if (toggle % 3 == 0) {
    uint8_t data[8] = { 0x47, 0x10, 0x47, 0x10, 0x47, 0x10, 0x47, 0x10 };
    Canbus.message_tx(MESSAGE_WHEEL_SPEED, data);
  }
  else if (toggle % 3 == 1) {
    uint8_t data[8] = { 0x20, 0x00, 0x9, 0x00, 0x30, 0xf1, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_TEMPS, data);
  }
  else if (toggle % 3 == 2) {
    uint8_t data[8] = { 0x20, 0x00, 0x9, 0x00, 0x30, 0xf1, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_FLUID_LEVELS, data);
  }
  toggle++;
  delay(1);
}
 
int counter = 0;
void loop() {
  
  // simulate();

	if (mcp2515_check_message()) {
    tCAN message;
    if (Canbus.full_message_rx(&message)) {

      Serial.println("Got a message");
    
      float lf_speed;
      float rf_speed;
      float lr_speed;
      float rr_speed;
      char wheel_speed_message[20];
      float coolant_temp;
      float ambient_temp;
      char temps_message[9];
      char fluid_levels_message[20];

      switch (message.id) {
        case MESSAGE_WHEEL_SPEED:
          lf_speed = ((((message.data[0] << 0x08) + message.data[1]) - 10000) / 100.0) * 0.62137;
          rf_speed = ((((message.data[2] << 0x08) + message.data[3]) - 10000) / 100.0) * 0.62137;
          lr_speed = ((((message.data[4] << 0x08) + message.data[5]) - 10000) / 100.0) * 0.62137;
          rr_speed = ((((message.data[6] << 0x08) + message.data[7]) - 10000) / 100.0) * 0.62137;
          sprintf(wheel_speed_message, "%2d %2d %2d %2d", (int)lf_speed, (int)rf_speed, (int)lr_speed, (int)rr_speed);
          set_cursor(0,3);
          sLCD.print(wheel_speed_message);
          break;

        case MESSAGE_TEMPS:
          coolant_temp = (message.data[0] - 40 + 32) * (9/5);
          ambient_temp = (message.data[4] - 40 + 32) * (9/5);
          sprintf(temps_message, "%3d,%2d", (int)coolant_temp, (int)ambient_temp);
          set_cursor(14,3);
          sLCD.print(temps_message);
          break;

        case MESSAGE_FLUID_LEVELS:
          sprintf(fluid_levels_message, "Fl: 0x%x 0x%x", message.data[0], message.data[1]);
          set_cursor(0,2);
          sLCD.print(fluid_levels_message);
          break;
      }

    }
  }

}
