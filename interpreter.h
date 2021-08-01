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
    Table<String, ValueType> typeId;
    Table<ValueType, String> typeName;
    Table<ValueType, int> typeSize;

    Table<String, SubroutinePatternMatching*> prefix;
    Table<String, SubroutinePatternMatching*> postfix;
    Table<String, SubroutinePatternMatching*> binary;
};

class Program {
public:
    Array<Table<String, ValueType*>> data;
    Array<Table<String, ValueType*>> namespaces;
    Module specification;

    Array<Instruction> stackInstructions;
    Array<Instruction> stackCalls;
    Array<Array<Instruction>> stackArrays;
    ValueLocation context;
    struct {
        uint8* data = 0;
        int currentSize = 0;
        int maxSize = 0;
    } memory;


    Program();
    Program(Array<Table<String, ValueType*>> data);

    /*~Program() {
        data.~Array();
        namespaces.~Array();

        stackInstructions.~Array();
        stackCalls.~Array();
        stackArrays.~Array();
    }*/

    Status run(const charT* script);

    //template<typename T> T pop();
    //template<> int32 pop<int32>();
};


int main();

void print(ValueType* memory);

uint32 test0(uint16 arg0, uint32 arg1, uint16 arg2, float32 arg3, float64 arg4);



// Involuntary contributors:

// Thanks for compile-time char/wchar conversion, SOLON7! https://habr.com/ru/post/164193/
// Thanks for a tip on "|" overloading to use enums as bitflags, eidolon! https://stackoverflow.com/a/1448478/13975990
// Thanks for 'typedef' for unordered_map, Philipp! https://stackoverflow.com/a/9576473/13975990  
        //Temporary renaming of type, for readability and possible replacement in future.