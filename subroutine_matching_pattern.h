#pragma once
#include "common_types.h"
#include "instruction.h"

enum class SubroutinePatternMatchingType : uint8 {  //Type _t -> Type _res
    None,
    Parameter,
    ParameterRange,
    ParameterSwitch,
    ParameterSwitchStrided,
    Argument,
    ArgumentFunction,
    //add varyfying parameter and argument function.
    ParameterEnd,
    ParameterRangeEnd,
    ParameterSwitchEnd,
    ParameterSwitchStridedEnd,
    ArgumentEnd,
    ArgumentFunctionEnd,
    Next,
    Jump,

    FuncNative,
    MethNative,
    ProcNative,
    FuncTarget,
    MethTarget,
    ProcTarget,
};

struct SubroutinePatternMatching {
    union {
        ValueType parameter;
        struct { //Range
            int skip; // I doubt jump for more then sizeof(ValueType) would arise.
            ValueType lessThen; //  (less - type) > more //  1 | 2 | 3 / 3 |  | 1
            ValueType moreThen;
        };
        struct { //JumpTable
            int stride;
            ValueType lessThen;
            ValueType moreThen;
        };
        void* pointer; //to function, value or varifying function.
    };
    SubroutinePatternMatchingType patternType;

    SubroutinePatternMatching() : patternType(SubroutinePatternMatchingType::None) {}

    SubroutinePatternMatching(SubroutinePatternMatchingType type, void* ptr) :
        patternType(type),
        pointer(ptr) {}

    SubroutinePatternMatching(SubroutinePatternMatchingType type, ValueType equal) :
        patternType(type),
        lessThen(equal) {}

    SubroutinePatternMatching(SubroutinePatternMatchingType type, ValueType lessThen, ValueType moreThen, int par) :
        patternType(type),
        lessThen(lessThen),
        moreThen(moreThen),
        skip(par) {}
};

//struct SubroutineParameterMatching //: std::unordered_map<

using SubroutineParameterMatching = std::unordered_map<uint64, ValueType*>;