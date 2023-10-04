#pragma once
#ifndef ARDUINO_ES920_BINARY_PARSER_H
#define ARDUINO_ES920_BINARY_PARSER_H

#include "../Constants.h"
#include <Packetizer.h>

namespace arduino {
namespace es920 {

    using BinaryCallbackType = Packetizer::CallbackType;
    using BinaryAlwaysCallbackType = Packetizer::CallbackAlwaysType;

    template <uint8_t PAYLOAD_SIZE>
    class BinaryParser {
        Packetizer::Decoder<Packetizer::encoding::COBS> unpacker;

        const uint8_t MAX_HEADER_SIZE {13};
        const uint8_t ok_bin_char[4] {2, 'O', 'K'};
        const StringType line_ok_bin {(const char*)ok_bin_char};
        const uint8_t ng_bin_char[5] {6, 'N', 'G', ' '};
        const StringType line_ng_bin {(const char*)ng_bin_char};

        bool b_reply {false};
        bool b_error {false};

        size_t error_count {0};
        StringType error_code {"000"};
        StringType version_str {""};
        int16_t remote_rssi;
        StringType remote_panid;
        StringType remote_ownid;
        StringType remote_hopid;  // ES920 only

        enum class State { SIZE,
                           VAGUE,
                           REPLY,
                           HEADER,
                           DATA };
        State state;
        StringType buffer;

    public:
        void feed(const uint8_t* data, const uint8_t size, const bool b_rssi, const bool b_rcvid, const bool b_exec_cb = true) {
            for (uint8_t i = 0; i < size; ++i) feed(data[i], b_rssi, b_rcvid, b_exec_cb);
        }

        void feed(const uint8_t d, const bool b_rssi, const bool b_rcvid, const bool b_exec_cb = true) {
            if (unpacker.parsing()) {
                unpacker.feed(&d, 1, b_exec_cb);
            } else {
                buffer += (char)d;

                if (state == State::SIZE) {
                    if (isFirstByteReply())
                        state = State::VAGUE;
                    else
                        state = State::HEADER;
                } else if (state == State::VAGUE) {
                    if (isSecondByteReply())
                        state = State::REPLY;
                    else
                        state = State::HEADER;
                }

                switch (state) {
                    case State::VAGUE: {
                        if (ES920_STRING_SIZE(buffer) != 1) {
                            LOG_ERROR("won't come here! curr buffer = ", buffer);
                            for (size_t i = 0; i < ES920_STRING_SIZE(buffer); ++i) LOG_ERROR((int)buffer[i]);

                            ES920_STRING_CLEAR(buffer);
                            state = State::SIZE;
                        }
                        break;
                    }
                    case State::REPLY: {
                        if (parseReply()) state = State::SIZE;
                        break;
                    }
                    case State::HEADER: {
                        if (parseHeader(b_rssi, b_rcvid)) {
                            // if buffer is not zero, feed to unpacker
                            if (ES920_STRING_SIZE(buffer) != 0) {
                                if (ES920_STRING_SIZE(buffer) != 1)
                                    LOG_ERROR("won't come here! unpexpected buffer rest size!!");

                                unpacker.feed((const uint8_t*)buffer.c_str(), ES920_STRING_SIZE(buffer), b_exec_cb);
                                ES920_STRING_CLEAR(buffer);
                                state = State::SIZE;
                            } else {
                                state = State::DATA;
                            }
                        }
                        break;
                    }
                    case State::DATA: {
                        unpacker.feed(&d, 1, b_exec_cb);
                        state = State::SIZE;
                        ES920_STRING_CLEAR(buffer);
                        break;
                    }
                    case State::SIZE:
                    default: {
                        LOG_ERROR("won't come here! curr buffer = ", buffer);
                        for (size_t i = 0; i < ES920_STRING_SIZE(buffer); ++i)
                            LOG_ERROR((int)buffer[i]);

                        ES920_STRING_CLEAR(buffer);
                        state = State::SIZE;
                        break;
                    }
                }
            }
        }

        void subscribe(const uint8_t id, const BinaryCallbackType& cb) {
            unpacker.subscribe(id, cb);
        }

        void subscribe(const BinaryAlwaysCallbackType& cb) {
            unpacker.subscribe(cb);
        }

        void callback() {
            unpacker.callback();
        }

        bool isParsing() const { return unpacker.parsing(); }
        size_t available() const { return unpacker.available(); }

        uint8_t index() const { return unpacker.index(); }
        const uint8_t* data() const { return unpacker.data(); }
        size_t size() const { return unpacker.size(); }
        void pop() { unpacker.pop(); }

        uint8_t index_latest() const { return unpacker.index_latest(); }
        const uint8_t* data_latest() const { return unpacker.data_latest(); }
        size_t size_latest() const { return unpacker.size_latest(); }
        void pop_back() { unpacker.pop_back(); }

        void clear() { unpacker.reset(); }

        int16_t remoteRssi() const { return remote_rssi; }
        const StringType& remotePanid() const { return remote_panid; }
        const StringType& remoteOwnid() const { return remote_ownid; }
        const StringType& remoteHopid() const { return remote_hopid; }

        bool hasReply() { return disableAndReturn(b_reply); }
        bool hasError() { return disableAndReturn(b_error); }

        const StringType& errorCode() const { return error_code; }
        size_t errorCount() const { return error_count; }

    private:
        bool isOkFirstByte() const { return (ES920_STRING_SIZE(buffer) < 1) ? false : (buffer[0] == line_ok_bin[0]); }
        bool isNgFirstByte() const { return (ES920_STRING_SIZE(buffer) < 1) ? false : (buffer[0] == line_ng_bin[0]); }
        bool isOkSecondByte() const { return (ES920_STRING_SIZE(buffer) < 2) ? false : (buffer[1] == line_ok_bin[1]); }
        bool isNgSecondByte() const { return (ES920_STRING_SIZE(buffer) < 2) ? false : (buffer[1] == line_ng_bin[1]); }
        bool isFirstByteReply() const { return (isOkFirstByte() || isNgFirstByte()); }
        bool isSecondByteReply() const { return (isOkSecondByte() || isNgSecondByte()); }

        uint8_t headerSizeES920(const bool b_rssi, const bool b_rcvid) const {
            uint8_t s = 1;
            if (b_rssi) s += 2;
            if (b_rcvid) s += 12;
            return s;
        }

        uint8_t headerSizeES920LR(const bool b_rssi, const bool b_rcvid) const {
            uint8_t s = 1;
            if (b_rssi) s += 4;
            if (b_rcvid) s += 8;
            return s;
        }

        bool parseReply() {
            // delete unexpcted first data
            // ignore rssi & rcvid because first byte varies depending on data size
            if (ES920_STRING_SIZE(buffer) > MAX_HEADER_SIZE) {
                while (ES920_STRING_SIZE(buffer)) {
                    if (!isFirstByteReply()) {
                        LOG_ERROR("first letter is out of range (int) : ", (int)buffer[0]);
                        ES920_STRING_ERASE(buffer, 0, 1);
                    } else
                        break;
                }
            }

            if (ES920_STRING_SIZE(buffer) == 0) {
                state = State::SIZE;
                return false;
            }

            // parse data
            if (
                (ES920_STRING_SIZE(buffer) >= 3) &&
                (ES920_STRING_SUBSTR(buffer, 0, 3) == line_ok_bin)) {
                b_reply = true;
                b_error = false;
                error_code = "000";
                ES920_STRING_ERASE(buffer, 0, 3);
                LOG_INFO("send OK, BINARY");
                return true;
            }

            else if (
                (ES920_STRING_SIZE(buffer) >= 7) &&
                (ES920_STRING_SUBSTR(buffer, 0, 4) == line_ng_bin)) {
                b_reply = true;
                b_error = true;
                error_code = ES920_STRING_SUBSTR(buffer, 4, 3);
                error_count++;
                ES920_STRING_ERASE(buffer, 0, 7);
                LOG_ERROR("send error (BINARY):", error_code, ", error count =", error_count);
                return true;
            }

            return false;
        }

        bool parseHeader(const bool b_rssi, const bool b_rcvid) {
            if (PAYLOAD_SIZE == PAYLOAD_SIZE_ES920)
                return parseHeaderES920(b_rssi, b_rcvid);
            else
                return parseHeaderES920LR(b_rssi, b_rcvid);
        }

        bool parseHeaderES920(const bool b_rssi, const bool b_rcvid) {
            const size_t header_size = headerSizeES920(b_rssi, b_rcvid);
            if (ES920_STRING_SIZE(buffer) >= header_size) {
                uint8_t data_head = 1;
                if (b_rssi) {
                    remote_rssi = -(strtol(ES920_STRING_SUBSTR(buffer, data_head, 2).c_str(), 0, 16)) / 2;
                    data_head += 2;
                }
                if (b_rcvid) {
                    remote_panid = ES920_STRING_SUBSTR(buffer, data_head + 0, 4);
                    remote_hopid = ES920_STRING_SUBSTR(buffer, data_head + 4, 4);
                    remote_ownid = ES920_STRING_SUBSTR(buffer, data_head + 8, 4);
                    LOG_INFO("got remote panid :", remote_panid);
                    LOG_INFO("got remote hopid :", remote_hopid);
                    LOG_INFO("got remote ownid :", remote_ownid);
                }
                ES920_STRING_ERASE(buffer, 0, header_size);
                return true;
            }
            return false;
        }

        bool parseHeaderES920LR(const bool b_rssi, const bool b_rcvid) {
            const size_t header_size = headerSizeES920LR(b_rssi, b_rcvid);
            if (ES920_STRING_SIZE(buffer) >= header_size) {
                uint8_t data_head = 1;
                if (b_rssi) {
                    remote_rssi = ES920_STRING_TO_INT(ES920_STRING_SUBSTR(buffer, data_head, 4));
                    data_head += 4;
                    LOG_INFO("got rssi :", remote_rssi);
                }
                if (b_rcvid) {
                    remote_panid = ES920_STRING_SUBSTR(buffer, data_head, 4);
                    remote_ownid = ES920_STRING_SUBSTR(buffer, data_head + 4, 4);
                    LOG_INFO("got remote panid :", remote_panid);
                    LOG_INFO("got remote ownid :", remote_ownid);
                }
                ES920_STRING_ERASE(buffer, 0, header_size);
                return true;
            }
            return false;
        }
    };

}  // namespace es920
}  // namespace arduino

#endif  // ARDUINO_ES920_BINARY_PARSER_H
