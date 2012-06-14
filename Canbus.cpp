/**
 * 
 *
 * Copyright (c) 2008-2009  All rights reserved.
 */
#include "WProgram.h"
#include "WConstants.h"
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pins_arduino.h"
#include <inttypes.h>
#include "global.h"
#include "mcp2515.h"
#include "defaults.h"
#include "Canbus.h"
#include <watch.h>

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
	MCP2515_FILTER(MESSAGE_LOAD),		// Filter 5
	
	MCP2515_FILTER(0x0fff),				// Mask 0 (for group 0)
	MCP2515_FILTER(0x0fff),		// Mask 1 (for group 1)
};


// Default filter -- all open
const prog_uint8_t all_open_filters[] PROGMEM = 
{
	// Group 0
	MCP2515_FILTER(0),				// Filter 0
	MCP2515_FILTER(0),				// Filter 1
	
	// Group 1
	MCP2515_FILTER_EXTENDED(0),		// Filter 2
	MCP2515_FILTER_EXTENDED(0),		// Filter 3
	MCP2515_FILTER_EXTENDED(0),		// Filter 4
	MCP2515_FILTER_EXTENDED(0),		// Filter 5
	
	MCP2515_FILTER(0),				// Mask 0 (for group 0)
	MCP2515_FILTER_EXTENDED(0),		// Mask 1 (for group 1)
};

/* C++ wrapper */
CanbusClass::CanbusClass() {
}

//
// Set loopback mode on the CAN bus controller
//
void CanbusClass::set_loopback_mode(void) {
  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0x40);
}

void CanbusClass::set_configuration_mode(void) {
	mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), (1<<REQOP2));
	while ((mcp2515_read_register(CANSTAT) & 0xe0) != (1<<REQOP2))
		;
}

//
// Set standard mode mode on the CAN bus controller
// Read + Write
//
void CanbusClass::set_standard_mode(void) {
  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
}

void CanbusClass::turn_on_filters(void) {
	mcp2515_write_register(RXB0CTRL, (1<<BUKT));
	mcp2515_write_register(RXB1CTRL, 0);
}

void CanbusClass::set_all_filters_open(void) {
  mcp2515_static_filter(all_open_filters);
}

void CanbusClass::set_gauge_filter(void) {
  mcp2515_static_filter(gauge_filter);
}

//
// Get the full message, including id and data
//
char CanbusClass::full_message_rx(tCAN *message) {
	if (mcp2515_read_status(SPI_RX_STATUS)) {
    uint8_t result = mcp2515_get_message(message);
    if (result) {
      return 1;
    }
  }
  return 0;
}

char CanbusClass::message_rx(unsigned char *buffer) {
  tCAN message;

  if (mcp2515_check_message()) {
    if (mcp2515_get_message(&message)) {
      buffer[0] = message.data[0];
      buffer[1] = message.data[1];
      buffer[2] = message.data[2];
      buffer[3] = message.data[3];
      buffer[4] = message.data[4];
      buffer[5] = message.data[5];
      buffer[6] = message.data[6];
      buffer[7] = message.data[7];								
      return 1;
    }
  }
  return 0;

}

char CanbusClass::message_tx(uint16_t id, uint8_t data[]) {
	tCAN message;

  // May be a few defaults here that should be parameterized?
	message.id = id;
	message.header.rtr = 0;
	message.header.length = 8;
	message.data[0] = data[0];
	message.data[1] = data[1];
	message.data[2] = data[2];
	message.data[3] = data[3];
	message.data[4] = data[4];
	message.data[5] = data[5];
	message.data[6] = data[6];
	message.data[7] = data[7];
	
	if (mcp2515_send_message(&message)) {
		return 1;
	}

	return 0;
}

char CanbusClass::ecu_req(unsigned char pid,  char *buffer) 
{
	tCAN message;
	float engine_data;
	int timeout = 0;
	char message_ok = 0;
	// Prepair message
	message.id = PID_REQUEST;
	message.header.rtr = 0;
	message.header.length = 8;
	message.data[0] = 0x02;
	message.data[1] = 0x01;
	message.data[2] = pid;
	message.data[3] = 0x00;
	message.data[4] = 0x00;
	message.data[5] = 0x00;
	message.data[6] = 0x00;
	message.data[7] = 0x00;						
	

	mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
//		SET(LED2_HIGH);	
	if (mcp2515_send_message(&message)) {
	}
	
	while(timeout < 4000)
	{
		timeout++;
				if (mcp2515_check_message()) 
				{

					if (mcp2515_get_message(&message)) 
					{
							if((message.id == PID_REPLY) && (message.data[2] == pid))	// Check message is the reply and its the right PID
							{
								switch(message.data[2])
								{   /* Details from http://en.wikipedia.org/wiki/OBD-II_PIDs */
									case ENGINE_RPM:  			//   ((A*256)+B)/4    [RPM]
									engine_data =  ((message.data[3]*256) + message.data[4])/4;
									sprintf(buffer,"%d rpm ",(int) engine_data);
									break;
							
									case ENGINE_COOLANT_TEMP: 	// 	A-40			  [degree C]
									engine_data =  message.data[3] - 40;
									sprintf(buffer,"%d degC",(int) engine_data);
									break;
							
									case VEHICLE_SPEED: 		// A				  [km]
									engine_data =  message.data[3];
									sprintf(buffer,"%d km ",(int) engine_data);
									break;

									case MAF_SENSOR:   			// ((256*A)+B) / 100  [g/s]
									engine_data =  ((message.data[3]*256) + message.data[4])/100;
									sprintf(buffer,"%d g/s",(int) engine_data);
									break;

									case O2_VOLTAGE:    		// A * 0.005   (B-128) * 100/128 (if B==0xFF, sensor is not used in trim calc)
									engine_data = message.data[3]*0.005;
									sprintf(buffer,"%d v",(int) engine_data);
							
									case THROTTLE:				// Throttle Position
									engine_data = (message.data[3]*100)/255;
									sprintf(buffer,"%d %% ",(int) engine_data);
									break;
							
								}
								message_ok = 1;
							}

					}
				}
				if(message_ok == 1) return 1;
	}


 	return 0;
}

char CanbusClass::init(unsigned char speed) {

  return mcp2515_init(speed);
 
}

CanbusClass Canbus;
