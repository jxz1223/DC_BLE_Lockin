#ifndef MULTI_NODE_PROTOCOL_H
#define MULTI_NODE_PROTOCOL_H

#include <stdint.h>

#define MULTI_PROTOCOL_MAGIC_0                 0xA5U
#define MULTI_PROTOCOL_MAGIC_1                 0x5AU
#define MULTI_PROTOCOL_VERSION                 0x02U
#define MULTI_PROTOCOL_HEADER_LEN              10U
#define MULTI_PROTOCOL_FRAME_OVERHEAD          (2U + MULTI_PROTOCOL_HEADER_LEN + 2U)
#define MULTI_PROTOCOL_MAX_FRAME               246U
#define MULTI_PROTOCOL_MAX_PAYLOAD             (MULTI_PROTOCOL_MAX_FRAME - MULTI_PROTOCOL_FRAME_OVERHEAD)

#define MULTI_PROTOCOL_NODE_RELAY              0x0000U
#define MULTI_PROTOCOL_NODE_BROADCAST          0xFFFFU

#define MULTI_PROTOCOL_FLAG_ACK_REQUIRED       0x01U
#define MULTI_PROTOCOL_FLAG_RETRANSMISSION     0x02U
#define MULTI_SET_DAC_FLAG_SAVE                0x04U

#define MULTI_CMD_SCAN_START                   0x10U
#define MULTI_CMD_SET_DAC                      0x11U
#define MULTI_CMD_ABORT                        0x12U
#define MULTI_CMD_STREAM_GRANT                 0x14U
#define MULTI_CMD_FRAME_ACK                    0x15U

#define MULTI_EVT_ACK                          0x80U
#define MULTI_EVT_NACK                         0x81U
#define MULTI_EVT_SCAN_BEGIN                   0x90U
#define MULTI_EVT_SCAN_POINTS                  0x91U
#define MULTI_EVT_SCAN_END                     0x92U
#define MULTI_EVT_SENSOR_READINGS              0x93U
#define MULTI_EVT_BLE_RAW                      0xA0U
#define MULTI_EVT_LINK_STATUS                  0xA1U
#define MULTI_EVT_RELAY_STATS                  0xA2U
#define MULTI_EVT_COMMAND_TRACE                0xA3U

#define MULTI_TRACE_UART_FRAME_VALID           0x00U
#define MULTI_TRACE_COMMAND_QUEUED             0x01U
#define MULTI_TRACE_BLE_WRITE_OK               0x02U
#define MULTI_TRACE_SENSOR_EVENT_RX            0x03U
#define MULTI_TRACE_SENSOR_ACK                 0x04U
#define MULTI_TRACE_SENSOR_NACK                0x05U
#define MULTI_TRACE_SENSOR_FRAME_INVALID       0x06U
#define MULTI_TRACE_COMMAND_RETRY              0x07U
#define MULTI_TRACE_COMMAND_TIMEOUT            0x08U
#define MULTI_TRACE_BLE_WRITE_ERROR            0x09U

#define MULTI_STATUS_OK                        0x00U
#define MULTI_STATUS_CRC_ERROR                 0x01U
#define MULTI_STATUS_BAD_VERSION               0x02U
#define MULTI_STATUS_BAD_LENGTH                0x03U
#define MULTI_STATUS_UNKNOWN_TYPE              0x04U
#define MULTI_STATUS_NO_LINK                   0x05U
#define MULTI_STATUS_BUSY                      0x06U
#define MULTI_STATUS_UNSUPPORTED               0x07U
#define MULTI_STATUS_RX_OVERFLOW               0x08U
#define MULTI_STATUS_TIMEOUT                   0x09U
#define MULTI_STATUS_QUEUE_FULL                0x0AU
#define MULTI_STATUS_NODE_NOT_FOUND            0x0BU
#define MULTI_STATUS_LINK_NOT_READY             0x0CU
#define MULTI_STATUS_TRANSPORT_ERROR            0x0DU
#define MULTI_STATUS_DEFERRED                   0xFFU

#define MULTI_FRAME_VERSION_OFFSET             2U
#define MULTI_FRAME_TYPE_OFFSET                3U
#define MULTI_FRAME_FLAGS_OFFSET               4U
#define MULTI_FRAME_NODE_ID_OFFSET             5U
#define MULTI_FRAME_SCAN_ID_OFFSET             7U
#define MULTI_FRAME_SEQ_OFFSET                 9U
#define MULTI_FRAME_PAYLOAD_LEN_OFFSET         11U
#define MULTI_FRAME_PAYLOAD_OFFSET             12U

static inline uint16_t Multi_ReadLe16(const uint8_t *pData)
{
  return (uint16_t)pData[0] | ((uint16_t)pData[1] << 8);
}

static inline void Multi_WriteLe16(uint8_t *pData, uint16_t value)
{
  pData[0] = (uint8_t)value;
  pData[1] = (uint8_t)(value >> 8);
}

#endif /* MULTI_NODE_PROTOCOL_H */
