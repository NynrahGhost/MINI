#include "core.h"
#include <iostream>
#include <libloaderapi.h>

//Unary function
#define uFun(LEFT_TYPE, FUNCTION) \
(*table)[ValueType::LEFT_TYPE] = Instruction::newValue(ValueType::unprocedure, FUNCTION);

//Binary function
#define bFun(LEFT_TYPE, RIGHT_TYPE, FUNCTION) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = Instruction::newValue(ValueType::unprocedure, FUNCTION);

///Binary function interface
#define bFunInterface(LEFT_TYPE, LEFT_CLASS, RIGHT_TYPE, RIGHT_CLASS, RESULT_TYPE, RESULT_CLASS, FUNCTION) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = Instruction::newValue(ValueType::unprocedure, binaryFunctionInterface<LEFT_CLASS, RIGHT_CLASS, RESULT_CLASS, ValueType::RESULT_TYPE, FUNCTION>);

///Binary function interface with matching ValueType and typename
#define bFunInterfaceMatch(LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNCTION) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = Instruction::newValue(ValueType::unprocedure, binaryFunctionInterface<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, ValueType::RESULT_TYPE, FUNCTION>);


namespace Core {
	Module getCore() {
		//if (Core::core != 0)
		//	return *Core::core;
		Module core;
		core.typeId = Table<String, ValueType>({	//typeName : typeID
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
			{T("array"), ValueType::arr},
			{T("expression"), ValueType::expression},
		});
		core.typeName = Table<ValueType, String>({	//typeID : typeName
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
			{ValueType::arr, T("array")},
			{ValueType::expression, T("expression")},
		});
		core.typeSize = Table<ValueType, int>({	//typeID : typeSize
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
			{ValueType::arr, 0},
			{ValueType::expression, sizeof(void*)},
		});
		core.prefix = Table<String, Table<ValueType, ValueType*>>();
		{
			Table<ValueType, ValueType*>* table;

			table = &core.prefix[T("%")];
			uFun(all, test);

			table = &core.prefix[T("*")];
			uFun(all, getValueProcedure);

			table = &core.prefix[T(">")];
			uFun(all, allArrayInclusive);

			table = &core.prefix[T("^")];
			uFun(all, allGroupInclusive);
		}
		core.postfix = Table<String, Table<ValueType, ValueType*>>();
		{
			Table<ValueType, ValueType*>* table;

			table = &core.postfix[T("%")];
			uFun(all, test);
			
			table = &core.postfix[T("*")];
			uFun(all, getValueProcedure);

			table = &core.postfix[T(">")];
			uFun(all, allArrayExclusive);

			table = &core.postfix[T("^")];
			uFun(all, allGroupExclusive);

			table = &core.postfix[T(":")];
			uFun(dict, allContext);
			uFun(name, allContext);
		}
		core.binary = Table<String, Table<ValueTypeBinary, ValueType*>>();
		{
			Table<ValueTypeBinary, ValueType*>* table;

			table = &core.binary[T("")];
			bFun(parameter_pattern, arr, invokeFunction);
			bFun(name, arr, invokeFunction);

			/*(*table)[ValueTypeBinary(ValueType::uprocedure, ValueType::arr)] = Instruction::newValue(ValueType::uprocedure, invokeFunction);
			(*table)[ValueTypeBinary(ValueType::ufunction, ValueType::arr)] = Instruction::newValue(ValueType::ufunction, invokeFunction);
			(*table)[ValueTypeBinary(ValueType::umethod, ValueType::arr)] = Instruction::newValue(ValueType::umethod, invokeFunction);
			(*table)[ValueTypeBinary(ValueType::unprocedure, ValueType::arr)] = Instruction::newValue(ValueType::unprocedure, invokeFunction);
			(*table)[ValueTypeBinary(ValueType::unfunction, ValueType::arr)] = Instruction::newValue(ValueType::unfunction, invokeFunction);
			(*table)[ValueTypeBinary(ValueType::unmethod, ValueType::arr)] = Instruction::newValue(ValueType::unmethod, invokeFunction);*/

			table = &core.binary[T("+")];
			bFunInterfaceMatch(int64, int64, int64, add);
			bFunInterfaceMatch(float64, int64, float64, add);
			bFunInterfaceMatch(int64, float64, float64, add);
			bFunInterfaceMatch(float64, float64, float64, add);

			table = &core.binary[T("-")];
			bFunInterfaceMatch(int64, int64, int64, sub);
			bFunInterfaceMatch(float64, int64, float64, sub);
			bFunInterfaceMatch(int64, float64, float64, sub);
			bFunInterfaceMatch(float64, float64, float64, sub);

			table = &core.binary[T("*")];
			bFunInterfaceMatch(int64, int64, int64, mul);
			bFunInterfaceMatch(float64, int64, float64, mul);
			bFunInterfaceMatch(int64, float64, float64, mul);
			bFunInterfaceMatch(float64, float64, float64, mul);

			table = &core.binary[T("/")];
			bFunInterfaceMatch(int64, int64, int64, div);
			bFunInterfaceMatch(float64, int64, float64, div);
			bFunInterfaceMatch(int64, float64, float64, div);
			bFunInterfaceMatch(float64, float64, float64, div);

			table = &core.binary[T("=")];
			bFun(name, all, assign);
			bFun(pointer, all, assign);

			table = &core.binary[T(".")];
			bFun(int64, int64, (createFloat<int64, int64, float64, ValueType::float64>));

			table = &core.binary[T(":")];
			bFun(name, name, getChild);
			bFun(dict, name, getChild);

			table = &core.binary[T(",")];
			bFun(all, all, comma);
		}

		return core;
	}

	void test(Program& program) {
		if (program.stackInstructions[program.stackInstructions.max_index].instr == InstructionType::value) {
			std::cout << program.specification.typeName.at(program.stackInstructions[program.stackInstructions.max_index].value) << std::endl;
			//program.stackInstructions.erase(program.stackInstructions.cend() - 2);
		}

		if (program.stackInstructions[program.stackInstructions.max_index - 2].instr == InstructionType::value) {
			std::cout << program.specification.typeName.at(program.stackInstructions[program.stackInstructions.max_index - 2].value) << std::endl;
			//program.stackInstructions.erase(program.stackInstructions.cend() - 2);
		} else {
			std::cout << program.specification.typeName.at(program.stackInstructions[program.stackInstructions.max_index - 1].value) << std::endl;
			//program.stackInstructions.erase(program.stackInstructions.cend() - 2);
		}
		/*
		program.stackInstructions.erase(program.stackInstructions.cend() - 2);
		switch (program.stackInstructions[program.stackInstructions.size() - 2].value)
		{
		case ValueType::string:
			std::cout << (**(String**)(program.memory.data + program.stackInstructions[program.stackInstructions.size() - 2].shift)).c_str() << std::endl;
		default:
			//std::cout << "Test! " << (int32)program.context.value << "\n";
			std::cout << program.specification.typeName.at(program.stackInstructions[program.stackInstructions.size() - 2].value) << std::endl;
		}*/
	}


	void invokeFunction(Program& program) {
		Instruction parameter = program.stackInstructions.get_r(0);
		Instruction function = program.stackInstructions.get_r(2);

		if (program.context.value == ValueType::dll)
		{
			GetProcAddress(program.memory.get<HMODULE>(program.context.shift), program.memory.get<String>(function.shift).c_str());
			program.stackArrays.get_r(parameter.modifier).content;
		}

		program.stackInstructions.max_index -= 2;

																									//TODO: add a function to add value
		program.stackInstructions.add(Instruction::call(parameter.value, parameter.shift));	//TODO: make a 'call' instruction that specifies change of executable string
		program.stackInstructions.add(Instruction::context(parameter.value, parameter.shift));


	}

	void invokeNativeProcedure(Program& program) {

	}


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType type, _TypeResult (*function) (_TypeLeft, _TypeRight)>
	void binaryFunctionInterface(Program& program) {
		_TypeLeft left = program.memory.get<_TypeLeft>(program.stackInstructions.get_r(2).shift);
		_TypeRight right = program.memory.get<_TypeRight>(program.stackInstructions.get_r(0).shift);

		program.memory.max_index -= sizeof(_TypeLeft);
		program.memory.max_index -= sizeof(void*);
		program.memory.max_index -= sizeof(_TypeRight);

		program.memory.add<_TypeResult>(((_TypeResult(*) (_TypeLeft, _TypeRight))function)(left, right));

		program.stackInstructions.max_index -= 2;
		program.stackInstructions.at_r(0).value = type;
	}


	void assign(Program& program) {
		auto left = program.stackInstructions.get_r(2);
		auto right = program.stackInstructions.get_r(0);

		ValueType* ptr = (ValueType*)malloc(sizeof(ValueType) + program.specification.typeSize[right.value]);
		*ptr = right.value;
		memcpy(ptr + 1, program.memory.content + right.shift, program.specification.typeSize[right.value]);

		program.namespaces.get_r(0)[program.memory.get<String>(left.shift)] = ptr;
	}


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType _valueType>
	void createFloat(Program& program) {
		_TypeResult number = 0;
		_TypeLeft left = program.memory.get<_TypeLeft>(program.stackInstructions.get_r(2).shift);
		_TypeRight right = program.memory.get<_TypeLeft>(program.stackInstructions.get_r(0).shift);

		if (right == 0)
			number = left * 1.0;
		else
			number = left * 1.0 + ((right*1.0) / pow(10, floor(log10(right))+1));

		program.memory.max_index -= sizeof(void*) + sizeof(_TypeLeft) + sizeof(_TypeRight);

		program.stackInstructions.max_index -= 2;
		program.stackInstructions.at_r(0).value = _valueType;
		program.memory.add<_TypeResult>(number);
	}

	void getValueProcedure(Program& program) { // Unordered map saves keys by address
		/*uint8* memory = program.memory.data + program.stackInstructions[program.stackInstructions.max_index].shift;
		uint8* value = (uint8*)(program.data.at_r(0)[**(String**)(memory)]);

		int l = program.specification.typeSize[*(ValueType*)value];

		program.stackInstructions[program.stackInstructions.max_index].value = *(ValueType*)value;

		value = (uint8*)((ValueType*)value + 1);

		for (int i = 0; i < l; ++i)
			*(memory + i) = *(value + i);

		program.memory.currentSize -= sizeof(void*);
		program.memory.currentSize += l;

		program.stackInstructions.at_r(1) = program.stackInstructions.at_r(0);
		program.stackInstructions.max_index -= 1;*/
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
		/*String* right = *(String**)(program.memory.data + program.stackInstructions.at_r(0).shift);
		program.stackInstructions.max_index -= 2;
		String* left = *(String**)(program.memory.data + program.stackInstructions.at_r(0).shift);

		auto ptr = program.data.at_r(0)[*left];

		if (*ptr == ValueType::dict)
		{
			ptr = (**(std::unordered_map<String, ValueType*>**)(ptr + 1))[*right];
			//++ptr;
			//std::unordered_map<String, ValueType*>* map = *(std::unordered_map<String, ValueType*>**)ptr;
			//ptr = (*map)[*right];

			program.stackInstructions.at_r(0).value = *ptr;

			program.memory.currentSize -= sizeof(void*) + sizeof(void*) + sizeof(void*);
			for (int i = 0; i < program.specification.typeSize[*ptr]; ++i)
				*(program.memory.data + program.memory.currentSize + i) = *((uint8*)ptr + sizeof(ValueType) + i);
			program.memory.currentSize += program.specification.typeSize[*ptr];

			delete left,
			delete right;
		}*/
	}
}