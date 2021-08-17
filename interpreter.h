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
    error_syntax,
};

enum class ProcedureResult {
    success,
    repeat,
};

class Program;  //Defined before Procedure, to break cyclic dependancy

using Procedure = void (*) (Program&);
using Destructor = void (*) (ValueType*);
using ToStringGlobal = String (*) (Program&, ValueType*);
using ToStringLocal = String (*) (Program&, Instruction);

struct Module {
    Table<String, String> metadata;

    Table<String, ValueType> typeId; //void toString(ValueType* memory);
    Table<ValueType, String> typeName;
    Table<ValueType, size_t> typeSize;
    Table<ValueType, ToStringGlobal> typeStringGlobal;
    Table<ValueType, ToStringLocal> typeStringLocal;
    Table<ValueType, Destructor> typeDestructor; //TODO: provide possibility to destroy with target function

    Table<String, Table<ValueType, Procedure>> opPrefix;
    Table<String, Table<ValueType, Procedure>> opPostfix;
    Table<String, Table<ValueTypeBinary, Procedure>> opBinary;
    Table<ValueTypeBinary, Procedure> opCoalescing;
};

class Program {
protected:
    static String* const EMPTY_STRING;
public:
    Array<Table<String, ValueType*>> data;
    Array<Table<String, ValueType*>> namespaces;
    Module specification;

    Array<Instruction> stackInstructions;
    Array<Instruction> stackCalls;
    Array<Array<Instruction>> stackArrays;
    ValueLocation context;
    Span memory;


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

// Involuntary contributors:

// Thanks for compile-time char/wchar conversion, SOLON7! https://habr.com/ru/post/164193/
// Thanks for a tip on "|" overloading to use enums as bitflags, eidolon! https://stackoverflow.com/a/1448478/13975990
// Thanks for 'typedef' for unordered_map, Philipp! https://stackoverflow.com/a/9576473/13975990  
        //Temporary renaming of type, for readability and possible replacement in future.