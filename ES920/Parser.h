#pragma once
#ifndef ARDUINO_ES920_PARSER_H
#define ARDUINO_ES920_PARSER_H

#include "Constants.h"
#include "Parser/AsciiParser.h"
#include "Parser/BinaryParser.h"

namespace arduino {
namespace es920 {

    template <typename Stream, uint8_t PAYLOAD_SIZE>
    class Parser {
        Stream* stream;
        AsciiParser<PAYLOAD_SIZE> asc_parser;
        BinaryParser<PAYLOAD_SIZE> bin_parser;

    public:
        void attach(const Stream& s, const Baudrate b) {
            stream = (Stream*)&s;
            setBaudrate(b);
        }

        void subscribeAscii(const AsciiCallbackType& cb) {
            asc_parser.subscribe(cb);
        }

        void subscribeBinary(const uint8_t id, const BinaryCallbackType& cb) {
            bin_parser.subscribe(id, cb);
        }

        void subscribeBinary(const BinaryAlwaysCallbackType& cb) {
            bin_parser.subscribe(cb);
        }

        void clear() {
            asc_parser.clear();
            bin_parser.clear();
        }

        size_t parseAscii(const bool b_rssi, const bool b_rcvid, const bool b_exec_cb = true) {
            while (stream->available()) {
                uint8_t d = ES920_READ_BYTE();
                asc_parser.feed(d, b_rssi, b_rcvid, b_exec_cb);
            }
            return availableAscii();
        }

        size_t parseBinary(const bool b_rssi, const bool b_rcvid, const bool b_exec_cb = true) {
            while (stream->available()) {
                uint8_t d = ES920_READ_BYTE();
                bin_parser.feed(d, b_rssi, b_rcvid, b_exec_cb);
            }
            return availableBinary();
        }

        void callbackAscii() {
            asc_parser.callback();
        }

        void callbackBinary() {
            bin_parser.callback();
        }

        uint8_t indexAscii() const { return 0; }  // TODO:
        uint8_t indexBinary() const { return bin_parser.index(); }

        uint8_t indexBackAscii() const { return 0; }  // TODO:
        uint8_t indexBackBinary() const { return bin_parser.index_back(); }

        const StringType& dataAscii() const { return asc_parser.data(); }
        const uint8_t* dataBinary() const { return bin_parser.data(); }

        const StringType& dataBackAscii() const { return StringType(""); }  // TODO:
        const uint8_t* dataBackBinary() const { return bin_parser.data_back(); }

        size_t sizeAscii() const { return asc_parser.size(); }
        size_t sizeBinary() const { return bin_parser.size(); }

        size_t sizeBackAscii() const { return 0; }  // TODO:
        size_t sizeBackBinary() const { return bin_parser.size_back(); }

        size_t availableAscii() const { return asc_parser.available(); }
        size_t availableBinary() const { return bin_parser.available(); }

        void popBinary() { bin_parser.pop(); }
        void popAscii() { asc_parser.pop(); }

        void popBackBinary() { bin_parser.pop_back(); }
        void popBackAscii() {
            LOG_WARN("subghz pop_back in Ascii is not implemented");
            ;
        }  // TODO:

        bool hasReplyAscii() { return asc_parser.hasReply(); }
        bool hasReplyBinary() { return bin_parser.hasReply(); }
        bool hasErrorAscii() { return asc_parser.hasError(); }
        bool hasErrorBinary() { return bin_parser.hasError(); }

        bool hasVersion() { return asc_parser.hasVersion(); }
        bool hasWakeup() { return asc_parser.hasWakeup(); }
        bool hasReset() { return asc_parser.hasReset(); }

        const StringType& errorCodeAscii() const { return asc_parser.errorCode(); }
        size_t errorCountAscii() const { return asc_parser.errorCount(); }
        const StringType& errorCodeBinary() const { return bin_parser.errorCode(); }
        size_t errorCountBinary() const { return bin_parser.errorCount(); }

        Mode detectedMode() { return asc_parser.detectedMode(); }

        int16_t remoteRssiAscii() const { return asc_parser.remoteRssi(); }
        int16_t remoteRssiBinary() const { return bin_parser.remoteRssi(); }
        const StringType& remotePanidAscii() const { return asc_parser.remotePanid(); }
        const StringType& remotePanidBinary() const { return bin_parser.remotePanid(); }
        const StringType& remoteOwnidAscii() const { return asc_parser.remoteOwnid(); }
        const StringType& remoteOwnidBinary() const { return bin_parser.remoteOwnid(); }
        const StringType& remoteHopidAscii() const { return asc_parser.remoteHopid(); }
        const StringType& remoteHopidBinary() const { return bin_parser.remoteHopid(); }

        bool detectReset(const uint32_t timeout_ms) {
            waitResponseAscii(timeout_ms);
            return asc_parser.hasReset();
        }

        Mode detectMode(const uint32_t timeout_ms) {
            LOG_INFO("mode detection start: wait for mode detection...");
            waitResponseAscii(timeout_ms);

            if (asc_parser.hasWakeup()) {
                LOG_INFO("wakeup message has come!");
                LOG_INFO("detected mode is: configuration");
                return asc_parser.detectedMode();
            } else {
                LOG_INFO("no wakeup message");
                LOG_INFO("detected mode is: (maybe) operation mode");
                return Mode::OPERATION;
            }
        }

        bool detectReplyAscii(const uint32_t timeout_ms) {
            return waitResponseAscii(timeout_ms);
        }

        bool detectReplyBinary(const uint32_t timeout_ms) {
            return waitResponseBinary(timeout_ms);
        }

        const StringType& detectVersion(const uint32_t timeout_ms) {
            waitResponseAscii(timeout_ms);
            if (asc_parser.hasVersion())
                return asc_parser.versionCode();
            return asc_parser.errorCode();
        }

        void setBaudrate(const Baudrate b) {
            asc_parser.setBaudrate(b);
        }

    private:
        bool waitResponseAscii(const uint32_t timeout_ms) {
            uint32_t start_ms = ELAPSED_TIME_MS();
            while (ELAPSED_TIME_MS() < start_ms + timeout_ms) {
                parseAscii(false, false);
                if (hasReplyAscii()) return true;
            }
            LOG_INFO("no reply from ascii parser");
            return false;
        }

        bool waitResponseBinary(const uint32_t timeout_ms) {
            uint32_t start_ms = ELAPSED_TIME_MS();
            while (ELAPSED_TIME_MS() < start_ms + timeout_ms) {
                parseBinary(false, false);
                if (hasReplyBinary()) return true;
            }
            LOG_ERROR("no reply from binary parser");
            return false;
        }

        // const uint32_t lag_send_encode_us {600};
        // const uint32_t lag_send_wireless_us {3200};
        // const uint32_t lag_recv_decode_us {100};

        // float uart_us_per_byte {10.f / 115200.f * 1000000.f};
        // static constexpr float air_us_per_byte {1.f / (100.f / 8.f * 1024.f) * 1000000.f};
        // static constexpr float LAG_USEC_SEND_ENCODE {700.f}; // 600.f
        // static constexpr float LAG_USEC_SEND_SEND {3200.f}; // 3200.f
        // static constexpr float LAG_USEC_RECV_DECODE {200.f}; // 100.f

        // void setBaudrate(const Baudrate b)
        // {
        //     asc_parser.setBaudrate(b);
        //     float baudrate = 0;
        //     if      (b == Baudrate::BD_9600)   baudrate = 9600;
        //     else if (b == Baudrate::BD_19200)  baudrate = 19200;
        //     else if (b == Baudrate::BD_38400)  baudrate = 38400;
        //     else if (b == Baudrate::BD_57600)  baudrate = 57600;
        //     else if (b == Baudrate::BD_115200) baudrate = 115200;
        //     else if (b == Baudrate::BD_230400) baudrate = 230400;
        //     uart_us_per_byte = 10.f / baudrate * 1000000.f;
        // }

        // inline uint32_t lag(const uint32_t size) const
        // {
        //     // serial_us = byte_size * 10[bit] / baudrate * 1000000
        //     float lag_serial_send = (float)size * uart_us_per_byte;
        //     // air_us = (byte_size + 26) / (100 / 8 * 1024) * 1000000
        //     float lag_send_air = ((float)size + 26.f) * air_us_per_byte;
        //     float lag_recv_serial = lag_serial_send; // TODO: rssi, rcvid, etc.
        //     return uint32_t(lag_serial_send + LAG_USEC_SEND_ENCODE + LAG_USEC_SEND_SEND + lag_send_air + LAG_USEC_RECV_DECODE + lag_recv_serial);
        // }
    };

}  // namespace es920
}  // namespace arduino

#endif  // ARDUINO_ES920_PARSER_H
