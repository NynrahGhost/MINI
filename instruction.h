#include"common_types.h"
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
    ignore_separator,
    ignore_array_start,
    find_value,


    value,
    op,
    context,
    call,
    error,
};



enum class ValueType : 
#if TYPE_SYSTEM_SIZE == 8
    uint8
#elif TYPE_SYSTEM_SIZE == 16
    uint16
#elif TYPE_SYSTEM_SIZE == 32
    uint32
#elif TYPE_SYSTEM_SIZE == 64
    uint64
#else
#error Incorrect type system size specified
#endif
{
    none,
    all,
    instruction,
    type,
    truth, lie,             // Boolean type, but values are stored explicitly (thus it's more efficient then storing in int8).
    int8, int16, int32, int64,
    uint8, uint16, uint32, uint64,
    float16, float32, float64, float128,
    data0, data8, data16, data32,
    name, string, expression, link,
    parameter_pattern,
    ufunction, umethod, uprocedure,
    unfunction, unmethod, unprocedure,
    bfunction, bmethod, bprocedure,
    bnfunction, bnmethod, bnprocedure,
    tuple,
    dict,
    pointer,
    autoptr,           // Alike automatic storage, gets freeed when exits stack.
    reference, autoref,
    smartPointer,           // Data that gets destroyed when there are no more references pointing to it.
    specification,
    dll,
};

struct ValueTypeUnary {
    ValueType type;

    void* operator new(size_t size) {
        return malloc(size);
    }
};

struct ValueTypeBinary {
    ValueType left;
    ValueType right;

    ValueTypeBinary(ValueType t_left, ValueType t_right) {
        left = t_left;
        right = t_right;
    }

    bool operator==(const ValueTypeBinary& t_values) {
        return (this->left == t_values.left) && (this->right == t_values.right);
    }

    friend bool operator==(const ValueTypeBinary& t_left, const ValueTypeBinary& t_right)
    {
        return (t_left.left == t_right.left) && (t_left.right == t_right.right);
    }
};


template <>
class std::hash<ValueTypeBinary>
{
public:
    size_t operator()(const ValueTypeBinary& t_types) const
    {
        return (uint64)t_types.left + ((uint64)t_types.right << sizeof(ValueType));
    }
    /*
    size_t operator()(const ValueTypeBinary& t_left, const ValueTypeBinary& t_right) const
    {
        return (t_left.left == t_right.left) && (t_left.right == t_right.right);
    }

    friend bool operator==(const ValueTypeBinary& t_left, const ValueTypeBinary& t_right)
    {
        return (t_left.left == t_right.left) && (t_left.right == t_right.right);
    }*/
};



struct ValueLocation {
    ValueType value;
    int shift;
};

struct Instruction {
    InstructionType instr;
    ValueType value;
#if TYPE_SYSTEM_SIZE == 8
    int16 modifier;
#elif TYPE_SYSTEM_SIZE == 16
    int8 modifier;
#endif
    int32 shift;

    inline static Instruction atom(InstructionType type) {
        return Instruction{
            type
        };
    }

    inline static Instruction pos(InstructionType type, int shift) {
        return Instruction{
            type,
            ValueType::none,
            0,
            shift
        };
    }

    inline static Instruction val(ValueType value, int shift) {
        return Instruction{
            InstructionType::value,
            value,
            0,
            shift,
        };
    }

    inline static Instruction context(ValueType value, int shift) {
        return Instruction{
            InstructionType::context,
            value,
            0,
            shift,
        };
    }

    inline static Instruction call(ValueType value, int shift) {
        return Instruction{
            InstructionType::call,
            value,
            0,
            shift,
        };
    }

    template<typename _Value>
    static ValueType* newValue(ValueType t_type, _Value t_value) {
        auto arr = (ValueType*)new uint8[sizeof(ValueType) + sizeof(_Value)];
        arr[0] = t_type;
        *(_Value*)(arr+1) = t_value;
        return arr;
    }
};


template <>
class std::hash<Table<ValueType, ValueType*>>
{
public:
    size_t operator()(const Table<ValueType, ValueType*>& t_table) const
    {
        return (size_t)(uintptr_t)&t_table;
    }
};
