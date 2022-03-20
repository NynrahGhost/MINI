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

#define uMod_ 0
#define uMod_P OperationModifier::a_put
#define uMod_D OperationModifier::a_del
#define uMod_PD (OperationModifier)((int32)OperationModifier::a_put | (int32)uMod_D)
#define uMod_oP (OperationModifier)((int32)OperationModifier::a_out | (int32)uMod_P)
#define uMod_oPD (OperationModifier)((int32)OperationModifier::a_del | (int32)uMod_oP)
#define uMod_N(ARG) (OperationModifier)((int32)OperationModifier::native | (int32) uMod_ ## ARG)
#define uMod_Res(ARG) (OperationModifier)((int32)OperationModifier::r_put | (int32) uMod_ ## ARG)
#define uMod_ResOut(ARG) (OperationModifier)((int32)OperationModifier::r_out | (int32) uMod_Res(ARG))


#define uMod_ 0
#define uMod_A 0
#define uMod_Ap OperationModifier::a_put
#define uMod_Ao OperationModifier::a_out
#define uMod_Ar (OperationModifier)((int32)uMod_Ao | (int32)uMod_Ap)
#define uMod_Ad OperationModifier::a_del
#define uMod_Apd (OperationModifier)((int32)OperationModifier::a_del | (int32)uMod_Ap)
#define uMod_Aod (OperationModifier)((int32)OperationModifier::a_del | (int32)uMod_Ao)
#define uMod_Ard (OperationModifier)((int32)OperationModifier::a_del | (int32)uMod_Ar)
#define uMod_M 0
#define uMod_Mp OperationModifier::m_put
#define uMod_Mo OperationModifier::m_out
#define uMod_Mr (OperationModifier)((int32)uMod_Mo | (int32)uMod_Mp)
#define uMod_Md OperationModifier::m_del
#define uMod_Mpd (OperationModifier)((int32)OperationModifier::m_del | (int32)uMod_Mp)
#define uMod_Mod (OperationModifier)((int32)OperationModifier::m_del | (int32)uMod_Mo)
#define uMod_Mrd (OperationModifier)((int32)OperationModifier::m_del | (int32)uMod_Mr)
#define uMod_N(METHOD, ARG) (OperationModifier)((int32)OperationModifier::native | (int32)(uMod_ ## METHOD) | (int32)(uMod_ ## ARG))



#define bMod_ 0
#define bMod_M 0
#define bMod_L 0
#define bMod_R 0
#define bMod_RevN (OperationModifier)(((int32)OperationModifier::reverse) | (int32)bMod_N)
#define bMod_Lp OperationModifier::l_put
#define bMod_Rp OperationModifier::r_put
#define bMod_Mp OperationModifier::m_put
#define bMod_Lo OperationModifier::l_out
#define bMod_Ro OperationModifier::r_out
#define bMod_Mo OperationModifier::m_out
#define bMod_Ld OperationModifier::l_del
#define bMod_Rd OperationModifier::r_del
#define bMod_Md OperationModifier::m_del
#define bMod_Lr (OperationModifier)((int32)OperationModifier::l_out | (int32)bMod_Lp)
#define bMod_Rr (OperationModifier)((int32)OperationModifier::r_out | (int32)bMod_Rp)
#define bMod_Mr (OperationModifier)((int32)OperationModifier::m_out | (int32)bMod_Mp)
#define bMod_Lpd (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_Lp)
#define bMod_Rpd (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_Rp)
#define bMod_Mpd (OperationModifier)((int32)OperationModifier::m_del | (int32)bMod_Mp)
#define bMod_Lod (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_Lo)
#define bMod_Rod (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_Ro)
#define bMod_Mod (OperationModifier)((int32)OperationModifier::m_del | (int32)bMod_Mo)
#define bMod_Lrd (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_Lr)
#define bMod_Rrd (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_Rr)
#define bMod_Mrd (OperationModifier)((int32)OperationModifier::m_del | (int32)bMod_Mr)
/*#define bMod_LpRp (OperationModifier)((int32)OperationModifier::l_put | (int32)bMod_Rp)
#define bMod_LoRp (OperationModifier)((int32)OperationModifier::l_out | (int32)bMod_LpRp)
#define bMod_LodRp (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_LoRp)
#define bMod_LpRo (OperationModifier)((int32)OperationModifier::r_out | (int32)bMod_LpRp)
#define bMod_LpRod (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_LpRo)
#define bMod_LoRo (OperationModifier)((int32)OperationModifier::l_out | (int32)bMod_LpRo)
#define bMod_LodRo (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_LoRo)
#define bMod_LoRod (OperationModifier)((int32)OperationModifier::r_del | (int32)bMod_LoRo)
#define bMod_LodRod (OperationModifier)((int32)OperationModifier::l_del | (int32)bMod_LoRod)*/
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
    native = 1,
    concrete = 1 << 1,
    m_put = 1 << 2,
    m_out = 1 << 3,
    m_del = 1 << 4,
    l_put = 1 << 5, a_put = 1 << 5,
    l_out = 1 << 6, a_out = 1 << 6,
    l_del = 1 << 7, a_del = 1 << 7,
    r_put = 1 << 8,
    r_out = 1 << 9,
    r_del = 1 << 10,
    reverse = 1 << 11,
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