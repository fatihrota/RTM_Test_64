

#include <cstdint>
#include <cstring>
#include "json.hpp"

typedef struct
{
	uint8_t startByte;
	uint8_t nrtmID;
	uint8_t rtmID;
	uint8_t testID;
	uint8_t ccID;
	uint8_t msgID;
	uint8_t isACK;
	uint8_t isFragment;
	uint8_t fragmentNo;
	uint16_t timestamp;
	uint16_t cmd;
	uint16_t dataLen;
} msg_header;

typedef struct
{
	uint16_t crc;
	uint8_t endByte;
	uint8_t reserved;
}msg_footer;

#define NRTM_MESSAGE_TOTAL_SIZE		10000
#define NRTM_MSG_SIZE				NRTM_MESSAGE_TOTAL_SIZE - sizeof(msg_header)/* - sizeof(msg_footer)*/

typedef struct
{
	msg_header header;
	uint8_t *msg;
	//msg_footer footer;
} nrtm_msg;
