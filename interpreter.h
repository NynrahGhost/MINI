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

    struct {
        Table<String, ValueType> id; //void toString(ValueType* memory);
        Table<ValueType, String> name;
        Table<ValueType, size_t> size;
        Table<ValueType, ToStringGlobal> stringGlobal;
        Table<ValueType, ToStringLocal> stringLocal;
        Table<ValueType, Destructor> destructor; //TODO: provide possibility to destroy with target function
    } type;

    struct {
        Table<String, Table<ValueType, Procedure>> prefix;
        Table<String, Table<ValueType, Procedure>> postfix;
        Table<String, Table<ValueTypeBinary, Procedure>> binary;
        Table<ValueTypeBinary, Procedure> coalescing;
    } op;
};

class Program {
protected:
    static String* const EMPTY_STRING;
public:
    Array<Table<String, ValueType*>> data;
    Array<Table<String, ValueType*>> namespaces;
    Module specification;

    struct _stacks {
        Array<Instruction> instructions;
        Array<Array<Instruction>> arrays;

        void drop(size_t amount);
    } stacks;

    struct {
        int32 shift;
        int16 modifier;
        ValueType value;
    } context;
    Span memory;


    Program();
    Program(Array<Table<String, ValueType*>> data);

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