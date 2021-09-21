#pragma once
#ifndef ARDUINO_ES920_CONFIGURATOR_H
#define ARDUINO_ES920_CONFIGURATOR_H

#include "Constants.h"
#include "Utils.h"

namespace arduino {
namespace es920 {
    template <typename Stream>
    class Configurator {
        Stream* stream;

    public:
        void attach(const Stream& s) {
            stream = (Stream*)&s;
        }

        void selectProcessorMode() {
            StringType cmd = "2\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("set processor mode : ", cmd);
        }

        void node(const Node n) {
            StringType cmd = "node " + ES920_STRING_CAST((int)n) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change node : ", cmd);
        }

        void channel(const uint8_t ch) {
            StringType cmd = "channel " + ES920_STRING_CAST((int)ch) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change channel : ", cmd);
        }

        void panid(const uint16_t addr) {
            StringType cmd = "panid " + arx::str::to_hex(addr) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change panid : ", cmd);
        }

        void ownid(const uint16_t addr) {
            StringType cmd = "ownid " + arx::str::to_hex(addr) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change ownid : ", cmd);
        }

        void dstid(const uint16_t addr) {
            StringType cmd = "dstid " + arx::str::to_hex(addr) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change dstid : ", cmd);
        }

        void ack(const bool b) {
            StringType cmd = "ack " + (b ? StringType("1") : StringType("2")) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change ack : ", cmd);
        }

        void retry(const uint8_t n) {
            StringType cmd = "retry " + ES920_STRING_CAST(n) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change retry : ", cmd);
        }

        void transmode(const TransMode m) {
            StringType cmd = "transmode " + ES920_STRING_CAST((int)m) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change transmode : ", cmd);
        }

        void rcvid(const bool b) {
            StringType cmd = "rcvid ";
            cmd += (b ? "1" : "2");
            cmd += "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change rcvid : ", cmd);
        }

        void rssi(const bool b) {
            StringType cmd = "rssi ";
            cmd += (b ? "1" : "2");
            cmd += "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change rssi : ", cmd);
        }

        void operation(const Mode m) {
            StringType cmd = "operation " + ES920_STRING_CAST((int)m) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change operation : ", cmd);
        }

        void baudrate(const Baudrate b) {
            StringType cmd = "baudrate " + ES920_STRING_CAST((int)b) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change baudrate : ", cmd);
        }

        void sleep(const SleepMode m) {
            StringType cmd = "sleep " + ES920_STRING_CAST((int)m) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change sleep : ", cmd);
        }

        void sleeptime(const uint32_t ms) {
            uint32_t st = ms / 100;
            StringType cmd = "sleeptime " + ES920_STRING_CAST(st) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change sleeptime : ", cmd);
        }

        void power(const int8_t pwr) {
            StringType cmd = "power " + ES920_STRING_CAST(pwr) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change power : ", cmd);
        }

        void version() {
            StringType cmd = "version\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change version : ", cmd);
        }

        void save() {
            StringType cmd = "save\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change config : ", cmd);
        }

        void load() {
            StringType cmd = "load\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("load factory setting : ", cmd);
        }

        void start() {
            StringType cmd = "start\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("start operation : ", cmd);
        }

        void format(const Format f) {
            StringType cmd = "format " + ES920_STRING_CAST((int)f) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change format : ", cmd);
        }

        void sendtime(const uint32_t sec) {
            StringType cmd = "sendtime " + ES920_STRING_CAST(sec) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change sendtime : ", cmd);
        }

        void senddata(const StringType& str) {
            StringType cmd = "senddata " + str + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change senddata : ", cmd);
        }

        // for ES920 only

        void hopcount(const uint8_t cnt) {
            StringType cmd = "hopcount " + ES920_STRING_CAST((int)cnt) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change hopcount : ", cmd);
        }

        void endid(const uint16_t addr) {
            StringType cmd = "endid " + arx::str::to_hex(addr) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change endid : ", cmd);
        }

        void route1(const uint16_t addr) {
            StringType cmd = "route1 " + arx::str::to_hex(addr) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change route1 : ", cmd);
        }

        void route2(const uint16_t addr) {
            StringType cmd = "route2 " + arx::str::to_hex(addr) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change route2 : ", cmd);
        }

        void route3(const uint16_t addr) {
            StringType cmd = "route3 " + arx::str::to_hex(addr) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change route3 : ", cmd);
        }

        void rate(const Rate r) {
            StringType cmd = "rate " + ES920_STRING_CAST((int)r) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change rate : ", cmd);
        }

        // for ES920LR only

        void bandwidth(const BW bw) {
            StringType cmd = "bw " + ES920_STRING_CAST((int)bw) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change bw : ", cmd);
        }

        void spreadingfactor(const SF sf) {
            StringType cmd = "sf " + ES920_STRING_CAST((int)sf) + "\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_INFO("change sf : ", cmd);
        }
    };

}  // namespace es920
}  // namespace arduino

#endif  // ARDUINO_ES920_CONFIGURATOR_H
