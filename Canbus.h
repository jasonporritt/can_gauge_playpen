/**
 * CAN BUS
 *
 * Copyright (c) 2010 Sukkin Pang All rights reserved.
 */
#include "mcp2515.h"

#ifndef canbus__h
#define canbus__h

#define CANSPEED_125 	  7		// CAN speed at 125 kbps
#define CANSPEED_250  	3		// CAN speed at 250 kbps
#define CANSPEED_500	  1		// CAN speed at 500 kbps

// 
// OBDII data definitions
#define ENGINE_COOLANT_TEMP 0x05
#define ENGINE_RPM          0x0C
#define VEHICLE_SPEED       0x0D
#define MAF_SENSOR          0x10
#define O2_VOLTAGE          0x14
#define THROTTLE			      0x11
#define MAP_SENSOR          0x0B
#define ENGINE_OIL_TEMP     0x5C            
#define FUEL_INJECTION_TIMING 0x5D

// OBDII request & response definitions
#define PID_REQUEST         0x7DF
#define PID_REPLY			      0x7E8

class CanbusClass
{
  public:

	CanbusClass();
  char init(unsigned char);
	char message_tx(uint16_t id, uint8_t data[]);
	char message_rx(unsigned char *buffer);
	char full_message_rx(tCAN *message);
	char ecu_req(unsigned char pid,  char *buffer);

  void set_loopback_mode(void);
  void set_standard_mode(void);
  void set_configuration_mode(void);

  void set_all_filters_open(void);
  void set_gauge_filter(void);
  void turn_on_filters(void);

  private:
	
};

extern CanbusClass Canbus;
// extern tCAN message;

#endif
