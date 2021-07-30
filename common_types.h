#pragma once
#include <string>

typedef char int8;
typedef short int int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef float float32;
typedef double float64;
typedef long double float128;


#if ENCODING == 8
typedef std::basic_string<char, std::char_traits<char>, std::allocator<char>> String;
typedef char charT;
#define T(STRING) STRING

#elif ENCODING == 16
//typedef std::basic_string<char16_t, std::char_traits<char16_t>, std::allocator<char16_t>> String;
typedef std::u16string String;
typedef char16_t charT;
#define T(STRING) u##STRING

#elif ENCODING == 32
typedef std::basic_string<char32_t, std::char_traits<char32_t>, std::allocator<char32_t>> String;
typedef char32_t charT;
#define T(STRING) U##STRING

#else
#error Incorrect encoding size specified
#endif