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

using Procedure = void (*) ();
using Destructor = void (*) (void*);
using ToStringGlobal = String (*) (ValueType*);
using ToStringLocal = String (*) (Instruction);

struct Module {
    Table<String, String> metadata;

    struct {
        Table<String, ValueType> id;
        Table<ValueType, String> name;
        Table<ValueType, size_t> size;
        //Specification may require variableSize field (Currently not belonging to size table means type is varsize)
        Table<ValueType, ToStringGlobal> stringGlobal;
        Table<ValueType, ToStringLocal> stringLocal;
        Table<ValueType, Destructor> destructor; 
        //Specification may require hasDestructor field (Currently not belonging to destructor table means type has no destructor)
    } type;

    struct {
        Table<String, Table<ValueType, Procedure>> prefix;
        Table<String, Table<ValueType, Procedure>> postfix;
        Table<String, Table<ValueTypeBinary, Procedure>> binary;
        Table<ValueTypeBinary, Procedure> coalescing;
    } op;
};


extern "C" Array<Table<String, ValueType*>> g_data;// = Array<Table<String, ValueType*>>(16);
extern "C" Array<Table<String, ValueType*>> g_namespaces;// = Array<Table<String, ValueType*>>(16);
extern "C" Module* g_specification;// = NULL;// = Core::initCore();
extern "C" Array<Instruction> g_stack_instruction;
extern "C" Array<Array<Instruction>> g_stack_array;

struct _context {
    int32 shift;
    int16 modifier;
    ValueType value;
};

extern "C" _context g_context;// = _context{ 0, 0, ValueType::none };

extern "C" Span g_memory;// = Span(1024);

void g_memory_delete_top();
void g_memory_delete_r(size_t index);
void g_memory_delete_span_r(size_t index);
void g_memory_delete_span_r(size_t indexBegin, size_t indexEnd);


Status run(const charT* script);

int main();

// Involuntary contributors:

// Thanks for compile-time char/wchar conversion, SOLON7! https://habr.com/ru/post/164193/
// Thanks for a tip on "|" overloading to use enums as bitflags, eidolon! https://stackoverflow.com/a/1448478/13975990
// Thanks for 'typedef' for unordered_map, Philipp! https://stackoverflow.com/a/9576473/13975990  
        //Temporary renaming of type, for readability and possible replacement in future.