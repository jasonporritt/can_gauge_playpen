#include "Arduino.h"
#include "SoftwareSerial.h"
#include <watch.h>
#include "defaults.h"
#include <Canbus.h>

/* Serial LCD is connected on pin 14 (Analog input 0) */
#define lcdRxPin 3
#define lcdTxPin 6
#define COMMAND 0xFE
#define CLEAR   0x01

// Filter set based on watch.h indicated message
const prog_uint8_t gauge_filter[] PROGMEM = 
{
	// Group 0
	MCP2515_FILTER(MESSAGE_WHEEL_SPEED),				// Filter 0
	MCP2515_FILTER(MESSAGE_STEERING),      				// Filter 1
	
	// Group 1
	MCP2515_FILTER(MESSAGE_ENGINE_PARAMS),		// Filter 2
	MCP2515_FILTER(MESSAGE_ENGINE),		// Filter 3
	MCP2515_FILTER(MESSAGE_BRAKES),		// Filter 4
	MCP2515_FILTER(MESSAGE_DYNAMICS),		// Filter 5
	
	MCP2515_FILTER(0x0fff),				// Mask 0 (for group 0)
	MCP2515_FILTER(0x0fff),		// Mask 1 (for group 1)
};

int LED2 = 8;
int LED3 = 7;

SoftwareSerial sLCD(lcdRxPin, lcdTxPin);

void clear_lcd(void)
{
  sLCD.write(COMMAND);
  sLCD.write(CLEAR);
}
void set_cursor(int xpos, int ypos){  
  uint8_t row_start[] = {0, 64, 20, 84};
  sLCD.write(254);
  sLCD.write(row_start[ypos] + xpos + 128);               
} 

tCAN last_wheel_speed_message;
tCAN last_fluids_message;
tCAN last_temps_message;
tCAN last_engine_message;
tCAN last_brakes_message;
tCAN last_engine_params_message;
tCAN last_steering_message;
tCAN last_dynamics_message;
void storeMessage(void) {
  tCAN message;
  if (Canbus.message_rx(&message)) {
    switch (message.id) {
      case MESSAGE_ENGINE_PARAMS:
        last_engine_params_message = message;
        break;
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
      case MESSAGE_STEERING:
        last_steering_message = message;
        break;
      case MESSAGE_DYNAMICS:
        last_dynamics_message = message;
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
    Canbus.set_filters(gauge_filter);
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
  if (toggle % 7 == 0) {
    uint8_t data[8] = { 0x47, 0x10, 0x47, 0x10, 0x47, 0x10, 0x47, 0x10 };
    Canbus.message_tx(MESSAGE_WHEEL_SPEED, data);
  }
  else if (toggle % 7 == 1) {
    uint8_t data[8] = { 0x20, 0x00, 0x9, 0x00, 0x30, 0xf1, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_TEMPS, data);
  }
  else if (toggle % 7 == 2) {
    uint8_t data[8] = { 0x20, 0x00, 0x9, 0x00, 0x30, 0xf1, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_FLUID_LEVELS, data);
  }
  else if (toggle % 7 == 3) {
    uint8_t data[8] = { 0x10, 0x40, 0x9, 0x00, 0x27, 0x10, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_ENGINE, data);
  }
  else if (toggle % 7 == 4) {
    uint8_t data[8] = { 0x78, 0x30, 0x30, 0x00, 0x27, 0x10, 0x1, 0x2 };
    Canbus.message_tx(MESSAGE_BRAKES, data);
  }
  else if (toggle % 7 == 5) {
    uint8_t data[8] = { 0x27, 0x01, 0x27, 0x22, 0x26, 0xef, 0x80, 0x72 };
    Canbus.message_tx(MESSAGE_ENGINE_PARAMS, data);
  }
  else if (toggle % 7 == 6) {
    uint8_t data[8] = { 0x29, 0x30, 0x27, 0x22, 0x26, 0xef, 0x80, 0x72 };
    Canbus.message_tx(MESSAGE_STEERING, data);
  }
  toggle++;
  delay(1);
}
 
int counter = 0;
void loop() {
  
  // simulate();
  
  // if (last_engine_params_message.id > 0) {
  //   float map  = ((((last_engine_params_message.data[0] << 0x8) + last_engine_params_message.data[1]) - 10000) * .0525) - 7.25;
  //   char engine_params_message[16];
  //   sprintf(engine_params_message, "MAP: %+ 2d psi", (int)map);
  //   set_cursor(0,2);
  //   sLCD.print(engine_params_message);
  // }

  if (last_dynamics_message.id > 0) {
    int yaw_rate = ((last_dynamics_message.data[4] << 0x8) + last_dynamics_message.data[5]) / 100;
    char yaw_message[10];
    sprintf(yaw_message, "Yaw: %03d", yaw_rate);
    set_cursor(0,2);
    sLCD.print(yaw_message);
  }

  if (last_steering_message.id > 0) {
    int steering_angle = (((last_steering_message.data[0] << 0x8) + last_steering_message.data[1]) - 10000) / 10;
    char steering_message[12];
    sprintf(steering_message, "St: %+3d", steering_angle);
    set_cursor(0,3);
    sLCD.print(steering_message);
    sLCD.print((char)223);
  }

  if (last_wheel_speed_message.id > 0) {
    float lf_speed = ((((last_wheel_speed_message.data[0] << 0x08) + last_wheel_speed_message.data[1]) - 10000) / 100.0) * 0.62137;
    float rf_speed = ((((last_wheel_speed_message.data[2] << 0x08) + last_wheel_speed_message.data[3]) - 10000) / 100.0) * 0.62137;
    float lr_speed = ((((last_wheel_speed_message.data[4] << 0x08) + last_wheel_speed_message.data[5]) - 10000) / 100.0) * 0.62137;
    float rr_speed = ((((last_wheel_speed_message.data[6] << 0x08) + last_wheel_speed_message.data[7]) - 10000) / 100.0) * 0.62137;
    char wheel_speed_message[20];
    sprintf(wheel_speed_message, "%2d %2d %2d %2d", (int)lf_speed, (int)rf_speed, (int)lr_speed, (int)rr_speed);
    set_cursor(0,0);
    sLCD.print(wheel_speed_message);
  }

  // if (last_fluids_message.id > 0) {
  //   char fluid_levels_message[20];
  //   sprintf(fluid_levels_message, "Fluids: 0x%02x 0x%02x", last_fluids_message.data[0], last_fluids_message.data[1]);
  //   set_cursor(0,2);
  //   sLCD.print(fluid_levels_message);
  // }

  // if (last_temps_message.id > 0) {
  //   float ambient_temp = (last_temps_message.data[0] - 40 + 32) * (9/5);
  //   char temps_message[9];
  //   sprintf(temps_message, "%3dF", (int)ambient_temp);
  //   set_cursor(17,3);
  //   sLCD.print(temps_message);
  // }

  if (last_brakes_message.id > 0) {
    char brakes_message[10];
    float brake_application = (((last_brakes_message.data[0] << 0x08) + last_brakes_message.data[1]) - 30000)/100;
    sprintf(brakes_message, "%3d%% brk", (int) brake_application);
    set_cursor(0,1);
    sLCD.print(brakes_message);
  }

  // if (last_engine_message.id > 0) {
  //   char rpm_message[10];
  //   char mph_message[10];
  //   float mph = (((last_engine_message.data[4] << 0x08) + last_engine_message.data[5])/100.0) * 0.62137;
  //   char throttle_message[10];
  //   sprintf(rpm_message, "%4d rpm", (int) (last_engine_message.data[0] << 0x08) + last_engine_message.data[1]);
  //   sprintf(mph_message, "%3d mph", (int) mph);
  //   sprintf(throttle_message, "%3d%% thr", (int)last_engine_message.data[6]/2);
  //   set_cursor(12,0);
  //   sLCD.print(rpm_message);
  //   set_cursor(13,1);
  //   sLCD.print(mph_message);
  //   set_cursor(0,0);
  //   sLCD.print(throttle_message);
  // }
}
