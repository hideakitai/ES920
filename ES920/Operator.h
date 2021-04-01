#pragma once
#ifndef ARDUINO_ES920_OPERATOR_H
#define ARDUINO_ES920_OPERATOR_H

#include "Constants.h"
#include "Utils.h"
#include "util/Packetizer/Packetizer.h"

namespace arduino {
namespace es920 {

    template <typename Stream, uint8_t PAYLOAD_SIZE>
    class Operator {
        Stream* stream;
        Packetizer::Encoder<Packetizer::encoding::COBS> packer;

    public:
        void attach(const Stream& s) {
            stream = (Stream*)&s;
        }

        bool sendPayload(const StringType& str) {
            if (ES920_STRING_SIZE(str) + 2 > PAYLOAD_SIZE)  // exclude "\r\n"
            {
                LOG_WARNING("too long data, must be <= ", PAYLOAD_SIZE - 2, ". size = ", ES920_STRING_SIZE(str));
                return false;
            } else {
                ES920_WRITE_BYTES(str.c_str(), ES920_STRING_SIZE(str));
                ES920_WRITE_BYTES("\r\n", 2);
                return true;
            }
        }

        bool sendPayload(const uint8_t* data, const uint8_t size, const uint8_t index) {
            if (size + 4 > PAYLOAD_SIZE)  // exclude header, index, size, footer
                LOG_WARNING("too long input data, must be <= ", PAYLOAD_SIZE - 4, ". size = ", size);
            else {
                packer.encode(index, data, size, true);
                if (packer.size() > PAYLOAD_SIZE)
                    LOG_WARNING("too long packetized data, must be <= ", PAYLOAD_SIZE, ". size = ", size);
                else {
                    ES920_WRITE_BYTE((uint8_t)packer.size());
                    ES920_WRITE_BYTES(packer.data(), packer.size());
                    return true;
                }
            }
            return false;
        }

        // TODO: extend to ES920 format
        bool sendFrame(const uint16_t pan, const uint16_t own, const StringType& str) {
            if (ES920_STRING_SIZE(str) + 2 > PAYLOAD_SIZE)  // exclude "\r\n"
                LOG_WARNING("too long data, must be <= ", PAYLOAD_SIZE - 2, ". size = ", ES920_STRING_SIZE(str));
            else {
                StringType header = arx::str::to_hex(pan) + arx::str::to_hex(own);
                ES920_WRITE_BYTES(header.c_str(), ES920_STRING_SIZE(header));
                ES920_WRITE_BYTES(str.c_str(), ES920_STRING_SIZE(str));
                ES920_WRITE_BYTES("\r\n", 2);
                return true;
            }
            return false;
        }

        // TODO: extend to ES920 format
        bool sendFrame(const uint16_t pan, const uint16_t own, const uint8_t* data, const uint8_t size, const uint8_t index) {
            if (size > PAYLOAD_SIZE)
                LOG_WARNING("too long data, must be <= ", PAYLOAD_SIZE, ". size = ", size);
            else {
                StringType header = arx::str::to_hex(pan) + arx::str::to_hex(own);
                uint8_t size_ext = (uint8_t)ES920_STRING_SIZE(header) + size;
                ES920_WRITE_BYTE(size_ext);
                ES920_WRITE_BYTES(header.c_str(), ES920_STRING_SIZE(header));
                packer.encode(index, data, size, true);
                if (packer.size() > PAYLOAD_SIZE)
                    LOG_WARNING("too long packetized data, must be <= ", PAYLOAD_SIZE, ". size = ", size);
                else {
                    ES920_WRITE_BYTES(packer.data(), packer.size());
                    return true;
                }
            }
            return false;
        }

        uint8_t size() const { return packer.size(); }
    };

}  // namespace es920
}  // namespace arduino

#endif  // ARDUINO_ES920_OPERATOR_H
