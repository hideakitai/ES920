#pragma once
#ifndef ARDUINO_ES920_H
#define ARDUINO_ES920_H

#define PACKETIZER_USE_INDEX_AS_DEFAULT
#define PACKETIZER_USE_CRC_AS_DEFAULT

#include "ES920/util/ArxContainer/ArxContainer.h"
#include "ES920/util/DebugLog/DebugLog.h"
#include "ES920/util/ArxStringUtils/ArxStringUtils.h"

#ifndef ARDUINO
    #include <string>
#elif defined (TEENSYDUINO)
    #include "ES920/util/TeensyDirtySTLErrorSolution/TeensyDirtySTLErrorSolution.h"
#endif

#include "ES920/Constants.h"
#include "ES920/Utils.h"
#include "ES920/Configurator.h"
#include "ES920/Parser.h"
#include "ES920/Operator.h"

namespace arduino {
namespace es920 {

    struct Config
    {
        // ES920 only
        Rate     rate     {Rate::RATE_50KBPS};
        uint8_t  hopcount {1};
        uint16_t endid    {0x0000};
        uint16_t route1   {0x0001};
        uint16_t route2   {0x0001};
        uint16_t route3   {0x0001};

        // ES920LR only
        BW bw {BW::BW_125_KHZ};
        SF sf {SF::SF_7};

        // common
        Node       node      {Node::ENDDEVICE};
        uint8_t    channel   {1};
        uint16_t   panid     {0x0001};
        uint16_t   ownid     {0x0001};
        uint16_t   dstid     {0x0000};
        bool       ack       {true};
        uint8_t    retry     {3};
        TransMode  transmode {TransMode::PAYLOAD};
        bool       rcvid     {false};
        bool       rssi      {false};
        Mode       operation {Mode::CONFIG};
        Baudrate   baudrate  {Baudrate::BD_115200};
        SleepMode  sleep     {SleepMode::NO_SLEEP};
        uint32_t   sleeptime {50};
        uint8_t    power     {13};
        Format     format    {Format::ASCII};
        uint32_t   sendtime  {0};
        StringType senddata  {""};

#ifdef OF_VERSION_MAJOR
        // serial i/f name
        StringType device { "" };
#endif
    };


    template <typename Stream, uint8_t PIN_RST, uint8_t PAYLOAD_SIZE>
    class ES920Base
    {
    protected:

        Configurator<Stream> configurator;
        Operator<Stream, PAYLOAD_SIZE> sender;
        Parser<Stream, PAYLOAD_SIZE> parser;

        Stream* stream;
        Config configs;

        const uint32_t wait_reply_ms {200};
        const uint32_t wait_start_ms {200};
        const uint32_t wait_reset_ms {2000};
        const uint32_t wait_reset_gpio_ms {50};
        const uint32_t wait_reset_manual_ms {5000};
        const uint32_t wait_config_trigger_ms {100};

    public:

        template <typename SerialType>
        void attach(SerialType& s, const Config& cfg, const bool b_verbose = false)
        {
            verbose(b_verbose);
            configs = cfg;
            changeBaudRate(s);
            stream = (Stream*)&s;
            configurator.attach(s);
            sender.attach(s);
            parser.attach(s, configs.baudrate);
            parser.clear();
#ifdef ARDUINO
            if (isResetPinSelected())
                pinMode(PIN_RST, OUTPUT);
#endif
            stream->flush();
            while (stream->available()) stream->read();
        }

        template <typename SerialType>
        bool begin(SerialType& s, const Config& cfg,
            const bool b_config_check = true,
            const bool b_force_config = true,
            const bool b_verbose = false
        ){
            attach(s, cfg, b_verbose);

#ifdef ARDUINO
#ifdef ESP_PLATFORM
            s.begin(configToBaudrate(cfg.baudrate));
#endif
            reset();
            if (!b_config_check) return detectReset();
#else
            if (!b_config_check) return true;
            reset();
#endif

            bool b_config = b_force_config;
            if (detectReset())
            {
                // force config if operation mode not matched
                if (detectMode() != configs.operation) b_config = true;
            }
            else
            {
                LOG_ERROR("cannot connect to module! or baudrate is different!");

                if (baudrate() != Baudrate::BD_115200)
                {
                    LOG_VERBOSE("retry with default 115200 baud");
                    Config c = cfg;
                    c.baudrate = Baudrate::BD_115200;
                    attach(s, c, b_verbose);
                    reset();

                    if (detectReset())
                    {
                        // force config if operation mode not matched
                        if (detectMode() != configs.operation) b_config = true;
                    }
                    else
                    {
                        attach(s, cfg, b_verbose);
                        LOG_ERROR("cannot connect to module! please check wiring");
                        return false;
                    }
                }
                else
                {
                    LOG_ERROR("cannot connect to module! please check wiring");
                    return false;
                }
            }

            // execute auto configuration
            if (b_config)
            {
                // wait until entering to config mode
                // timeout after 4sec for auto reset (if reset pin is availble)
                // timeout after 30sec for manual reset (if reset pin is not asigned)
                if (autoProcedureFromAnywhereToConfigMode(10000))
                {
                    LOG_VERBOSE("successfully entered to configuration mode!");
                    bool b_success = config(s, cfg);
                    if (!b_success) LOG_ERROR("some configuration setting has write error!");
                    return b_success;
                }
                else
                {
                    LOG_ERROR("failed to enter configuration mode!");
                    return false;
                }
            }

            return true;
        }

        template <typename SerialType>
        bool config(SerialType& s, const Config& cfg)
        {
            configs = cfg;

            bool success = true;

            success &= configDeviceSpecificMode(cfg);

            // basic configuration (common)
            success &= channel(configs.channel);
            success &= node(configs.node); // COORDINATOR, ENDDEVICE
            success &= format(configs.format); // ASCII, BINARY
            success &= transmode(configs.transmode); // PAYLOAD, FRAME

            // id settings
            success &= panid(configs.panid); // 0x0001 - 0xFFFE
            success &= ownid(configs.ownid); // 0x0000 - 0xFFFE
            success &= dstid(configs.dstid); // 0x0000 - 0xFFFF (0xFFFF = broadcast)

            success &= ack(configs.ack);
            success &= retry(configs.retry); // 0 - 10
            success &= power(configs.power); // -4dB to +13dB

            // options
            success &= rssi(configs.rssi); // add rssi info to data
            success &= rcvid(configs.rcvid); // add remote panid & ownid to data

            // finally change baudrate
            LOG_VERBOSE("change to baudrate", configToBaudrate(configs.baudrate));
            success &= baudrate(configs.baudrate); // baudrate will be changed right after this command
            wait(100);
            changeBaudRate(s);
            wait(100);
            LOG_VERBOSE("changed baudrate to", configToBaudrate(configs.baudrate));
            LOG_VERBOSE("configuration done, change to operation mode");

            // change operation mode and save configuration and restart
            success &= autoProcedureSaveAndRestart(10000);

            return success;
        }

        virtual bool configDeviceSpecificMode(const Config& cfg) = 0;


        // sending data

        bool send(const StringType& str, const uint32_t timeout_ms = 0)
        {
            if (configs.transmode != TransMode::PAYLOAD)
            {
                LOG_WARNING("TransMode is not matched. Please set PAN ID & OWN ID");
                return false;
            }
            if (!sender.sendPayload(str)) return false;
            return (timeout_ms != 0) ? parser.detectReplyAscii(timeout_ms) : true;
        }

        bool send(const uint8_t* data, const uint8_t size, const uint32_t timeout_ms = 0)
        {
            return send(0, data, size, timeout_ms);
        }

        bool send(const uint8_t index, const uint8_t* data, const uint8_t size, const uint32_t timeout_ms = 0)
        {
            if (configs.transmode != TransMode::PAYLOAD)
            {
                LOG_WARNING("TransMode is not matched. Please set PAN ID & OWN ID");
                return false;
            }
            if (!sender.sendPayload(data, size, index)) return false;
            return (timeout_ms != 0) ? parser.detectReplyBinary(timeout_ms) : true;
        }

        bool send(const uint16_t pan, const uint16_t own, const StringType& str, const uint32_t timeout_ms = 0)
        {
            if (configs.transmode != TransMode::FRAME)
            {
                LOG_WARNING("TransMode is not matched. Please remove PAN ID & OWN ID");
                return false;
            }
            if (!sender.sendFrame(pan, own, str)) return false;
            return (timeout_ms != 0) ? parser.detectReplyAscii(timeout_ms) : true;
        }

        bool send(const uint16_t pan, const uint16_t own, const uint8_t* data, const uint8_t size, const uint32_t timeout_ms = 0)
        {
            return send(pan, own, 0, data, size, timeout_ms);
        }

        bool send(const uint16_t pan, const uint16_t own, const uint8_t index, const uint8_t* data, const uint8_t size, const uint32_t timeout_ms = 0)
        {
            if (configs.transmode != TransMode::FRAME)
            {
                LOG_WARNING("TransMode is not matched. Please remove PAN ID & OWN ID");
                return false;
            }
            if (!sender.sendFrame(pan, own, data, size, index)) return false;
            return (timeout_ms != 0) ? parser.detectReplyBinary(timeout_ms) : true;
        }


        // received data management

        void subscribe(const uint8_t id, const BinaryCallbackType& cb)
        {
            parser.subscribeBinary(id, cb);
        }

        void subscribe(const BinaryAlwaysCallbackType& cb)
        {
            parser.subscribeBinary(cb);
        }

        void subscribe(const AsciiCallbackType& cb)
        {
            parser.subscribeAscii(cb);
        }

        size_t parse(const bool b_exec_cb = true)
        {
            if ((configs.operation == Mode::OPERATION) && (configs.format == Format::BINARY))
                return parser.parseBinary(configs.rssi, configs.rcvid, b_exec_cb);
            else
                return parser.parseAscii(configs.rssi, configs.rcvid, b_exec_cb);
        }

        void callback()
        {
            if ((configs.operation == Mode::OPERATION) && (configs.format == Format::BINARY))
                return parser.callbackBinary();
            else
                return parser.callbackAscii();
        }

        size_t available() const
        {
            if (configs.format == Format::BINARY)
                return parser.availableBinary();
            else
                return parser.availableAscii();
        }

        uint8_t index() const
        {
            if (configs.format == Format::BINARY)
                return parser.indexBinary();
            else
                return parser.indexAscii();
        }

        uint8_t index_back() const
        {
            if (configs.format == Format::BINARY)
                return parser.indexBackBinary();
            else
                return parser.indexBackAscii();
        }

        const uint8_t* data() const
        {
            if (configs.format == Format::ASCII)
                return (const uint8_t*)parser.dataAscii().c_str();
            else
                return parser.dataBinary();
        }

        const uint8_t* data_back() const
        {
            if (configs.format == Format::ASCII)
                return (const uint8_t*)parser.dataBackAscii().c_str();
            else
                return parser.dataBackBinary();
        }

        uint8_t data(const uint8_t i) const
        {
            if (configs.format == Format::ASCII)
                return parser.dataAscii()[i];
            else
                return parser.dataBinary()[i];
        }

        uint8_t size() const
        {
            if (configs.format == Format::ASCII)
                return parser.sizeAscii();
            else
                return parser.sizeBinary();
        }

        uint8_t size_back() const
        {
            if (configs.format == Format::ASCII)
                return parser.sizeBackAscii();
            else
                return parser.sizeBackBinary();
        }

        const StringType& dataString() const
        {
            return parser.dataAscii();
        }

        void pop()
        {
            if (configs.format == Format::ASCII)
                return parser.popAscii();
            else
                return parser.popBinary();
        }

        void pop_back()
        {
            if (configs.format == Format::ASCII)
                return parser.popBackAscii();
            else
                return parser.popBackBinary();
        }

        int16_t remoteRssi() const
        {
            if (configs.format == Format::ASCII)
                return parser.remoteRssiAscii();
            else
                return parser.remoteRssiBinary();
        }

        const StringType& remotePanid() const
        {
            if (configs.format == Format::ASCII)
                return parser.remotePanidAscii();
            else
                return parser.remotePanidBinary();
        }
        const StringType& remoteOwnid() const
        {
            if (configs.format == Format::ASCII)
                return parser.remoteOwnidAscii();
            else
                return parser.remoteOwnidBinary();
        }
        const StringType& remoteHopid() const // only for ES920
        {
            if (configs.format == Format::ASCII)
                return parser.remoteHopidAscii();
            else
                return parser.remoteHopidBinary();
        }

        bool hasReply()
        {
            if (configs.format == Format::ASCII)
                return parser.hasReplyAscii();
            else
                return parser.hasReplyBinary();
        }

        bool hasError()
        {
            if (configs.format == Format::ASCII)
                return parser.hasErrorAscii();
            else
                return parser.hasErrorBinary();
        }

        const StringType& errorCode() const
        {
            if (configs.format == Format::ASCII)
                return parser.errorCodeAscii();
            else
                return parser.errorCodeBinary();
        }
        size_t errorCount() const
        {
            if (configs.format == Format::ASCII)
                return parser.errorCountAscii();
            else
                return parser.errorCountBinary();
        }


        // configure commands

        bool node(const Node n)
        {
            configurator.node(n);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.node = n;
                return true;
            }
            return false;
        }

        bool channel(const uint8_t ch)
        {
            configurator.channel(ch);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.channel = ch;
                return true;
            }
            return false;
        }

        bool panid(const uint16_t addr)
        {
            if ((addr < 0x0001) || (addr > 0xFFFE))
            {
                LOG_WARNING("PAN ID is out of range : ", arx::str::to_hex(addr));
                return false;
            }
            configurator.panid(addr);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.panid = addr;
                return true;
            }
            return false;
        }

        bool ownid(const uint16_t addr)
        {
            if (configs.node == Node::COORDINATOR)
            {
                if (addr != 0)
                {
                    LOG_WARNING("coordinator's ownid must be 0");
                    return false;
                }
            }
            else
            {
                if ((addr < 0x0001) || (addr > 0xFFFE))
                {
                    LOG_WARNING("OWN ID is out of range : ", arx::str::to_hex(addr));
                    return false;
                }
            }
            configurator.ownid(addr);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.ownid = addr;
                return true;
            }
            return false;
        }

        bool dstid(const uint16_t addr)
        {
            configurator.dstid(addr);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.dstid = addr;
                return true;
            }
            return false;
        }

        bool dstidBroadcast()
        {
            return dstid(0xFFFF);
        }

        bool ack(const bool b)
        {
            configurator.ack(b);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.ack = b;
                return true;
            }
            return false;
        }

        bool retry(const uint8_t i)
        {
            if (i > 10)
            {
                LOG_WARNING("too many retry : ", i);
                return false;
            }
            configurator.retry(i);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.retry = i;
                return true;
            }
            return false;
        }

        bool transmode(const TransMode m)
        {
            configurator.transmode(m);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.transmode = m;
                return true;
            }
            return false;
        }

        bool rcvid(const bool b)
        {
            configurator.rcvid(b);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.rcvid = b;
                return true;
            }
            return false;
        }

        bool rssi(const bool b)
        {
            configurator.rssi(b);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.rssi = b;
                return true;
            }
            return false;
        }

        bool operation(const Mode m)
        {
            configurator.operation(m);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.operation = m;
                return true;
            }
            return false;
        }

        bool baudrate(const Baudrate b)
        {
            configs.baudrate = b;
            configurator.baudrate(b);
            parser.setBaudrate(b);
            return true; // we cant get reply because baudrate has already been changed
        }

        bool sleep(const SleepMode m)
        {
            configurator.sleep(m);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.sleep = m;
                return true;
            }
            return false;
        }

        bool sleeptime(const uint32_t ms)
        {
            configurator.sleeptime(ms);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.sleeptime = ms;
                return true;
            }
            return false;
        }

        bool power(const int8_t pwr)
        {
            if ((pwr < -4) || (pwr > 13))
            {
                LOG_WARNING("power value is out of range : ", pwr);
                return false;
            }
            configurator.power(pwr);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.power = pwr;
                return true;
            }
            return false;
        }

        StringType version()
        {
            configurator.version();
            return parser.checkVersion();
        }

        bool save()
        {
            configurator.save();
            return parser.detectReplyAscii(wait_reply_ms);
        }

        bool load()
        {
            configurator.load();
            return parser.detectReplyAscii(wait_reply_ms);
        }

        bool start()
        {
            configurator.start();
            return parser.detectReplyAscii(wait_reply_ms);
        }

        bool format(const Format f)
        {
            configurator.format(f);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.format = f;
                return true;
            }
            return false;
        }

        bool sendtime(const uint32_t sec)
        {
            configurator.sendtime(sec);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.sendtime = sec;
                return true;
            }
            return false;
        }

        bool senddata(const StringType& str)
        {
            if (ES920_STRING_SIZE(str) > PAYLOAD_SIZE_ES920LR)
            {
                LOG_WARNING("too long data, must be under : ", ES920_STRING_SIZE(str));
                return false;
            }
            configurator.senddata(str);
            if (parser.detectReplyAscii(wait_reply_ms))
            {
                configs.senddata = str;
                return true;
            }
            return false;
        }


        // get internal variables

        const Config& getConfigs() const { return configs; }
        Node node() const { return configs.node; }
        uint8_t channel() const { return configs.channel; }
        uint16_t panid() const { return configs.panid; }
        uint16_t ownid() const { return configs.ownid; }
        uint16_t dstid() const { return configs.dstid; }
        bool ack() const { return configs.ack; }
        uint8_t retry() const { return configs.retry; }
        TransMode transmode() const { return configs.transmode; }
        bool rcvid() const { return configs.rcvid; }
        bool rssi() const { return configs.rssi; }
        Mode operation() const { return configs.operation; }
        Baudrate baudrate() const { return configs.baudrate; }
        SleepMode sleep() const { return configs.sleep; }
        uint32_t sleeptime() const { return configs.sleeptime; }
        int8_t power() const { return configs.power; }
        Format format() const { return configs.format; }
        uint32_t sendtime() const { return configs.sendtime; }
        const StringType& senddata() const { return configs.senddata; }


        void reset()
        {
#ifdef ARDUINO
            reset(true);
            delay(wait_reset_gpio_ms);
            stream->flush();
            while (stream->available()) ES920_READ_BYTE();
            parser.clear();
            reset(false);
            LOG_VERBOSE("reset signal trigger done");
#else
            stream->flush();
            while (stream->available()) ES920_READ_BYTE();
            parser.clear();
            PRINTLN("please push reset button");
            wait(wait_reset_manual_ms);
#endif
        }

        // for debug
        void verbose(const bool b) { LOG_SET_LEVEL(b ? DebugLogLevel::VERBOSE : DebugLogLevel::ERRORS); }
#ifndef NDEBUG
        bool verbose() const { return LOG_GET_LEVEL() == DebugLogLevel::VERBOSE; }
#endif


    private:

        bool isResetPinSelected() const { return (PIN_RST != 0xFF); }

        uint32_t configToBaudrate(const Baudrate b)
        {
            switch(b)
            {
                case Baudrate::BD_9600:   return 9600;
                case Baudrate::BD_19200:  return 19200;
                case Baudrate::BD_38400:  return 38400;
                case Baudrate::BD_57600:  return 57600;
                case Baudrate::BD_115200: return 115200;
                case Baudrate::BD_230400: return 230400;
                default:                  return 115200;
            }
        }


        // utility to change operation and config mode

        bool autoProcedureFromAnywhereToConfigMode(const uint32_t timeout_ms)
        {
            uint32_t start_ms = ELAPSED_TIME_MS();

            while (ELAPSED_TIME_MS() < start_ms + timeout_ms)
            {
                // reset to detect current mode
                while (1)
                {
                    reset();
                    if (detectReset()) break;
                    LOG_ERROR("reset has not been detected... try again");
                }

                // check curent mode
                Mode m = detectMode();
                LOG_VERBOSE("detected mode is ", (int)m);

                // if current mode is Mode::Config, select Processor Mode
                if (m == Mode::CONFIG)
                {
                    LOG_VERBOSE("enter to Processor Mode");
                    selectProcessorMode();
                    LOG_VERBOSE("mode change done, current mode is ", (int)m);
                    return true;
                }
                // if current mode is Mode::Operation, trigger to enter config mode and reset again
                else
                {
                    LOG_VERBOSE("maybe operation mode, reboot to enter config mode");
                    fromOperationToConfigTrigger();
                }
            }
            LOG_ERROR("mode change failed, timeout");
            return false;
        }

        bool autoProcedureSaveAndRestart(const uint32_t timeout_ms, const bool b_save = true)
        {
            operation(configs.operation);

            if (b_save) save();
            else        start();

            uint32_t start_ms = ELAPSED_TIME_MS();

            while (ELAPSED_TIME_MS() < start_ms + timeout_ms)
            {
                // reset to change to operation mode
                reset();

                if (detectReset())
                {
                    LOG_VERBOSE("reset detected !");
                    break;
                }
                else
                {
                    LOG_ERROR("reset has not been detected... try again");
                }
            }

            LOG_VERBOSE("current mode is ", (int)operation());

            auto m = detectMode();
            if (m == configs.operation)
            {
                LOG_VERBOSE("operation mode change success!");

                if (m == Mode::OPERATION)
                    LOG_VERBOSE("start operation");
                else
                    LOG_VERBOSE("start config mode");
                return true;
            }
            else
            {
                LOG_ERROR("operation mode is not expected!");
                return false;
            }
        }


        // utility to manage special reply (especially in boot sequence)

        bool detectReset()
        {
            return parser.detectReset(wait_reset_ms);
        }

        Mode detectMode()
        {
            return parser.detectMode(wait_start_ms);
        }

        bool selectProcessorMode()
        {
            configurator.selectProcessorMode();
            return parser.detectReplyAscii(wait_reply_ms);
        }

        void fromOperationToConfigTrigger()
        {
            StringType cmd = "config\r\n";
            ES920_WRITE_BYTES(cmd.c_str(), ES920_STRING_SIZE(cmd));
            LOG_VERBOSE("from operation to config : ", cmd);
            wait(wait_config_trigger_ms);
        }

#ifdef ARDUINO
        void reset(const bool b)
        {
            if (isResetPinSelected())
            {
                LOG_VERBOSE("reset module ", !b);
                digitalWrite(PIN_RST, !b);
            }
        }
#endif

        template <typename SerialType>
        void changeBaudRate(SerialType& s)
        {
#ifdef ARDUINO
#ifdef ESP_PLATFORM
            s.updateBaudRate(configToBaudrate(configs.baudrate));
#else
            ES920_SERIAL_BEGIN(s, configToBaudrate(configs.baudrate));
#endif
#elif defined OF_VERSION_MAJOR
            ES920_SERIAL_END(s);
            ES920_SERIAL_BEGIN(s, configs.device, configToBaudrate(configs.baudrate));
#endif
        }
    };


    template <typename Stream, uint8_t PIN_RST = 0xFF>
    class ES920_ : public ES920Base<Stream, PIN_RST, PAYLOAD_SIZE_ES920>
    {
    public:

        bool hopcount(const uint8_t i)
        {
            if ((i < 1) || (i > 4))
            {
                LOG_WARNING("hop count is out of range : ", i);
                return false;
            }
            this->configurator.hopcount(i);
            return this->parser.detectReplyAscii(this->wait_reply_ms);
        }

        bool endid(const uint16_t addr)
        {
            if (addr > 0xFFFE)
            {
                LOG_WARNING("END ID is out of range : ", arx::str::to_hex(addr));
                return false;
            }
            this->configurator.endid(addr);
            return this->parser.detectReplyAscii(this->wait_reply_ms);
        }

        bool route1(const uint16_t addr)
        {
            if ((addr < 0x0001) || (addr > 0xFFFE))
            {
                LOG_WARNING("route1 is out of range : ", arx::str::to_hex(addr));
                return false;
            }
            this->configurator.route1(addr);
            return this->parser.detectReplyAscii(this->wait_reply_ms);
        }

        bool route2(const uint16_t addr)
        {
            if ((addr < 0x0001) || (addr > 0xFFFE))
            {
                LOG_WARNING("route2 is out of range : ", arx::str::to_hex(addr));
                return false;
            }
            this->configurator.route2(addr);
            return this->parser.detectReplyAscii(this->wait_reply_ms);
        }

        bool route3(const uint16_t addr)
        {
            if ((addr < 0x0001) || (addr > 0xFFFE))
            {
                LOG_WARNING("route3 is out of range : ", arx::str::to_hex(addr));
                return false;
            }
            this->configurator.route3(addr);
            return this->parser.detectReplyAscii(this->wait_reply_ms);
        }

        bool rate(const Rate r)
        {
            this->configurator.rate(r);
            return this->parser.detectReplyAscii(this->wait_reply_ms);
        }

        uint8_t hopcount() const { return this->configs.hopcount; }
        uint16_t endid() const { return this->configs.endid; }
        uint16_t route1() const { return this->configs.route1; }
        uint16_t route2() const { return this->configs.route2; }
        uint16_t route3() const { return this->configs.route3; }
        Rate rate() const { return this->configs.rate; }

    private:

        virtual bool configDeviceSpecificMode(const Config& cfg) override
        {
            bool b = true;
            b &= rate(cfg.rate);
            b &= hopcount(cfg.hopcount);
            b &= endid(cfg.endid);
            b &= route1(cfg.route1);
            b &= route2(cfg.route2);
            b &= route3(cfg.route3);
            return b;
        }

        // inline uint32_t lagUs()
        // {
        //     if (this->configs.format == Format::ASCII)
        //         return this->parser.lag(this->parser.sizeAscii());
        //     else
        //         return this->parser.lag(this->parser.sizeBinary());
        // }

        // bool isSuspendTimeElapsed()
        // {
        //     if (fps.isRunning() && !fps.isNext())
        //     {
        //         LOG_WARNING("you must wait until the next timing...");
        //         return false;
        //     }
        //     return true;
        // }

        // void addTransmissionTime(const StringType& str)
        // {
        //     uint8_t size = ES920_STRING_SIZE(str) + 2; // CR LF
        //     addTransmissionTime(size);
        // }

        // void addTransmissionTime(const uint8_t size)
        // {
        //     sum_comm_ms += ((double)size + 26.) / ((double)rate_khz / 8. * 1024.) * 1000.;
        // }
    };


    template <typename Stream, uint8_t PIN_RST = 0xFF>
    struct ES920LR_ : public ES920Base<Stream, PIN_RST, PAYLOAD_SIZE_ES920LR>
    {
    public:

        bool bandwidth(const BW bw)
        {
            this->configurator.bandwidth(bw);
            return this->parser.detectReplyAscii(this->wait_reply_ms);
        }

        bool spreadingfactor(const SF sf)
        {
            this->configurator.spreadingfactor(sf);
            return this->parser.detectReplyAscii(this->wait_reply_ms);
        }

        BW bandwidth() const { return this->configs.bw; }
        SF spreadingfactor() const { return this->configs.sf; }

    private:

        virtual bool configDeviceSpecificMode(const Config& cfg) override
        {
            bool b = true;
            b &= bandwidth(cfg.bw);
            b &= spreadingfactor(cfg.sf);
            return b;
        }

    };


#ifdef ARDUINO
    template <uint8_t PIN_RST = 0xFF>
    using ES920 = ES920_<Stream, PIN_RST>;
    template <uint8_t PIN_RST = 0xFF>
    using ES920LR = ES920LR_<Stream, PIN_RST>;
#else
    using ES920 = ES920_<ofSerial>;
    using ES920LR = ES920LR_<ofSerial>;
#endif

} // namespace es920
} // namespace arduino

namespace ES920 = arduino::es920;

#endif // ARDUINO_ES920_H
