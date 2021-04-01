#pragma once
#ifndef ARDUINO_ES920_CONSTANTS_H
#define ARDUINO_ES920_CONSTANTS_H

namespace arduino {
namespace es920 {

#ifdef ARDUINO
    using StringType = String;
#elif defined(OF_VERSION_MAJOR)
    using StringType = std::string;
#endif

    enum class ErrorCode : uint8_t {
        UndefinedCommand = 1,
        OptionValue = 2,
        FlashErase = 3,
        FlashWrite = 4,
        FlashRead = 5,
        SendDataLength = 100,
        SendProcessError = 101,
        CarriorSense = 102,
        MissingAck = 103
    };

    // common

    enum class Mode : uint8_t {
        CONFIG = 1,
        OPERATION
    };

    enum class Node : uint8_t {
        COORDINATOR = 1,
        ENDDEVICE
    };

    enum class TransMode : uint8_t {
        PAYLOAD = 1,
        FRAME
    };

    enum class Baudrate : uint8_t {
        BD_9600 = 1,
        BD_19200,
        BD_38400,
        BD_57600,
        BD_115200,
        BD_230400
    };

    enum class SleepMode : uint8_t {
        NO_SLEEP = 1,
        TIMER_WAKEUP,
        INT_WAKEUP
    };

    enum class Format : uint8_t {
        ASCII = 1,
        BINARY
    };

    // only for ES920

    enum class Rate : uint8_t {
        RATE_50KBPS = 1,
        RATE_100KBPS
    };

    namespace ChannelRate50kbps {
        enum ChannelRate50kbps : uint8_t {
            CH01_920_6_MHZ = 1,
            CH02_920_8_MHZ,
            CH03_921_0_MHZ,
            CH04_921_2_MHZ,
            CH05_921_4_MHZ,
            CH06_921_6_MHZ,
            CH07_921_8_MHZ,
            CH08_922_0_MHZ,
            CH09_922_2_MHZ,
            CH10_922_4_MHZ,
            CH11_922_6_MHZ,
            CH12_922_8_MHZ,
            CH13_923_0_MHZ,
            CH14_923_2_MHZ,
            CH15_923_4_MHZ,
            CH16_923_6_MHZ,
            CH17_923_8_MHZ,
            CH18_924_0_MHZ,
            CH19_924_2_MHZ,
            CH20_924_4_MHZ,
            CH21_924_6_MHZ,
            CH22_924_8_MHZ,
            CH23_925_0_MHZ,
            CH24_925_2_MHZ,
            CH25_925_4_MHZ,
            CH26_925_6_MHZ,
            CH27_925_8_MHZ,
            CH28_926_0_MHZ,
            CH29_926_2_MHZ,
            CH30_926_4_MHZ,
            CH31_926_6_MHZ,
            CH32_926_8_MHZ,
            CH33_927_0_MHZ,
            CH34_927_2_MHZ,
            CH35_927_4_MHZ,
            CH36_927_6_MHZ,
            CH37_927_8_MHZ,
            CH38_928_0_MHZ,
        };
    }

    namespace ChannelRate100kbps {
        enum ChannelRate100kbps : uint8_t {
            CH01_920_7_MHZ = 1,
            CH02_921_1_MHZ,
            CH03_921_5_MHZ,
            CH04_921_9_MHZ,
            CH05_922_3_MHZ,
            CH06_922_7_MHZ,
            CH07_923_1_MHZ,
            CH08_923_5_MHZ,
            CH09_923_9_MHZ,
            CH10_924_3_MHZ,
            CH11_924_7_MHZ,
            CH12_925_1_MHZ,
            CH13_925_5_MHZ,
            CH14_925_9_MHZ,
            CH15_926_3_MHZ,
            CH16_926_7_MHZ,
            CH17_927_1_MHZ,
            CH18_927_5_MHZ,
            CH19_927_9_MHZ,
        };
    }

    // only for ES920LR

    enum class BW : uint8_t {
        BW_62_5_KHZ = 3,
        BW_125_KHZ,  // default
        BW_250_KHZ,
        BW_500_KHZ
    };

    enum class SF : uint8_t {
        SF_7 = 7,  // default
        SF_8,
        SF_9,
        SF_10,
        SF_11,
        SF_12
    };

    namespace ChannelBW62_5kHz {
        enum ChannelBW62_5kHz : uint8_t {
            CH01_920_6_MHZ = 1,
            CH02_920_8_MHZ,
            CH03_921_0_MHZ,
            CH04_921_2_MHZ,
            CH05_921_4_MHZ,
            CH06_921_6_MHZ,
            CH07_921_8_MHZ,
            CH08_922_0_MHZ,
            CH09_922_2_MHZ,
            CH10_922_4_MHZ,
            CH11_922_6_MHZ,
            CH12_922_8_MHZ,
            CH13_923_0_MHZ,
            CH14_923_2_MHZ,
            CH15_923_4_MHZ,
        };
    }

    namespace ChannelBW125kHz {
        enum ChannelBW125kHz : uint8_t {
            CH01_920_6_MHZ = 1,
            CH02_920_8_MHZ,
            CH03_921_0_MHZ,
            CH04_921_2_MHZ,
            CH05_921_4_MHZ,
            CH06_921_6_MHZ,
            CH07_921_8_MHZ,
            CH08_922_0_MHZ,
            CH09_922_2_MHZ,
            CH10_922_4_MHZ,
            CH11_922_6_MHZ,
            CH12_922_8_MHZ,
            CH13_923_0_MHZ,
            CH14_923_2_MHZ,
            CH15_923_4_MHZ,
        };
    }

    namespace ChennelBW250kHz {
        enum ChannelBW250kHz : uint8_t {
            CH01_920_7_MHZ = 1,
            CH02_921_1_MHZ,
            CH03_921_5_MHZ,
            CH04_921_9_MHZ,
            CH05_922_3_MHZ,
            CH06_922_7_MHZ,
            CH07_923_1_MHZ,
        };
    }

    namespace ChannelBW500kHz {
        enum ChannelBW500kHz : uint8_t {
            CH01_920_8_MHZ = 1,
            CH02_921_4_MHZ,
            CH03_922_0_MHZ,
            CH04_922_6_MHZ,
            CH05_923_2_MHZ,
        };
    }

    constexpr uint8_t PAYLOAD_SIZE_ES920 {225};
    constexpr uint8_t PAYLOAD_SIZE_ES920LR {50};

}  // namespace es920
}  // namespace arduino

#endif  // ARDUINO_ES920_CONSTANTS_H
