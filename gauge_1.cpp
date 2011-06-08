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
#include "defaults.h"

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

tCAN last_wheel_speed_message;
tCAN last_fluids_message;
tCAN last_temps_message;
tCAN last_engine_message;
tCAN last_brakes_message;
void storeMessage(void) {
  tCAN message;
  if (Canbus.full_message_rx(&message)) {
    switch (message.id) {
      case MESSAGE_WHEEL_SPEED:
        last_wheel_speed_message = message;
        break;
      case MESSAGE_FLUID_LEVELS:
        last_fluids_message = message;
        break;
      case MESSAGE_TEMPS:
        last_temps_message = message;
        break;
      case MESSAGE_ENGINE:
        last_engine_message = message;
        break;
      case MESSAGE_BRAKES:
        last_brakes_message = message;
        break;
    }
  }
	SET(MCP2515_INT);
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
    Canbus.set_gauge_filter();
  } else
  {
    Serial.println("Can't init CAN");
    sLCD.print("CAN init FAILED");
  } 

  attachInterrupt(0, storeMessage, LOW);

  // Canbus.set_loopback_mode();
 
  delay(1000);
  clear_lcd();
}



volatile int toggle = 0;
void simulate(void) {
  if (toggle % 5 == 0) {
    uint8_t data[8] = { 0x47, 0x10, 0x47, 0x10, 0x47, 0x10, 0x47, 0x10 };
    Canbus.message_tx(MESSAGE_WHEEL_SPEED, data);
  }
  else if (toggle % 5 == 1) {
    uint8_t data[8] = { 0x20, 0x00, 0x9, 0x00, 0x30, 0xf1, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_TEMPS, data);
  }
  else if (toggle % 5 == 2) {
    uint8_t data[8] = { 0x20, 0x00, 0x9, 0x00, 0x30, 0xf1, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_FLUID_LEVELS, data);
  }
  else if (toggle % 5 == 3) {
    uint8_t data[8] = { 0x10, 0x40, 0x9, 0x00, 0x27, 0x10, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_ENGINE, data);
  }
  else if (toggle % 5 == 4) {
    uint8_t data[8] = { 0x10, 0x75, 0x30, 0x00, 0x27, 0x10, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_BRAKES, data);
  }
  toggle++;
  delay(1);
}
 
int counter = 0;
void loop() {
  
  // simulate();

  if (last_wheel_speed_message.id > 0) {
    float lf_speed = ((((last_wheel_speed_message.data[0] << 0x08) + last_wheel_speed_message.data[1]) - 10000) / 100.0) * 0.62137;
    float rf_speed = ((((last_wheel_speed_message.data[2] << 0x08) + last_wheel_speed_message.data[3]) - 10000) / 100.0) * 0.62137;
    float lr_speed = ((((last_wheel_speed_message.data[4] << 0x08) + last_wheel_speed_message.data[5]) - 10000) / 100.0) * 0.62137;
    float rr_speed = ((((last_wheel_speed_message.data[6] << 0x08) + last_wheel_speed_message.data[7]) - 10000) / 100.0) * 0.62137;
    char wheel_speed_message[20];
    sprintf(wheel_speed_message, "%2d %2d %2d %2d", (int)lf_speed, (int)rf_speed, (int)lr_speed, (int)rr_speed);
    set_cursor(0,3);
    sLCD.print(wheel_speed_message);
  }

  if (last_temps_message.id > 0) {
    float coolant_temp = (last_temps_message.data[0] - 40 + 32) * (9/5);
    float ambient_temp = (last_temps_message.data[4] - 40 + 32) * (9/5);
    char temps_message[9];
    sprintf(temps_message, "%3d,%2d", (int)coolant_temp, (int)ambient_temp);
    set_cursor(14,3);
    sLCD.print(temps_message);
  }

  if (last_fluids_message.id > 0) {
    char fluid_levels_message[20];
    sprintf(fluid_levels_message, "Fluids: 0x%02x 0x%02x", last_fluids_message.data[0], last_fluids_message.data[1]);
    set_cursor(0,2);
    sLCD.print(fluid_levels_message);
  }

  if (last_brakes_message.id > 0) {
    char brakes_message[10];
    float brake_application = (((last_brakes_message.data[0] << 0x08) + last_brakes_message.data[1]) - 30000)/1500;
    sprintf(brakes_message, "%3d%% brk", (int) brake_application);
    set_cursor(0,1);
    sLCD.print(brakes_message);
  }

  if (last_engine_message.id > 0) {
    char rpm_message[10];
    char mph_message[10];
    float mph = (((last_engine_message.data[4] << 0x08) + last_engine_message.data[5])/100.0) * 0.62137;
    char throttle_message[10];
    sprintf(rpm_message, "%4d rpm", (int) (last_engine_message.data[0] << 0x08) + last_engine_message.data[1]);
    sprintf(mph_message, "%3d mph", (int) mph);
    sprintf(throttle_message, "%3d%% thr", (int)last_engine_message.data[6]/2);
    set_cursor(12,0);
    sLCD.print(rpm_message);
    set_cursor(13,1);
    sLCD.print(mph_message);
    set_cursor(0,0);
    sLCD.print(throttle_message);
  }
}
