// #define ES920_DEBUGLOG_ENABLE
#include <ES920.h>

const uint8_t PIN_RST {9};
ES920::Config config;

// change ES920 or ES920LR here
ES920::ES920<PIN_RST> subghz;
// ES920::ES920LR<PIN_RST> subghz;

void setup() {
    Serial.begin(115200);
    delay(2000);

    // set config struct
    // you only need to make changes if necessary
    // others will be set as default value

    // ES920 only
    config.rate = ES920::Rate::RATE_100KBPS;
    config.hopcount = 1;
    config.endid = 0x0000;
    config.route1 = 0x0001;
    config.route2 = 0x0001;
    config.route3 = 0x0001;

    // ES920LR only
    config.bw = ES920::BW::BW_125_KHZ;
    config.sf = ES920::SF::SF_7;

    // common
    config.node = ES920::Node::ENDDEVICE;
    config.channel = ES920::ChannelRate100kbps::CH02_921_1_MHZ;
    config.panid = 0x0003;
    config.ownid = 0x0004;
    config.dstid = 0x0000;
    config.ack = true;
    config.retry = 3;
    config.transmode = ES920::TransMode::PAYLOAD;
    config.rcvid = false;
    config.rssi = false;
    config.operation = ES920::Mode::OPERATION;
    config.baudrate = ES920::Baudrate::BD_230400;
    config.sleep = ES920::SleepMode::NO_SLEEP;
    config.sleeptime = 50;
    config.power = 13;
    config.format = ES920::Format::ASCII;
    config.sendtime = 0;
    config.senddata = "";

    // set ascii format callback
    // you can add only one callback
    subghz.subscribe([](const String& str) {
        // PRINTLN is utility for ES920 library, see below for detail
        // https://github.com/hideakitai/DebugLog
        PRINTLN("subghz data received! size =", str.length());
        PRINTLN("data =", str);
    });

    // begin ES920 with your configuration
    // set required Serial here and optionally you can choose:
    // - b_config_check : check if current operation mode is matched to desired
    // - b_force_config : (optional, default = true) force to write config data if b_config_check is true
    // - b_verbose      : (optional, default = false) show verbose log to your usb-serial (Serial)

    if (subghz.begin(Serial3, config, true, true, false))
        Serial.println("begin ES920 sucess!");
    else
        Serial.println("begin ES920 failed!");

    Serial.println("start operation mode");
}

void loop() {
    subghz.parse();  // must be called to trigger callback

    // send data in one seconds
    static uint32_t prev_ms = millis();
    if (millis() > prev_ms + 1000) {
        String data = "hello, now = ";
        data += String((int)((prev_ms / 1000) % 255));

        subghz.send(data);

        PRINTLN("send data : size =", data.length());
        PRINTLN("data =", data);

        prev_ms = millis();
    }
}
