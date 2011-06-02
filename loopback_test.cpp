/* Welcome to the ECU Reader project. This sketch uses the Canbus library.
It requires the CAN-bus shield for the Arduino. This shield contains the MCP2515 CAN controller and the MCP2551 CAN-bus driver.
A connector for an EM406 GPS receiver and an uSDcard holder with 3v level convertor for use in data logging applications.
The output data can be displayed on a serial LCD.

SK Pang Electronics www.skpang.co.uk

v1.0 28-03-10

*/

#include "WProgram.h"
#include <Canbus.h>

#define COMMAND 0xFE
#define CLEAR   0x01
#define LINE0   0x80
#define LINE1   0xC0

int LED2 = 8;
int LED3 = 7;

void serial_print_can_message(tCAN message) {
  char output[50];
  sprintf(output, "id=%#x -- %#x %#x %#x %d %d %d %d %d", message.id, message.data[0], message.data[1], message.data[2], message.data[3], message.data[4], message.data[5], message.data[6], message.data[7]);
  Serial.println(output);
}
void serial_print_register(int reg) {
  char reg_result[40];
	sprintf(reg_result, "Register %i: %#x", reg, mcp2515_read_register(reg));
  Serial.println(reg_result);
}

void setup() {
  
  pinMode(LED2, OUTPUT); 
  pinMode(LED3, OUTPUT); 

  Serial.begin(115200);
  Serial.println("ECU Reader");  /* For debug use */
  
  if(Canbus.init(CANSPEED_500))  /* Initialise MCP2515 CAN controller at the specified speed */
  // if(Canbus.init(CANSPEED_125))  /* Initialise MCP2515 CAN controller at the specified speed */
  {
    Serial.println("CAN Init ok");
  } else
  {
    Serial.println("Can't init CAN");
  } 

  Canbus.set_all_filters_open();

  serial_print_register(RXM0);
  serial_print_register(RXM1);
  serial_print_register(RXB0CTRL);
  serial_print_register(RXB1CTRL);

  Canbus.set_loopback_mode();

  delay(100);
}


 
tCAN message;
void loop() {
  
  uint8_t data[8] = { 0x20, 0x00, 0x11, 0x19, 0xaf, 0x00, 0x00, 0xf0 };
  Canbus.message_tx(0x28f, data);

  delay(5);

	if (mcp2515_check_message()) {
    Serial.println("There are messages");
    if (Canbus.full_message_rx(&message)) {
      serial_print_can_message(message);
    }
  }
  else {
    Serial.println("check_messages() says there are no messages");
  }

  delay(1000);
}
