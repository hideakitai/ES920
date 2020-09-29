#pragma once
#ifndef ARX_STRINGUTILS_H
#define ARX_STRINGUTILS_H

#ifdef ARDUINO
#include <Arduino.h>
    #define ARXSTRUTIL_STRING_CAST(b) String(b)
    #define ARXSTRUTIL_STRING_SIZE(s) s.length()
    #define ARXSTRUTIL_STRING_POP_BACK(s) s.remove(s.length() - 1)
    #define ARXSTRUTIL_STRING_CLEAR(s) s = ""
    #define ARXSTRUTIL_STRING_SUBSTR(s, i, j) s.substring(i, i + j)
    #define ARXSTRUTIL_STRING_ERASE(s, i, j) s.remove(i, j)
    #define ARXSTRUTIL_STRING_TO_INT(s) s.toInt()
#elif defined (OF_VERSION_MAJOR)
    #include "ofMain.h"
    #define ARXSTRUTIL_STRING_CAST(b) ofToString(b)
    #define ARXSTRUTIL_STRING_SIZE(s) s.size()
    #define ARXSTRUTIL_STRING_POP_BACK(s) s.pop_back()
    #define ARXSTRUTIL_STRING_CLEAR(s) s.clear()
    #define ARXSTRUTIL_STRING_SUBSTR(s, i, j) s.substr(i, j)
    #define ARXSTRUTIL_STRING_ERASE(s, i, j) s.erase(i, j)
    #define ARXSTRUTIL_STRING_TO_INT(s) std::stoi(s)
#endif
#include "util/ArxTypeTraits/ArxTypeTraits.h"

namespace arx {
namespace str {

#ifdef ARDUINO
    using StringType = String;
#else
    using StringType = std::string;
#endif

    namespace detail
    {
        template <size_t size>
        struct same_size_int;

        template <size_t size>
        using same_size_int_t = typename same_size_int<size>::type;

        template <> struct same_size_int<1> { using type = int8_t; };
        template <> struct same_size_int<2> { using type = int16_t; };
        template <> struct same_size_int<4> { using type = int32_t; };
        template <> struct same_size_int<8> { using type = int64_t; };

        template <typename T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
        struct IntFloatUnion_impl {
            using type = union {
                same_size_int_t<sizeof(T)> x;
                T f;
            };
        };

        template <typename T>
        using IntFloatUnion = typename IntFloatUnion_impl<T>::type;

        template <typename T>
        inline auto to_int(const StringType &intString)
        -> typename std::enable_if<std::is_integral<T>::value, T>::type
        {
            return (T)intString.toInt();
        }
    }

    template <typename T, size_t length = sizeof(T) * 2>
    inline auto to_hex(const T& value, bool b_leading_zeros = true)
    -> typename std::enable_if<std::is_integral<T>::value, StringType>::type
    {
        StringType format;
        if (b_leading_zeros) format = "%0" + ARXSTRUTIL_STRING_CAST(length) + "X";
        else                 format = "%X";
        char hex[length + 1];
        sprintf(hex, format.c_str(), value);
        return ARXSTRUTIL_STRING_CAST(hex);
    }

    template <typename T, size_t length = sizeof(T) * 2>
    inline auto to_hex(const T& value, bool b_leading_zeros = true)
    -> typename std::enable_if<std::is_floating_point<T>::value, StringType>::type
    {
        detail::IntFloatUnion<T> myUnion;
        myUnion.f = value;
        return to_hex(myUnion.x, b_leading_zeros);
    }

    inline int from_hex_to_int(const StringType& intHexString)
    {
        return (int)strtol(intHexString.c_str(), NULL, 16);
    }

    inline char from_hex_to_char(const StringType& charHexString)
    {
        return (char)strtol(charHexString.c_str(), NULL, 16);
    }

    inline float from_hex_to_float(const StringType& floatHexString)
    {
        detail::IntFloatUnion<float> myUnion;
        myUnion.x = from_hex_to_int(floatHexString);
        return myUnion.f;
    }

    inline double from_hex_to_double(const StringType& doubleHexString)
    {
        detail::IntFloatUnion<double> myUnion;
        myUnion.x = from_hex_to_int(doubleHexString);
        return myUnion.f;
    }

    template <typename T>
    inline auto from_dec_to_bcd(const T& n)
    -> typename std::enable_if<std::is_integral<T>::value, size_t>::type
    {
        return n + 6 * (n / 10);
    }

    template <typename T>
    inline auto from_bcd_to_dec(const T& n)
    -> typename std::enable_if<std::is_integral<T>::value, size_t>::type
    {
        return n - 6 * (n >> 4);
    }

    template <typename T>
    inline auto string_length(const T& value)
    -> typename std::enable_if<std::is_integral<T>::value, size_t>::type
    {
        return floor(log10(value) + 1);
    }

    template <typename T>
    inline auto to_string(const T& value, size_t width)
    -> typename std::enable_if<std::is_integral<T>::value, StringType>::type
    {
        StringType format;
        format = "%0" + String(width) + "d";
        const size_t str_len = string_length(value);
        const size_t len = (width > str_len) ? width : str_len;
        char dec[len + 1];
        sprintf(dec, format.c_str(), value);
        return ARXSTRUTIL_STRING_CAST(dec);
    }

    template <typename T>
    inline auto to_string(const T& value)
    -> typename std::enable_if<std::is_integral<T>::value, StringType>::type
    {
        return to_string(value, 0);
    }

    template <typename T>
    inline auto to_string(const T& value, size_t precision, size_t width)
    -> typename std::enable_if<std::is_floating_point<T>::value, StringType>::type
    {
        StringType format;
        format = "%0" + String(width) + "." + String(precision) + "f";
        const size_t str_len = String(value).length();
        const size_t len = (width > str_len) ? width : str_len;
        char dec[len + 1];
        sprintf(dec, format.c_str(), value);
        return ARXSTRUTIL_STRING_CAST(dec);
    }

    template <typename T>
    inline auto to_string(const T& value, size_t precision)
    -> typename std::enable_if<std::is_floating_point<T>::value, StringType>::type
    {
        return to_string(value, precision, 0);
    }


#ifdef ARDUINO
#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L // Have libstdc++11

    #include <vector>

    inline std::vector<StringType> split_string(const StringType& s, const StringType& delim)
    {
        std::vector<StringType> result;
        std::vector<size_t> pos;
        // std::vector<size_t> length;
        pos.emplace_back(0);
        for (size_t i = 0; i < s.length(); ++i)
        {
            if (s.charAt(i) == *(delim.c_str()))
            {
                pos.emplace_back(i + 1);
            }
            else if (s.charAt(i) == '\r')
            {
                pos.emplace_back(i + 1);
                break;
            }
            else if (s.charAt(i) == '\n')
            {
                pos.emplace_back(i + 1);
                break;
            }
        }
        for (size_t i = 0; i < pos.size() - 1; ++i)
        {
            result.push_back(s.substring(pos[i], pos[i + 1] - 1));
        }
        if (!s.endsWith(String('\n')))
        {
            result.push_back(s.substring(pos[pos.size() - 1], s.length()));
        }
        return result; // local value will be moved
    }

#endif
#endif

//    String to_bin(const String& value)
//    {
//        String out("");
//        size_t numBytes = value.length();
//        for(size_t i = 0; i < numBytes; i++) {
//            std::bitset<8> bitBuffer(value.c_str()[i]);
//            out += String(bitBuffer.to_ulong(), BIN);
//        }
//        return out;
//    }
//
//    String to_bin(const char* value) { return toBinary(String(value)); }
//
//    int from_bin_to_int(const String& value)
//    {
//        const size_t intSize = sizeof(int) * 8;
//        std::bitset<intSize> binaryString(value.c_str()[0]);
//        return (int)binaryString.to_ulong();
//    }
//
//    char from_bin_to_char(const String& value)
//    {
//        const size_t charSize = sizeof(char) * 8;
//        std::bitset<charSize> binaryString(value.c_str()[0]);
//        return (char) binaryString.to_ulong();
//    }
//
//    float from_bin_to_float(const String& value)
//    {
//        const size_t floatSize = sizeof(float) * 8;
//        std::bitset<floatSize> binaryString(value.c_str()[0]);
//        union ulongFloatUnion {
//                unsigned long result;
//                float f;
//        } myUFUnion;
//        myUFUnion.result = binaryString.to_ulong();
//        return myUFUnion.f;
//    }
//
//    template <class T>
//    String to_bin(const T& value) {
//        return std::bitset<8 * sizeof(T)>(*reinterpret_cast<const uint64_t*>(&value)).to_string();
//    }

} // namespace str
} // namespace arx


#endif // EMBEDDEDUTILS_CALCULUS_H
