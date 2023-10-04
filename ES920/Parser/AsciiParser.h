#pragma once
#ifndef ARDUINO_ES920_ASCII_PARSER_H
#define ARDUINO_ES920_ASCII_PARSER_H

#include "../Constants.h"
#include <ArxContainer.h>

namespace arduino {
namespace es920 {

#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L  // Have libstdc++11
    using Buffer = std::deque<StringType>;
#else
    using Buffer = arx::deque<StringType, 4>;
#endif
    // typedef void (*AsciiCallbackType)(const StringType& str);
    using AsciiCallbackType = std::function<void(const StringType& str)>;

    template <uint8_t PAYLOAD_SIZE>
    class AsciiParser {
        const StringType line_ok {"OK"};
        const StringType line_ng {"NG "};
        const StringType line_ver {"VER"};
        const StringType line_config {"Select Mode [1.terminal or 2.processor]"};
        const StringType line_operation {" ----- operation mode is ready ----- "};
        const uint8_t reset_char_9600[1] {};
        const uint8_t reset_char_19200[2] {0xFF};
        const uint8_t reset_char_38400[4] {0xFF, 0xFF, 0xFF};
        const uint8_t reset_char_57600[4] {0xFF, 0xFF, 0xFF};
        const uint8_t reset_char_115200[4] {0xFC, 0xFC, 0xFC};
        const uint8_t reset_char_230400[4] {0xE0, 0xE0, 0xE0};
        const uint8_t ok_bin_char[4] {2, 'O', 'K'};
        const uint8_t ng_bin_char[5] {6, 'N', 'G', ' '};
        const StringType line_reset_9600 {(const char*)reset_char_9600};
        const StringType line_reset_19200 {(const char*)reset_char_19200};
        const StringType line_reset_38400 {(const char*)reset_char_38400};
        const StringType line_reset_57600 {(const char*)reset_char_57600};
        const StringType line_reset_115200 {(const char*)reset_char_115200};
        const StringType line_reset_230400 {(const char*)reset_char_230400};
        const StringType line_ok_bin {(const char*)ok_bin_char};
        const StringType line_ng_bin {(const char*)ng_bin_char};
        StringType line_reset {""};

        // reply type
        bool b_reply {false};
        bool b_error {false};
        bool b_version {false};
        bool b_wakeup {false};
        bool b_reset {false};

        size_t error_count {0};
        StringType error_code {"000"};
        StringType version_str {""};
        int16_t remote_rssi;
        StringType remote_panid;
        StringType remote_ownid;
        StringType remote_hopid;  // ES920 only
        Mode mode;

        Buffer payloads;
        StringType buffer;
        AsciiCallbackType asc_callback;

    public:
        void feed(const uint8_t* data, const uint8_t size, const bool b_rssi, const bool b_rcvid, const bool b_exec_cb = true) {
            for (uint8_t i = 0; i < size; ++i) feed(data[i], b_rssi, b_rcvid, b_exec_cb);
        }

        void feed(const char c, const bool b_rssi, const bool b_rcvid, const bool b_exec_cb = true) {
            if (c == '\n') {
                if (buffer[ES920_STRING_SIZE(buffer) - 1] != '\r') {
                    LOG_ERROR("packet format is wrong, reset buffer : ", buffer);
                } else {
                    ES920_STRING_POP_BACK(buffer);  // remove '\r'
                    parseReply(buffer, b_rssi, b_rcvid);

                    while (available() && b_exec_cb) {
                        callback();
                        pop();
                    }
                }
                ES920_STRING_CLEAR(buffer);
            } else
                buffer += c;
        }

        bool isParsing() const { return ES920_STRING_SIZE(buffer) != 0; }
        size_t available() const { return payloads.size(); }
        const StringType& data() const {
            static StringType s;
            return payloads.empty() ? s : payloads.front();
        }
        size_t size() const { return payloads.empty() ? 0 : ES920_STRING_SIZE(payloads.front()); }

        void pop() {
            if (!payloads.empty()) payloads.pop_front();
        }

        void subscribe(const AsciiCallbackType& cb) { asc_callback = cb; }
        void callback() {
            if (asc_callback && available()) asc_callback(data());
        }

        void clear() {
            b_reply = b_error = b_version = b_wakeup = b_reset = false;
            payloads.clear();
            ES920_STRING_CLEAR(buffer);
            ES920_STRING_CLEAR(error_code);
            ES920_STRING_CLEAR(version_str);
            error_count = 0;
        }

        bool hasReply() { return disableAndReturn(b_reply); }
        bool hasError() { return disableAndReturn(b_error); }
        bool hasVersion() { return disableAndReturn(b_version); }
        bool hasWakeup() { return disableAndReturn(b_wakeup); }
        bool hasReset() { return disableAndReturn(b_reset); }

        int16_t remoteRssi() const { return remote_rssi; }
        const StringType& remotePanid() const { return remote_panid; }
        const StringType& remoteOwnid() const { return remote_ownid; }
        const StringType& remoteHopid() const { return remote_hopid; }

        Mode detectedMode() const { return mode; }
        const StringType& errorCode() const { return error_code; }
        size_t errorCount() const { return error_count; }
        const StringType& versionCode() const { return version_str; }

        void setBaudrate(const Baudrate b) {
            if (b == Baudrate::BD_9600)
                line_reset = line_reset_9600;
            else if (b == Baudrate::BD_19200)
                line_reset = line_reset_19200;
            else if (b == Baudrate::BD_38400)
                line_reset = line_reset_38400;
            else if (b == Baudrate::BD_57600)
                line_reset = line_reset_57600;
            else if (b == Baudrate::BD_115200)
                line_reset = line_reset_115200;
            else if (b == Baudrate::BD_230400)
                line_reset = line_reset_230400;
        }

    private:
        void parseReply(const StringType& str, const bool b_rssi, const bool b_rcvid) {
            const size_t str_size = ES920_STRING_SIZE(str);
            if (str == line_ok) {
                b_reply = true;
                b_error = false;
                error_code = "000";
                LOG_INFO("received OK");
            } else if (
                (str_size == 6) &&
                (ES920_STRING_SUBSTR(str, 0, 3) == line_ng)) {
                b_reply = true;
                b_error = true;
                error_code = ES920_STRING_SUBSTR(str, 3, 3);
                error_count++;
                LOG_ERROR("received error :", error_code, ", error count =", error_count);
            } else if (
                (str_size == 8) &&
                (ES920_STRING_SUBSTR(str, 0, 3) == line_ver)) {
                b_reply = true;
                b_version = true;
                version_str = ES920_STRING_SUBSTR(str, 3, 4);
                LOG_INFO("version message is detected!!! ver =", version_str);
            } else if (str == line_config) {
                b_wakeup = true;
                mode = Mode::CONFIG;
                LOG_INFO("wakeup message (config) is detected!!! mode =", (int)mode);
            } else if (str == line_operation) {
                b_wakeup = true;
                mode = Mode::OPERATION;
                LOG_INFO("wakeup message (operation) is detected!!! mode =", (int)mode);
            } else if (
                (str_size >= 3) &&
                ((ES920_STRING_SUBSTR(str, 0, 3) == line_reset) ||
                (ES920_STRING_SUBSTR(str, str_size - 3, str_size) == line_reset))) {
                b_reset = true;
                b_wakeup = false;
                LOG_INFO("reset message is detected!!!");
            } else {
                if (PAYLOAD_SIZE == PAYLOAD_SIZE_ES920)
                    parsePayloadES920(str, b_rssi, b_rcvid);
                else
                    parsePayloadES920LR(str, b_rssi, b_rcvid);
            }
        }

        void parsePayloadES920(const StringType& str, const bool b_rssi, const bool b_rcvid) {
            if (b_rssi || b_rcvid) {
                uint8_t data_head = 0;
                if (b_rssi) {
                    remote_rssi = -(strtol(ES920_STRING_SUBSTR(buffer, data_head, 2).c_str(), 0, 16)) / 2;
                    data_head += 2;
                }
                if (b_rcvid) {
                    remote_panid = ES920_STRING_SUBSTR(str, data_head + 0, 4);
                    remote_hopid = ES920_STRING_SUBSTR(str, data_head + 4, 4);
                    remote_ownid = ES920_STRING_SUBSTR(str, data_head + 8, 4);
                    data_head += 12;
                    LOG_INFO("got remote panid :", remote_panid);
                    LOG_INFO("got remote hopid :", remote_hopid);
                    LOG_INFO("got remote ownid :", remote_ownid);
                }
                payloads.push_back(ES920_STRING_SUBSTR(str, data_head, ES920_STRING_SIZE(str) - data_head));
            } else {
                payloads.push_back(str);
            }
        }

        void parsePayloadES920LR(const StringType& str, const bool b_rssi, const bool b_rcvid) {
            if (b_rssi || b_rcvid) {
                uint8_t data_head = 0;
                if (b_rssi) {
                    remote_rssi = ES920_STRING_TO_INT(ES920_STRING_SUBSTR(str, data_head, 4));
                    data_head += 4;
                    LOG_INFO("got rssi :", remote_rssi);
                }
                if (b_rcvid) {
                    remote_panid = ES920_STRING_SUBSTR(str, data_head, 4);
                    remote_ownid = ES920_STRING_SUBSTR(str, data_head + 4, 4);
                    data_head += 8;
                    LOG_INFO("got remote panid :", remote_panid);
                    LOG_INFO("got remote ownid :", remote_ownid);
                }
                payloads.push_back(ES920_STRING_SUBSTR(str, data_head, ES920_STRING_SIZE(str) - data_head));
            } else {
                payloads.push_back(str);
            }
        }
    };

}  // namespace es920
}  // namespace arduino

#endif  // ARDUINO_ES920_ASCII_PARSER_H
