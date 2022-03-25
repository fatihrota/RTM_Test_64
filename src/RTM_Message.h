

#include <cstdint>
#include <cstring>
#include "json.hpp"

typedef struct
{
	nlohmann::json test;
	uint8_t startByte;
	uint8_t nrtmID;
	uint8_t rtmID;
	uint8_t testID;
	uint8_t ccID;
	uint8_t msgID;
	uint8_t isACK;
	uint8_t reserved;
	uint16_t timestamp;
	uint16_t cmd;
	uint16_t dataLen;
	uint16_t reserved_2;
} msg_header __attribute__((packed));

typedef struct
{
	uint16_t crc;
	uint8_t endByte;
	uint8_t reserved;
}msg_footer;

#define NRTM_MESSAGE_TOTAL_SIZE		1500
#define NRTM_MSG_SIZE				NRTM_MESSAGE_TOTAL_SIZE - sizeof(msg_header) - sizeof(msg_footer)

typedef struct
{
	msg_header header;
	uint8_t msg[NRTM_MSG_SIZE];
	msg_footer footer;
} nrtm_msg __attribute__((packed));
