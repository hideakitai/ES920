#pragma once
#ifndef ARDUINO_ES920_UTILS_H
#define ARDUINO_ES920_UTILS_H

#include "Constants.h"

#ifdef ARDUINO
    #include <Arduino.h>
    #define ELAPSED_TIME_MS millis
    #define ES920_SERIAL_BEGIN(s, b) s.begin(b)
    #define ES920_SERIAL_END(s) s.end()
    #define ES920_READ_BYTE stream->read
    #define ES920_WRITE_BYTE stream->write
    #define ES920_WRITE_BYTES stream->write
    #define ES920_STRING_CAST(b) String(b)
    #define ES920_STRING_SIZE(s) s.length()
    #define ES920_STRING_POP_BACK(s) s.remove(s.length() - 1)
    #define ES920_STRING_CLEAR(s) s = ""
    #define ES920_STRING_SUBSTR(s, i, j) s.substring(i, i + j)
    #define ES920_STRING_ERASE(s, i, j) s.remove(i, j)
    #define ES920_STRING_TO_INT(s) s.toInt()
#elif defined (OF_VERSION_MAJOR)
    #include "ofMain.h"
    #define ELAPSED_TIME_MS ofGetElapsedTimeMillis
    #define ES920_SERIAL_BEGIN(s, n, b) s.setup(n, b)
    #define ES920_SERIAL_END(s) s.close()
    #define ES920_READ_BYTE stream->readByte
    #define ES920_WRITE_BYTE stream->writeByte
    #define ES920_WRITE_BYTES stream->writeBytes
    #define ES920_STRING_CAST(b) std::to_string(b)
    #define ES920_STRING_SIZE(s) s.size()
    #define ES920_STRING_POP_BACK(s) s.pop_back()
    #define ES920_STRING_CLEAR(s) s.clear()
    #define ES920_STRING_SUBSTR(s, i, j) s.substr(i, j)
    #define ES920_STRING_ERASE(s, i, j) s.erase(i, j)
    #define ES920_STRING_TO_INT(s) std::stoi(s)
#endif

namespace arduino {
namespace es920 {

    inline void wait(const uint64_t ms)
    {
        uint64_t start_ms = ELAPSED_TIME_MS();
        while (ELAPSED_TIME_MS() < start_ms + ms) ;
    }

    inline bool disableAndReturn(bool& b)
    {
        bool r = b;
        b = false;
        return r;
    }

} // namespace es920
} // namespace arduino

#endif // ARDUINO_ES920_UTILS_H
