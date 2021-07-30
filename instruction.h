#pragma once


enum class InstructionType : uint8 {
    start,
    end,
    separator,
    spacing,

    start_group,
    start_array,
    start_context,
    end_group,
    end_array,
    end_context,
    all_array,
    all_group,
    all_context,

    append_n,
    skip_next,
    skip_after_next,
    skip_n,
    find_value,


    value,
    op,
    context,
    call,
};


#if TYPE_SYSTEM_SIZE == 8
enum class ValueType : uint8
#elif TYPE_SYSTEM_SIZE == 16
enum class type : uint16
#elif TYPE_SYSTEM_SIZE == 32
enum class type : uint32
#elif TYPE_SYSTEM_SIZE == 64
enum class type : uint64
#else
#error Incorrect type system size specified
#endif
{
    none,
    all,
    type,
    specification,
    dll,
    truth, lie,             // Boolean type, but values are stored explicitly (thus it's more efficient then storing in int8).
    int8, int16, int32, int64,
    uint8, uint16, uint32, uint64,
    float16, float32, float64, float128,
    data0, data8, data16, data32,
    ufunction, umethod, uprocedure,
    unfunction, unmethod, unprocedure,
    bfunction, bmethod, bprocedure,
    bnfunction, bnmethod, bnprocedure,
    name,
    string,
    expression,
    link,
    arr,
    dict,
    pointer,                // Address to existing node.
    smartPointer,           // Data that gets destroyed when there are no more references pointing to it
    object,                 // Starts with address of schema it uses.
};

struct ValueLocation {
    int shift;
    ValueType value;
};

struct Instruction {
    InstructionType instr;
    union {
        ValueLocation location;
        struct {
            int shift;
            ValueType value;
        };
        int modifier;
    };

    static Instruction atom(InstructionType type) {
        return Instruction{
            type
        };
    }

    static Instruction pos(InstructionType type, int shift) {
        return Instruction{
            type,
            shift
        };
    }

    static Instruction val(ValueType value, int shift) {
        return Instruction{
            InstructionType::value,
            shift,
            value
        };
    }

    static Instruction context(ValueType value, int shift) {
        return Instruction{
            InstructionType::context,
            shift,
            value
        };
    }
};
