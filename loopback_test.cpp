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

#define COMMAND 0xFE
#define CLEAR   0x01
#define LINE0   0x80
#define LINE1   0xC0

int LED2 = 8;
int LED3 = 7;

/* Serial LCD is connected on pin 14 (Analog input 0) */
NewSoftSerial sLCD =  NewSoftSerial(3, 6); 
#define COMMAND 0xFE
#define CLEAR   0x01
#define LINE0   0x80
#define LINE1   0xC0


/* Define Joystick connection */
#define UP     A1
#define RIGHT  A2
#define DOWN   A3
#define CLICK  A4
#define LEFT   A5

/* LCD stuff */
#define     SET_CURSOR      0b.1000.0000    //SET_CURSOR + X : Sets cursor position to X

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



void clear_lcd(void)
{
  sLCD.print(COMMAND,BYTE);
  sLCD.print(CLEAR,BYTE);
}
void new_line(void) {
  sLCD.print(10,BYTE);
}
void cursor_set(int xpos, int ypos){  
  uint8_t row_start[] = {0, 64, 20, 84};
  sLCD.print(254, BYTE);
  sLCD.print(row_start[ypos] + xpos + 128, BYTE);               
  // sLCD.print(xpos);   //Column position   
  // sLCD.print(ypos); //Row position 
} 


void setup() {
  
  pinMode(LED2, OUTPUT); 
  pinMode(LED3, OUTPUT); 


  Serial.begin(115200);
  Serial.println("ECU Reader");  /* For debug use */

  sLCD.begin(9600);              /* Setup serial LCD and clear the screen */
  
  if(Canbus.init(CANSPEED_500))  /* Initialise MCP2515 CAN controller at the specified speed */
  // if(Canbus.init(CANSPEED_125))  /* Initialise MCP2515 CAN controller at the specified speed */
  {
    Serial.println("CAN Init ok");
    clear_lcd();
    sLCD.print("CAN Init ok");
  } else
  {
    Serial.println("Can't init CAN");
  } 

  // Canbus.set_all_filters_open();

  // serial_print_register(RXM0);
  // serial_print_register(RXM1);
  // serial_print_register(RXB0CTRL);
  // serial_print_register(RXB1CTRL);

  // Canbus.set_loopback_mode();

  delay(1000);

  clear_lcd();
}


#define MAZDA_CAN_500_RPM_SPEED 0x201
#define MAZDA_CAN_500_STEERING_ANGLE 0x4DA
 
tCAN message;
void loop() {
  
  // uint8_t data[8] = { 0x18, 0xf2, 0x11, 0x19, 0x31, 0x00, 0x00, 0xf0 };
  // Canbus.message_tx(0x201, data);

  // uint8_t data2[8] = { 0x18, 0xf2, 0x11, 0x19, 0x31, 0x00, 0x00, 0xf0 };
  // Canbus.message_tx(MAZDA_CAN_500_STEERING_ANGLE, data2);

  // delay(1);

	if (mcp2515_check_message()) {
    Serial.println("There are messages");
    if (Canbus.full_message_rx(&message)) {

      char rpm_message[10];
      int engine_rpms;
      char speed_message[10];
      float speed;
      char steering_angle_message[10];
      float angle;


      switch (message.id) {

        case MAZDA_CAN_500_RPM_SPEED:
        rpm_message[10];
        engine_rpms = (message.data[0] << 0x08) + message.data[1];
        sprintf(rpm_message, "%4d rpm", engine_rpms);
        cursor_set(0,0);
        sLCD.print(rpm_message);

        speed_message[10];
        speed = ((message.data[4] << 0x08) + message.data[5]) / 100.0;
        sprintf(speed_message, "%3d km/h", (int)speed);
        cursor_set(12,0);
        sLCD.print(speed_message);

        break;

        case MAZDA_CAN_500_STEERING_ANGLE:
        steering_angle_message[10];
        angle = (((message.data[0] << 0x08) + message.data[1]) - 32768) / 10;
        sprintf(steering_angle_message, "%d degrees", (int)angle);
        cursor_set(0,2);
        sLCD.print(steering_angle_message);
        break;

      }
    }
  }
  // else {
  //   Serial.println("check_messages() says there are no messages");
  // }

  // delay(1000);
}
