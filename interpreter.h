#pragma once

#include "compile_options.h"
#include "common_types.h"
#include "instruction.h"
#include "subroutine_matching_pattern.h"
#include <vector>
#include <string>
#include <unordered_map>
//#include "caller.asm"


//extern "C" int sayHello();

enum class Status
{
    success,
    error_memory_allocation,
    error_string_missing_closing_quote,
    error_operator_name_exceeded,
    error_syntax
};

struct Module {
    std::unordered_map<String, ValueType> typeId;
    std::unordered_map<ValueType, String> typeName;
    std::unordered_map<ValueType, int> typeSize;

    std::unordered_map<String, SubroutinePatternMatching*> prefix;
    std::unordered_map<String, SubroutinePatternMatching*> postfix;
    std::unordered_map<String, SubroutinePatternMatching*> binary;
};

class Program {
public:
    std::vector<std::unordered_map<String, ValueType*>> data;
    std::vector<std::unordered_map<String, ValueType*>> namespaces;
    Module specification;

    std::vector<Instruction> stackInstructions;
    std::vector<Instruction> stackCalls;
    std::vector<std::vector<Instruction>> stackArrays;
    ValueLocation context;
    struct {
        uint8* data = 0;
        int currentSize = 0;
        int maxSize = 0;
    } memory;


    Program();
    Program(std::vector<std::unordered_map<String, ValueType*>> data);
    Status run(const charT* script);

    //template<typename T> T pop();
    //template<> int32 pop<int32>();
};


int main();

void print(ValueType* memory);

uint32 test0(uint16 arg0, uint32 arg1, uint16 arg2, float32 arg3, float64 arg4);


class StringDictionary {
    void* _arr;

};



// Involuntary contributors:

// Thanks for compile-time char/wchar conversion, SOLON7! https://habr.com/ru/post/164193/
// Thanks for a tip on "|" overloading to use enums as bitflags, eidolon! https://stackoverflow.com/a/1448478/13975990