#include "ofApp.h"
#include "ofxEs920.h"

ES920::ES920 subghz;
// ES920::ES920LR subghz;
ES920::Config config;
ofSerial serial;

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(false);
    ofSetFrameRate(60);
    ofSetBackgroundColor(ofColor::black);

    serial.listDevices();
    std::cout << "start config" << std::endl;
    LOG_SET_LEVEL(DebugLogLevel::INFO);

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
    config.node = ES920::Node::COORDINATOR;
    config.channel = ES920::ChannelRate100kbps::CH02_921_1_MHZ;
    config.panid = 0x0003;
    config.ownid = 0x0000;
    config.dstid = 0x0004;
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

    // serial device name (oF only)
    config.device = "COM61";

    // set binary format callback and data index
    // you can add several callbacks depending on index
    subghz.subscribe([](const std::string& str)
    {
        // PRINTLN is utility for ES920 library, see below for detail
        // https://github.com/hideakitai/DebugLog
        PRINTLN("subghz data received! size =", str.length());
        PRINTLN("data =", str);
    });


    PRINTLN("start setup subghz");
    // if (subghz.begin(serial, config, true, true, true))
    if (subghz.begin(serial, config, false))
        PRINTLN("begin ES920 sucess!");
    else
        PRINTLN("begin ES920 failed!");
}

//--------------------------------------------------------------
void ofApp::update()
{
    subghz.parse(); // must be called to trigger callback

    // send data in one seconds
    static uint32_t prev_ms = ofGetElapsedTimeMillis();
    if (ofGetElapsedTimeMillis() > prev_ms + 1000)
    {
        std::string data = "hello, now = ";
        data += ofToString((int)((prev_ms / 1000) % 255));

        subghz.send(data);

        PRINTLN("send data : size =", data.length());
        PRINTLN("data =", data);

        prev_ms = ofGetElapsedTimeMillis();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
