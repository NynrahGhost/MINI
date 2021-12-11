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
    error_syntax,
    error_invalid_op,
    error_no_context_onEnter,
    error_no_context_onExit,
};

using Procedure = void (*) ();
using Method = void (*) (String*);
using Destructor = void (*) (void*);
using DestructorProcedure = void (*) (void*, Instruction&);
using ToStringGlobal = String (*) (ValueType*);
using ToStringLocal = String (*) (Instruction);

struct Module {
    Table<String, String> metadata;

    struct {
        uint32 count;
        Table<String, ValueType> id;
        Table<ValueType, String> name;
        uint8* size;
            //Table<ValueType, size_t> size;
        //Specification may require variableSize field (Currently not belonging to size table means type is varsize)
        Table<ValueType, ToStringGlobal> stringGlobal;
        Table<ValueType, ToStringLocal> stringLocal;
        Table<ValueType, DestructorProcedure> destructor;
        Table<ValueType, Destructor> destructorGlobal;
        //Specification may require hasDestructor field (Currently not belonging to destructor table means type has no destructor)
    } type;

    struct {
        Table<String, Table<ValueType, Procedure>> prefixForward;
        Table<String, Table<ValueType, Procedure>> postfixForward;
        Table<String, Table<ValueTypeBinary, Procedure>> binaryForward;

        Table<ValueType, Table<String, Procedure>> prefixReverse;
        Table<ValueType, Table<String, Procedure>> postfixReverse;
        Table<ValueTypeBinary, Table<String, Procedure>> binaryReverse;

        Table<ValueType, Procedure> prefixGeneral;
        Table<ValueType, Procedure> postfixGeneral;
        Table<ValueTypeBinary, Procedure> binaryGeneral;

        Table<ValueTypeBinary, Procedure> coalescing;
    } op;

    struct {
        Table<ValueType, Procedure> onEnter;
        Table<ValueType, Procedure> onExit;
    } context;
};

const size_t memory_init_size = 1024;
const size_t string_buffer_init_size = 1024;

extern "C" Table<String, ValueType*> g_data;
extern "C" thread_local std::istream* stream;
extern "C" thread_local charT* script;
extern "C" thread_local int scriptIndex;
extern "C" thread_local Module* g_specification;
extern "C" thread_local Array<Table<String, ValueType*>*> g_stack_namespace;
extern "C" thread_local Array<Table<String, ValueType*>> g_stack_local;
extern "C" thread_local Array<Instruction> g_stack_context;
extern "C" thread_local Array<Instruction> g_stack_instruction;
//extern "C" thread_local Array<Array<Instruction>*> g_stack_array;

/*struct _context {
    int32 shift;
    int16 modifier;
    ValueType value;
};*/

//extern "C" thread_local _context g_context;

extern "C" thread_local Span g_memory;

void g_memory_delete_top();
void g_memory_delete_r(size_t index);
void g_memory_delete_span_r(size_t index);
void g_memory_delete_span_r(size_t indexBegin, size_t indexEnd);


Status run();

int main();

// Involuntary contributors:

// Thanks for compile-time char/wchar conversion, SOLON7! https://habr.com/ru/post/164193/
// Thanks for a tip on "|" overloading to use enums as bitflags, eidolon! https://stackoverflow.com/a/1448478/13975990
// Thanks for 'typedef' for unordered_map, Philipp! https://stackoverflow.com/a/9576473/13975990  
        //Temporary renaming of type, for readability and possible replacement in future.