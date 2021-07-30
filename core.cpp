#include "core.h"
#include <iostream>

// I don't trust inlines.
#define alloc_guard(SIZE)														\
	if (memory.maxSize - memory.currentSize < SIZE)								\
		if (tmpPtr = (uint8*)realloc(memory.data, memory.maxSize <<= 1))		\
			memory.data = tmpPtr;												\
		else																	\
			return;


namespace Core {
	Module getCore() {
		//if (Core::core != 0)
		//	return *Core::core;
		Module core;
		core.typeId = std::unordered_map<String, ValueType>({	//typeName : typeID
			{T("int64"), ValueType::int64},
			{T("int32"), ValueType::int32},
			{T("int16"), ValueType::int16},
			{T("int8"), ValueType::int8},
			{T("uint64"), ValueType::uint64},
			{T("uint32"), ValueType::uint32},
			{T("uint16"), ValueType::uint16},
			{T("uint8"), ValueType::uint8},
			{T("float128"), ValueType::float128},
			{T("float64"), ValueType::float64},
			{T("float32"), ValueType::float32},
			{T("string"), ValueType::string},
			{T("name"), ValueType::name},
		});
		core.typeName = std::unordered_map<ValueType, String>({	//typeID : typeName
			{ValueType::int64, T("int64")},
			{ValueType::int32, T("int32")},
			{ValueType::int16, T("int16")},
			{ValueType::int8, T("int8")},
			{ValueType::uint64, T("uint64")},
			{ValueType::uint32, T("uint32")},
			{ValueType::uint16, T("uint16")},
			{ValueType::uint8, T("uint8")},
			{ValueType::float128, T("float128")},
			{ValueType::float64, T("float64")},
			{ValueType::float32, T("float32")},
			{ValueType::string, T("string")},
			{ValueType::name, T("name")},
		});
		core.typeSize = std::unordered_map<ValueType, int>({	//typeID : typeSize
			{ValueType::int64, 8},
			{ValueType::int32, 4},
			{ValueType::int16, 2},
			{ValueType::int8, 1},
			{ValueType::uint64, 8},
			{ValueType::uint32, 4},
			{ValueType::uint16, 2},
			{ValueType::uint8, 1},
			{ValueType::float128, 16},
			{ValueType::float64, 8},
			{ValueType::float32, 4},
			{ValueType::string, sizeof(void*)},
			{ValueType::name, sizeof(void*)},
		});
		core.prefix = std::unordered_map<String, SubroutinePatternMatching*>();
		{
			core.prefix[T("%")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				arr[0] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, test);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
			core.prefix[T("*")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				arr[0] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, getValueProcedure);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
			core.prefix[T(">")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				arr[0] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, allArrayInclusive);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
			core.prefix[T("^")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				arr[0] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, allGroupInclusive);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
		}
		core.postfix = std::unordered_map<String, SubroutinePatternMatching*>();
		{
			core.postfix[T("%")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				arr[0] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, test);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
			core.postfix[T(">")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				arr[0] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, allArrayExclusive);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
			core.postfix[T("^")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				arr[0] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, allGroupExclusive);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
			core.postfix[T(":")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[5];
				arr[0] = SubroutinePatternMatching(SubroutinePatternMatchingType::Parameter, ValueType::dict);
				arr[1] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, allContext);
				arr[2] = SubroutinePatternMatching(SubroutinePatternMatchingType::Parameter, ValueType::name);
				arr[3] = SubroutinePatternMatching(SubroutinePatternMatchingType::ProcNative, allContext);
				arr[4] = SubroutinePatternMatching();
				return arr;
			} ();
		}
		core.binary = std::unordered_map<String, SubroutinePatternMatching*>();
		{
			core.binary[T("+")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				//arr[0] = Binary(BinaryType::TypeTypeType | BinaryType::funcNative, &([](int64 l, int64 r) {return l + r;}), ValueType::int64, 0, ValueType::int64, 0, ValueType::int64);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
			core.binary[T(":")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[3];
				//arr[0] = Binary(BinaryType::TypeTypeAll | BinaryType::procNative, getChild, ValueType::dict, 0, ValueType::name, 0, ValueType::all);
				//arr[1] = Binary(BinaryType::TypeTypeAll | BinaryType::procNative, getChild, ValueType::name, 0, ValueType::name, 0, ValueType::all);
				arr[2] = SubroutinePatternMatching();
				return arr;
			} ();
			core.binary[T(",")] = [] {
				SubroutinePatternMatching* arr = new SubroutinePatternMatching[2];
				//arr[0] = Binary(BinaryType::AllAllAll | BinaryType::procNative, comma, ValueType::all, 0, ValueType::all, 0, ValueType::all);
				arr[1] = SubroutinePatternMatching();
				return arr;
			} ();
		}

		//Core::core = new Module(core);

		return core;
	}

	void test(Program& program) {
		program.stackInstructions.erase(program.stackInstructions.cend() - 2);
		if (program.stackInstructions[program.stackInstructions.size() - 2].value == ValueType::string)
			std::cout << (**(String**)(program.memory.data + program.stackInstructions[program.stackInstructions.size() - 2].shift)).c_str() << std::endl;
		else
			std::cout << "Test! " << (int32)program.context.value << "\n";
	}

	void getValueProcedure(Program& program) { // Unordered map saves keys by address
		uint8* memory = program.memory.data + program.stackInstructions[program.stackInstructions.size() - 1].shift;
		uint8* value = (uint8*)(program.data.back()[**(String**)(memory)]);

		int l = program.specification.typeSize[*(ValueType*)value];

		program.stackInstructions[program.stackInstructions.size() - 1].value = *(ValueType*)value;

		value = (uint8*)((ValueType*)value + 1);

		for (int i = 0; i < l; ++i)
			*(memory + i) = *(value + i);

		program.memory.currentSize -= sizeof(void*);
		program.memory.currentSize += l;

		program.stackInstructions.erase(program.stackInstructions.cend() - 2);
	}

	void allArrayInclusive(Program& program) {

	}

	void allArrayExclusive(Program& program) {

	}

	void allGroupInclusive(Program& program) {

	}

	void allGroupExclusive(Program& program) {

	}

	void allContext(Program& program) {

	}

	void comma(Program& program) {

	}


	void getChild(Program& program) {
		String* right = *(String**)(program.memory.data + program.stackInstructions.back().shift);
		program.stackInstructions.pop_back();
		program.stackInstructions.pop_back();
		String* left = *(String**)(program.memory.data + program.stackInstructions.back().shift);

		auto ptr = program.data.back()[*left];

		if (*ptr == ValueType::dict)
		{
			ptr = (**(std::unordered_map<String, ValueType*>**)(ptr + 1))[*right];
			//++ptr;
			//std::unordered_map<String, ValueType*>* map = *(std::unordered_map<String, ValueType*>**)ptr;
			//ptr = (*map)[*right];

			program.stackInstructions.back().value = *ptr;

			program.memory.currentSize -= sizeof(void*) + sizeof(void*) + sizeof(void*);
			for (int i = 0; i < program.specification.typeSize[*ptr]; ++i)
				*(program.memory.data + program.memory.currentSize + i) = *((uint8*)ptr + sizeof(ValueType) + i);
			program.memory.currentSize += program.specification.typeSize[*ptr];

			delete left,
			delete right;
		}
	}
}