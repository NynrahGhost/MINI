#pragma once

#include "compile_options.h"
#include "common_types.h"
#include "instruction.h"
#include "subroutine_matching_pattern.h"
#include <vector>
#include <string>
#include <unordered_map>
//#include "caller.asm"



//Macros for operation modifiers
/*
    M - method (operator)
    A - argument (for unary)
    L - left argument (for binary)
    R - right argument (for binary)
    v - pass by value
    vv - pass by pointer to value
    i - pass by instruction
    ii - pass by pointer to instruction
    d - delete after function have finished execution
*/
#define uMod_ 0
#define uMod_delete (OperationModifier)((int32)OperationModifier::m_del | (int32)OperationModifier::a_del)

#define uMod_M 0
#define uMod_Mv OperationModifier::m_put
#define uMod_Mvv (OperationModifier)((int32)OperationModifier::m_out | (int32)uMod_Mv)
#define uMod_Mi OperationModifier::m_instr
#define uMod_Mii (OperationModifier)((int32)OperationModifier::m_out | (int32)uMod_Mi)
#define uMod_Md OperationModifier::m_del
#define uMod_Mvd (OperationModifier)((int32)OperationModifier::m_del | (int32)uMod_Mv)
#define uMod_Mvvd (OperationModifier)((int32)OperationModifier::m_del | (int32)uMod_Mvv)
#define uMod_Mid (OperationModifier)((int32)OperationModifier::m_del | (int32)uMod_Mi)
#define uMod_Miid (OperationModifier)((int32)OperationModifier::m_del | (int32)uMod_Mii)

#define uMod_A 0
#define uMod_Av OperationModifier::a_put
#define uMod_Avv (OperationModifier)((int32)OperationModifier::a_out | (int32)uMod_Av)
#define uMod_Ai OperationModifier::a_instr
#define uMod_Aii (OperationModifier)((int32)OperationModifier::a_out | (int32)uMod_Ai)
#define uMod_Ad OperationModifier::a_del
#define uMod_Avd (OperationModifier)((int32)OperationModifier::a_del | (int32)uMod_Av)
#define uMod_Avvd (OperationModifier)((int32)OperationModifier::a_del | (int32)uMod_Avv)
#define uMod_Aid (OperationModifier)((int32)OperationModifier::a_del | (int32)uMod_Ai)
#define uMod_Aiid (OperationModifier)((int32)OperationModifier::a_del | (int32)uMod_Aii)

#define uMod_N OperationModifier::native
#define uMod_C(ARG) (OperationModifier)((int32)OperationModifier::concrete | (int32)(uMod_ ## ARG))
#define uMod_N(ARG) (OperationModifier)((int32)OperationModifier::native | (int32)(uMod_ ## ARG))
#define uMod_N(METHOD, ARG) (OperationModifier)((int32)OperationModifier::native | (int32)(uMod_ ## METHOD) | (int32)(uMod_ ## ARG))


#define bMod_ 0
#define bMod_delete (OperationModifier)((int32)OperationModifier::m_del | (int32)OperationModifier::l_del | (int32)OperationModifier::r_del)

#define bMod_M 0
#define bMod_Mv OperationModifier::m_put
#define bMod_Mvv (OperationModifier)((int32)OperationModifier::m_out | (int32)bMod_Mv)
#define bMod_Mi OperationModifier::m_instr
#define bMod_Mii (OperationModifier)((int32)OperationModifier::m_out | (int32)bMod_Mi)
#define bMod_Md OperationModifier::m_del
#define bMod_Mvd (OperationModifier)((int32)OperationModifier::m_del | (int32)bMod_Mv)
#define bMod_Mvvd (OperationModifier)((int32)OperationModifier::m_del | (int32)bMod_Mvv)
#define bMod_Mid (OperationModifier)((int32)OperationModifier::m_del | (int32)bMod_Mi)
#define bMod_Miid (OperationModifier)((int32)OperationModifier::m_del | (int32)bMod_Mii)

#define bMod_L 0
#define bMod_Lv OperationModifier::l_put
#define bMod_Lvv (OperationModifier)((int32)OperationModifier::l_out | (int32)bMod_Lv)
#define bMod_Li OperationModifier::l_instr
#define bMod_Lii (OperationModifier)((int32)OperationModifier::l_out | (int32)bMod_Li)
#define bMod_Ld OperationModifier::l_del
#define bMod_Lvd (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_Lv)
#define bMod_Lvvd (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_Lvv)
#define bMod_Lid (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_Li)
#define bMod_Liid (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_Lii)

#define bMod_R 0
#define bMod_Rv OperationModifier::r_put
#define bMod_Rvv (OperationModifier)((int32)OperationModifier::r_out | (int32)bMod_Rv)
#define bMod_Ri OperationModifier::r_instr
#define bMod_Rii (OperationModifier)((int32)OperationModifier::r_out | (int32)bMod_Ri)
#define bMod_Rd OperationModifier::r_del
#define bMod_Rvd (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_Rv)
#define bMod_Rvvd (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_Rvv)
#define bMod_Rid (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_Ri)
#define bMod_Riid (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_Rii)

#define bMod(ARG) bMod_ ## ARG
#define bMod_rev(ARG) (OperationModifier)((int32)OperationModifier::reverse | (int32) bMod_ ## ARG)
#define bMod_N(ARG) (OperationModifier)((int32)OperationModifier::native | (int32) bMod_ ## ARG)

#define bMod_N(METHOD, LEFT, RIGHT) (OperationModifier)((int32)OperationModifier::native | (int32)(bMod_ ## METHOD) | (int32)(bMod_ ## LEFT) | (int32)(bMod_ ## RIGHT))


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


enum class OperationModifier : int16 {
    none = 0,
    native = 1,
    concrete = 1 << 1,
    m_put = 1 << 2,
    m_instr = 1 << 3,
    m_out = 1 << 4,
    m_del = 1 << 5,
    l_put = 1 << 6, a_put = 1 << 6,
    l_instr = 1 << 7, a_instr = 1 << 7,
    l_out = 1 << 8, a_out = 1 << 8,
    l_del = 1 << 9, a_del = 1 << 9,
    r_put = 1 << 10,
    r_instr = 1 << 11,
    r_out = 1 << 12,
    r_del = 1 << 13,
    reverse = 1 << 14,
};

struct Operation {
    void* pointer;
    ValueType returnType;
    OperationModifier modifier;

    //Operation(OperationModifier mod, void* ptr);
};

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
        Table<String, Table<ValueType, Operation>> prefixForward;
        Table<String, Table<ValueType, Operation>> postfixForward;
        Table<String, Table<ValueTypeBinary, Operation>> binaryForward;

        Table<ValueType, Table<String, Operation>> prefixReverse;
        Table<ValueType, Table<String, Operation>> postfixReverse;
        Table<ValueTypeBinary, Table<String, Operation>> binaryReverse;

        Table<ValueType, Operation> prefixGeneral;
        Table<ValueType, Operation> postfixGeneral;
        Table<ValueTypeBinary, Operation> binaryGeneral;

        Table<ValueTypeBinary, Operation> coalescing;
    } op;

    struct {
        Table<ValueType, Operation> onEnter;
        Table<ValueType, Operation> onExit;
    } context;
};

const size_t memory_init_size = 1024;
const size_t string_buffer_init_size = 1024;

extern "C" Table<String, ValueType*> g_data;
extern "C" thread_local std::istream* g_stream;
extern "C" thread_local charT* g_script;
extern "C" thread_local int g_script_index;
extern "C" thread_local Module* g_specification;
extern "C" thread_local Array<Table<String, ValueType*>*> g_stack_namespace;
extern "C" thread_local Array<Table<String, ValueType*>> g_stack_local;
extern "C" thread_local Array<Instruction> g_stack_context;
extern "C" thread_local Array<Instruction> g_stack_instruction;
extern "C" thread_local Instruction g_instr_slot;
//extern "C" thread_local Array<Array<Instruction>*> g_stack_array;

/*struct _context {
    int32 shift;
    int16 modifier;
    ValueType value;
};*/

//extern "C" thread_local _context g_context;

extern "C" thread_local Span g_val_mem;
extern "C" thread_local Span g_op_mem;

void inline eval_prefix(Operation op, Instruction instruction_r0, Instruction instruction_r1);
void inline eval_postfix(Operation op, Instruction instruction_r0, Instruction instruction_r1, Instruction instruction_r2);
void inline eval_binary(Operation op, Instruction instruction_r0, Instruction instruction_r1, Instruction instruction_r2);
void inline eval_operator_result(Operation op, void* res);
void inline eval_context(Operation op, Instruction instruction_r1, int32 index);

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