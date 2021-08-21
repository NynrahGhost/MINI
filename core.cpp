#include "core.h"
#include <iostream>
//#include <libloaderapi.h>

//String representation of heap memory
#define toStrGlobal(TYPE, FUNCTION) \
core.type.stringGlobal[ValueType::TYPE] = FUNCTION;

//String representation of stack memory
#define toStrLocal(TYPE, FUNCTION) \
core.type.stringLocal[ValueType::TYPE] = FUNCTION;

//Destructor
#define destr(TYPE, FUNCTION) \
core.type.destructor[ValueType::TYPE] = FUNCTION;

//Unary function
#define uFun(LEFT_TYPE, FUNCTION) \
(*table)[ValueType::LEFT_TYPE] = FUNCTION;

//Binary function
#define bFun(LEFT_TYPE, RIGHT_TYPE, FUNCTION) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = FUNCTION;

//Binary function interface
#define bFunInterface(LEFT_TYPE, LEFT_CLASS, RIGHT_TYPE, RIGHT_CLASS, RESULT_TYPE, RESULT_CLASS, FUNCTION) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = binaryFunctionInterface<LEFT_CLASS, RIGHT_CLASS, RESULT_CLASS, ValueType::RESULT_TYPE, FUNCTION>;

//Binary function interface with matching ValueType and typename
#define bFunInterfaceMatch(LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNCTION) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = binaryFunctionInterface<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, ValueType::RESULT_TYPE, FUNCTION>;


namespace Core {
	Module initCore() {
		//if (Core::core != 0)
		//	return *Core::core;
		Module core;
		core.type.id = Table<String, ValueType>({	//type.name : typeID
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
		core.type.name = Table<ValueType, String>({	//typeID : type.name
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
		core.type.size = Table<ValueType, size_t>({	//typeID : type.size
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
			{ValueType::unprocedure, sizeof(void*)}, //TODO: void* for 'all'
		});
		core.type.stringGlobal = Table<ValueType, ToStringGlobal>();
		{
			toStrGlobal(all, toStringGlobal);
		}
		core.type.stringLocal = Table<ValueType, ToStringLocal>();
		{
			toStrLocal(all, toStringLocal);
			toStrLocal(arr, toStringLocalArray);
		}
		core.type.destructor = Table<ValueType, Destructor>();
		{
			destr(all, (Destructor)free); //TODO: Fill with string, table and stuff
		}
		core.op.prefix = Table<String, Table<ValueType, Procedure>>();
		{
			Table<ValueType, Procedure>* table;

			table = &core.op.prefix[T("?")];
			uFun(all, conditional);
			uFun(truth, conditionalTrue);
			uFun(lie, conditionalFalse);

			table = &core.op.prefix[T(",")];
			uFun(all, commaPrefix);

			table = &core.op.prefix[T("%")];
			uFun(all, test);

			table = &core.op.prefix[T("&")];
			uFun(name, (getReference<1>));

			table = &core.op.prefix[T("*")];
			uFun(name, (getValue<1>));

			table = &core.op.prefix[T(">")];
			uFun(all, allArrayInclusive);

			table = &core.op.prefix[T("^")];
			uFun(int64, getNamespace);

			table = &core.op.prefix[T("@")];
			uFun(int64, atContextByIndex);
			uFun(name, atContextByName);
			uFun(string, atContextByName);
			uFun(arr, renameArrayContext);
		}
		core.op.postfix = Table<String, Table<ValueType, Procedure>>();
		{
			Table<ValueType, Procedure>* table;

			table = &core.op.postfix[T("%")];
			uFun(all, test);

			table = &core.op.postfix[T(",")];
			uFun(all, commaPostfix);
			
			table = &core.op.postfix[T("*")];
			uFun(all, getValueProcedure);

			table = &core.op.postfix[T(">")];
			uFun(all, allArrayExclusive);

			table = &core.op.postfix[T("^")];
			uFun(all, allGroupExclusive);

			table = &core.op.postfix[T(":")];
			uFun(dict, allContext);
			uFun(name, allContext);
		}

		core.op.binary = Table<String, Table<ValueTypeBinary, Procedure>>();
		{
			Table<ValueTypeBinary, Procedure>* table;

			/*table = &core.binary[T("")];
			bFun(name, arr, (getValue<2>));
			bFun(reference, arr, (getValue<2>));
			bFun(unprocedure, arr, invokeProcedure);
			bFun(parameter_pattern, arr, invokeFunction);
			bFun(unfunction, arr, invokeNativeFunction);
			*/

			table = &core.op.binary[T("@")];
			bFun(unfunction, arr, callWithContext);

			table = &core.op.binary[T(",")];
			bFun(all, all, commaBinary);

			table = &core.op.binary[T("+")];
			bFunInterfaceMatch(int64, int64, int64, add);
			bFunInterfaceMatch(float64, int64, float64, add);
			bFunInterfaceMatch(int64, float64, float64, add);
			bFunInterfaceMatch(float64, float64, float64, add);

			table = &core.op.binary[T("-")];
			bFunInterfaceMatch(int64, int64, int64, sub);
			bFunInterfaceMatch(float64, int64, float64, sub);
			bFunInterfaceMatch(int64, float64, float64, sub);
			bFunInterfaceMatch(float64, float64, float64, sub);

			table = &core.op.binary[T("*")];
			bFunInterfaceMatch(int64, int64, int64, mul);
			bFunInterfaceMatch(float64, int64, float64, mul);
			bFunInterfaceMatch(int64, float64, float64, mul);
			bFunInterfaceMatch(float64, float64, float64, mul);

			table = &core.op.binary[T("/")];
			bFunInterfaceMatch(int64, int64, int64, div);
			bFunInterfaceMatch(float64, int64, float64, div);
			bFunInterfaceMatch(int64, float64, float64, div);
			bFunInterfaceMatch(float64, float64, float64, div);

			table = &core.op.binary[T("=")];
			bFun(name, all, assign);
			bFun(pointer, all, assign);

			table = &core.op.binary[T(".")];
			bFun(int64, int64, (createFloat<int64, int64, float64, ValueType::float64>));

			table = &core.op.binary[T(":")];
			bFun(name, name, getChild);
			bFun(dict, name, getChild);
		}

		core.op.coalescing = Table<ValueTypeBinary, Procedure>();
		{
			Table<ValueTypeBinary, Procedure>* table = &core.op.coalescing;

			bFun(name, all, (getValue<1>)); //arr
			bFun(reference, arr, (getValue<1>));
			bFun(unprocedure, all, invokeProcedure); //arr
			bFun(parameter_pattern, arr, invokeFunction);
			bFun(unfunction, arr, invokeNativeFunction);
		}

		return core;
	}

	Table<String, ValueType*> initCoreData() {
		Table<String, ValueType*> data = Table<String, ValueType*>();
		ValueType* ptr;

		ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*)); //TODO: memory check
		*ptr = ValueType::unprocedure;
		*(Procedure*)(ptr + 1) = print;
		data[T("print")] = ptr;

		return data;
	}

	String fromInt(int64 num) {
		String result;
		bool negative = num < 0;
		while (num != 0) {
			result.push_back((charT)((num % 10) + T('0')));
			num /= 10;
		}
		if (negative)
			result.push_back('-');
		std::reverse(result.rbegin(), result.rend());
		return result;
	}
	String fromUint(uint64 num) {
		String result;
		while ((num / 10) != 0) {
			result.push_back((charT)((num % 10) + T('0')));
			num /= 10;
		}
		std::reverse(result.rbegin(), result.rend());
		return result;
	}

	String toStringGlobal(Program& program, ValueType* value) {
		String result = String();

		void* tmp;

		switch (*value) {
		case ValueType::int8:
			return fromInt(*(int8*)(value + 1));
		case ValueType::int16:
			return fromInt(*(int16*)(value + 1));
		case ValueType::int32:
			return fromInt(*(int32*)(value + 1));
		case ValueType::int64:
			return fromInt(*(int64*)(value + 1));
		case ValueType::uint8:
			return fromInt(*(uint8*)(value + 1));
		case ValueType::uint16:
			return fromInt(*(uint16*)(value + 1));
		case ValueType::uint32:
			return fromInt(*(uint32*)(value + 1));
		case ValueType::uint64:
			return fromInt(*(uint64*)(value + 1));
		case ValueType::string:
		case ValueType::name:
		case ValueType::link:
			return *(String*)(value + 1);
		case ValueType::dict:
			result.append(T("{\n"));
			for (auto var : *(Table<String, ValueType*>*)(value + 1))
			{
				result.append(var.first);
				result.append(T(" : "));
				result.append(toStringGlobal(program, var.second));
				result.append(T("; "));
			}
			result.append(T("}"));
			return result;
		case ValueType::arr:
			result.append(T("["));
			for (int i = 0; i < (*(Array<ValueType*>*)(value + 1)).max_index; ++i)
			{
				result.append(toStringGlobal(program, (*(Array<ValueType*>*)(value + 1)).get(i)));
				result.append(T("; "));
			}
			result.append(T("]"));
			return result;
		default:
			result.append((charT*)(value + 1), program.specification.type.size[*value]);
			return result;
		}
	}

	String toStringLocal(Program& program, Instruction instruction) {
		String result = String();

		void* tmp;

		switch (instruction.value) {
		case ValueType::int8:
			return fromInt(*(int8*)(program.memory.content + instruction.shift));
		case ValueType::int16:
			return fromInt(*(int16*)(program.memory.content + instruction.shift));
		case ValueType::int32:
			return fromInt(*(int32*)(program.memory.content + instruction.shift));
		case ValueType::int64:
			return fromInt(*(int64*)(program.memory.content + instruction.shift));
		case ValueType::uint8:
			return fromInt(*(uint8*)(program.memory.content + instruction.shift));
		case ValueType::uint16:
			return fromInt(*(uint16*)(program.memory.content + instruction.shift));
		case ValueType::uint32:
			return fromInt(*(uint32*)(program.memory.content + instruction.shift));
		case ValueType::uint64:
			return fromInt(*(uint64*)(program.memory.content + instruction.shift));
		case ValueType::string:
		case ValueType::name:
		case ValueType::link:
			return **(String**)(program.memory.content + instruction.shift);
		case ValueType::dict:
			result.append(T("{\n"));
			for (auto var : *(Table<String, ValueType*>*)(program.memory.content + instruction.shift))
			{
				result.append(var.first);
				result.append(T(" : "));
				result.append(toStringGlobal(program, var.second));
				result.append(T("; "));
			}
			result.append(T("}"));
			return result;
		case ValueType::arr:
			result.append(T("["));
			for (int i = 0; i < program.stacks.arrays.at(instruction.shift).max_index; ++i)
			{
				result.append(toStringLocal(program, program.stacks.arrays.at(instruction.shift).at(i)));
				result.append(T("; "));
			}
			result.append(toStringLocal(program, program.stacks.arrays.at(instruction.shift).at_r(0)));
			result.append(T("]"));
			return result;
		default:
			result.append((charT*)(program.memory.content + instruction.shift), program.specification.type.size[instruction.value]);
			return result;
		}
	}

	String toStringLocalArray(Program& program, Instruction instruction) {
		String result = String();
		auto arr = program.stacks.arrays.get(instruction.shift);

		result.append(T("[ "));
		for (int i = 0; i < arr.max_index; ++i) {
			instruction = arr.get(i);
			result.append(program.specification.type.stringLocal[instruction.value](program, instruction));
			result.append(T(", "));
		}
		result.append(T("] "));
		return result;
	}

	void print(Program& program) {
		Instruction instruction_r0 = program.stacks.instructions.get_r(0);
		Instruction instruction_r1 = program.stacks.instructions.get_r(1);
		ToStringLocal function = 0;
		if (program.specification.type.stringLocal.count(instruction_r0.value))
			function = program.specification.type.stringLocal.at(instruction_r0.value);
		if (program.specification.type.stringLocal.count(ValueType::all))
			function = program.specification.type.stringLocal.at(ValueType::all);
		std::cout << function(program, instruction_r0);

		//delete program.memory.at<String*>(instruction_r1.shift); //Possible memory leak as ptr to ptr gets freed

		program.memory.move_relative(instruction_r0.shift, program.specification.type.size[instruction_r0.value], -(int64)sizeof(String**));
		program.stacks.instructions.at_r(1) = instruction_r0;
		--program.stacks.instructions.max_index;
	}

	void test(Program& program) {
		if (program.stacks.instructions[program.stacks.instructions.max_index].instr == InstructionType::value) {
			std::cout << program.specification.type.name.at(program.stacks.instructions[program.stacks.instructions.max_index].value) << std::endl;
			//program.stack.instructions.erase(program.stack.instructions.cend() - 2);
		}

		if (program.stacks.instructions[program.stacks.instructions.max_index - 2].instr == InstructionType::value) {
			std::cout << program.specification.type.name.at(program.stacks.instructions[program.stacks.instructions.max_index - 2].value) << std::endl;
			//program.stack.instructions.erase(program.stack.instructions.cend() - 2);
		} else {
			std::cout << program.specification.type.name.at(program.stacks.instructions[program.stacks.instructions.max_index - 1].value) << std::endl;
			//program.stack.instructions.erase(program.stack.instructions.cend() - 2);
		}
		/*
		program.stack.instructions.erase(program.stack.instructions.cend() - 2);
		switch (program.stack.instructions[program.stack.instructions.size() - 2].value)
		{
		case ValueType::string:
			std::cout << (**(String**)(program.memory.data + program.stack.instructions[program.stack.instructions.size() - 2].shift)).c_str() << std::endl;
		default:
			//std::cout << "Test! " << (int32)program.context.value << "\n";
			std::cout << program.specification.type.name.at(program.stack.instructions[program.stack.instructions.size() - 2].value) << std::endl;
		}*/
	}

	void conditional(Program& program) {
		size_t size = program.specification.type.size[program.stacks.instructions.at_r(0).value];

		uint8* ptr = (program.memory.content + program.stacks.instructions.at_r(0).shift);

		while(size)
			if (*(ptr + --size) != 0)
			{
				delete (*(String**)(program.memory.content + program.stacks.instructions.at_r(1).shift));
					//program.specification.type.destructor[program.stacks.instructions.at_r(2).value]; //TODO: delete value
					//delete ((String**)program.memory.content + program.stacks.instructions.at_r(1).shift);
				program.memory.max_index -= sizeof(void*);
				program.memory.max_index -= sizeof(int64);
				program.stacks.instructions.max_index -= 1;
				program.stacks.instructions.at_r(0).instr = InstructionType::skip_after_next;
				program.stacks.instructions.at_r(0).modifier = 0;
				return;
			}

		delete (*(String**)(program.memory.content + program.stacks.instructions.at_r(1).shift));
		program.memory.max_index -= sizeof(void*);
		program.memory.max_index -= sizeof(int64);
		program.stacks.instructions.max_index -= 1;
		program.stacks.instructions.at_r(0).instr = InstructionType::skip_next;
		program.stacks.instructions.at_r(0).modifier = 0;
	}

	void conditionalTrue(Program& program) {
		delete (*(String**)(program.memory.content + program.stacks.instructions.at_r(1).shift));
		program.memory.max_index -= sizeof(void*);
		program.memory.max_index -= sizeof(int64);
		program.stacks.instructions.max_index -= 1;
		program.stacks.instructions.at_r(0).instr = InstructionType::skip_after_next;
		program.stacks.instructions.at_r(0).modifier = 0;
	}

	void conditionalFalse(Program& program) {
		delete (*(String**)(program.memory.content + program.stacks.instructions.at_r(1).shift));
		program.memory.max_index -= sizeof(void*);
		program.memory.max_index -= sizeof(int64);
		program.stacks.instructions.max_index -= 1;
		program.stacks.instructions.at_r(0).instr = InstructionType::skip_next;
		program.stacks.instructions.at_r(0).modifier = 0;
	}

	//Implementation is the same in all three until I realise wether it's needed to free the memory occupied by local values
	void commaPrefix(Program& program) {
		program.stacks.instructions.at_r(2) = program.stacks.instructions.at_r(0);
		program.stacks.instructions.max_index -= 2;
	}
	void commaPostfix(Program& program) {
		program.stacks.instructions.at_r(2) = program.stacks.instructions.at_r(0);
		program.stacks.instructions.max_index -= 2;
	}
	void commaBinary(Program& program) {
		program.stacks.instructions.at_r(2) = program.stacks.instructions.at_r(0);
		program.stacks.instructions.max_index -= 2;
	}

	void getNamespace(Program& program) {}
	void atContextByIndex(Program& program) {}
	void atContextByName(Program& program) {}

	void callWithContext(Program& program) {}
	void renameArrayContext(Program& program) {}


	void invokeResolve(Program& program) {}

	void invokeProcedure(Program& program) {
		program.memory.at<Procedure>(program.stacks.instructions.at_r(1).shift)(program);
	}

	void invokeFunction(Program& program) {
		Instruction parameter = program.stacks.instructions.get_r(0);
		Instruction function = program.stacks.instructions.get_r(2);

		/*if (program.context.value == ValueType::dll)
		{
			GetProcAddress(program.memory.get<HMODULE>(program.context.shift), program.memory.get<String>(function.shift).c_str());
			program.stack.arrays.get_r(parameter.modifier).content;
		}*/

		program.stacks.instructions.max_index -= 2;

																									//TODO: add a function to add value
		program.stacks.instructions.add(Instruction::call(parameter.value, parameter.shift));	//TODO: make a 'call' instruction that specifies change of executable string
		program.stacks.instructions.add(Instruction::context(parameter.value, parameter.shift));


	}

	void invokeNativeFunction(Program& program) {

	}


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType type, _TypeResult (*function) (_TypeLeft, _TypeRight)>
	void binaryFunctionInterface(Program& program) {
		_TypeLeft left = program.memory.get<_TypeLeft>(program.stacks.instructions.get_r(2).shift);
		_TypeRight right = program.memory.get<_TypeRight>(program.stacks.instructions.get_r(0).shift);

		program.memory.max_index -= sizeof(_TypeLeft);
		program.memory.max_index -= sizeof(void*);
		program.memory.max_index -= sizeof(_TypeRight);

		program.memory.add<_TypeResult>(((_TypeResult(*) (_TypeLeft, _TypeRight))function)(left, right));

		program.stacks.instructions.max_index -= 2;
		program.stacks.instructions.at_r(0).value = type;
	}

	/*
	void getReferenceR0(Program& program) {
		getReference(program, program.stack.instructions.at_r(0));
	}

	void getReferenceR1(Program& program) {
		getReference(program, program.stack.instructions.at_r(1));
	}

	void getReferenceR2(Program& program) {
		getReference(program, program.stack.instructions.at_r(2));
	}*/

	template<size_t _index_r>
	void getReference(Program& program) {
		Instruction& name = program.stacks.instructions.at_r(_index_r);
		auto table = program.data.at_r(0);

		if (table.count(*program.memory.at<String*>(name.shift)))
		{
			ValueType* ptr = program.data.at_r(0)[*program.memory.at<String*>(name.shift)];
			
			delete (String**)program.memory.at<String*>(name.shift);

			program.memory.at<ValueType*>(name.shift) = (ValueType*&)ptr;
		}
		else
		{
			ValueType* ptr = (ValueType*)malloc(sizeof(ValueType));
			*ptr = ValueType::none;
			program.data.at_r(0)[*program.memory.at<String*>(name.shift)] = ptr;

			delete program.memory.at<String*>(name.shift);

			program.memory.at<ValueType*>(name.shift) = ptr;
		}
		name.value = ValueType::reference;
	}

	template<size_t _index_r>
	void getValue(Program& program) {
		Instruction& name = program.stacks.instructions.at_r(_index_r);
		auto table = program.data.at_r(0);

		if (table.count(*program.memory.at<String*>(name.shift)))
		{
			ValueType* ptr = program.data.at_r(0)[*program.memory.at<String*>(name.shift)];

			delete (String**)program.memory.at<String*>(name.shift); //TODO: probably memory leak as ptr to ptr deleted

			program.memory.replace(ptr + 1, program.specification.type.size[*ptr], name.shift, sizeof(void*));

			name.value = *ptr;
		}
		else
		{
			delete program.memory.at<String*>(name.shift);
			program.memory.erase(name.shift, sizeof(void*));
			name.value = ValueType::none;
		}
	}


	void assign(Program& program) {
		auto left = program.stacks.instructions.get_r(2);
		auto right = program.stacks.instructions.get_r(0);

		ValueType* ptr = (ValueType*)malloc(sizeof(ValueType) + program.specification.type.size[right.value]);
		*ptr = right.value;
		memcpy(ptr + 1, program.memory.content + right.shift, program.specification.type.size[right.value]);

		program.namespaces.get_r(0)[program.memory.get<String>(left.shift)] = ptr;
	}

	void assignToReference(Program& program) {
		auto left = program.stacks.instructions.get_r(2);
		auto right = program.stacks.instructions.get_r(0);

		ValueType* ptr = (ValueType*)malloc(sizeof(ValueType) + program.specification.type.size[right.value]);
		*ptr = right.value;
		memcpy(ptr + 1, program.memory.content + right.shift, program.specification.type.size[right.value]);

		program.namespaces.get_r(0)[program.memory.get<String>(left.shift)] = ptr;
	}


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType _valueType>
	void createFloat(Program& program) {
		_TypeResult number = 0;
		_TypeLeft left = program.memory.get<_TypeLeft>(program.stacks.instructions.get_r(2).shift);
		_TypeRight right = program.memory.get<_TypeLeft>(program.stacks.instructions.get_r(0).shift);

		if (right == 0)
			number = left * 1.0;
		else
			number = left * 1.0 + ((right*1.0) / pow(10, floor(log10(right))+1));

		program.memory.max_index -= sizeof(void*) + sizeof(_TypeLeft) + sizeof(_TypeRight);

		program.stacks.instructions.max_index -= 2;
		program.stacks.instructions.at_r(0).value = _valueType;
		program.memory.add<_TypeResult>(number);
	}

	void getValueProcedure(Program& program) { // Unordered map saves keys by address
		/*uint8* memory = program.memory.data + program.stack.instructions[program.stack.instructions.max_index].shift;
		uint8* value = (uint8*)(program.data.at_r(0)[**(String**)(memory)]);

		int l = program.specification.type.size[*(ValueType*)value];

		program.stack.instructions[program.stack.instructions.max_index].value = *(ValueType*)value;

		value = (uint8*)((ValueType*)value + 1);

		for (int i = 0; i < l; ++i)
			*(memory + i) = *(value + i);

		program.memory.currentSize -= sizeof(void*);
		program.memory.currentSize += l;

		program.stack.instructions.at_r(1) = program.stack.instructions.at_r(0);
		program.stack.instructions.max_index -= 1;*/
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
		/*String* right = *(String**)(program.memory.data + program.stack.instructions.at_r(0).shift);
		program.stack.instructions.max_index -= 2;
		String* left = *(String**)(program.memory.data + program.stack.instructions.at_r(0).shift);

		auto ptr = program.data.at_r(0)[*left];

		if (*ptr == ValueType::dict)
		{
			ptr = (**(std::unordered_map<String, ValueType*>**)(ptr + 1))[*right];
			//++ptr;
			//std::unordered_map<String, ValueType*>* map = *(std::unordered_map<String, ValueType*>**)ptr;
			//ptr = (*map)[*right];

			program.stack.instructions.at_r(0).value = *ptr;

			program.memory.currentSize -= sizeof(void*) + sizeof(void*) + sizeof(void*);
			for (int i = 0; i < program.specification.type.size[*ptr]; ++i)
				*(program.memory.data + program.memory.currentSize + i) = *((uint8*)ptr + sizeof(ValueType) + i);
			program.memory.currentSize += program.specification.type.size[*ptr];

			delete left,
			delete right;
		}*/
	}
}