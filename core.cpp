#include "core.h"
#include <iostream>
//#include <libloaderapi.h>

//String representation of heap memory
#define toStrGlobal(TYPE, FUNCTION) \
core->type.stringGlobal[ValueType::TYPE] = FUNCTION;

//String representation of stack memory
#define toStrLocal(TYPE, FUNCTION) \
core->type.stringLocal[ValueType::TYPE] = FUNCTION;

//Destructor
#define destr(TYPE, FUNCTION) \
core->type.destructor[ValueType::TYPE] = FUNCTION;

//Unary function
#define uFun(LEFT_TYPE, FUNCTION) \
(*table)[ValueType::LEFT_TYPE] = FUNCTION;

//Unary function interface
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
	Module* initCore() {
		//if (Core::core != 0)
		//	return *Core::core;
		Module* core = new Module();
		core->type.id = Table<String, ValueType>({	//type.name : typeID
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
		core->type.name = Table<ValueType, String>({	//typeID : type.name
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
		core->type.size = Table<ValueType, size_t>({	//typeID : type.size
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
			{ValueType::reference, sizeof(void*)},
			{ValueType::pointer, sizeof(void*)},
		});
		core->type.stringGlobal = Table<ValueType, ToStringGlobal>();
		{
			toStrGlobal(all, toStringGlobal);
		}
		core->type.stringLocal = Table<ValueType, ToStringLocal>();
		{
			toStrLocal(all, toStringLocal);
			toStrLocal(arr, toStringLocalArray);
		}
		core->type.destructor = Table<ValueType, Destructor>();
		{
			destr(all, (Destructor)free); //TODO: Fill with string, table and stuff
		}
		core->op.prefix = Table<String, Table<ValueType, Procedure>>();
		{
			Table<ValueType, Procedure>* table;

			table = &core->op.prefix[T("//")];
			uFun(all, ignore);
			
			table = &core->op.prefix[T("?")];
			uFun(all, conditional);
			uFun(truth, conditionalTrue);
			uFun(lie, conditionalFalse);

			table = &core->op.prefix[T("?.")];
			//uFun(all, conditionalShort);

			table = &core->op.prefix[T("?|")];
			//uFun(all, loopWhile);

			table = &core->op.prefix[T("!<<")];
			uFun(name, getReference<0>);
			uFun(all, print);

			table = &core->op.prefix[T("!>>")];
			uFun(name, getReference<0>);
			uFun(reference, scan);

			table = &core->op.prefix[T(",")];
			uFun(all, commaPrefix);

			//table = &core->op.prefix[T("-")];
			//u uFun(int64, (negate<int64>));
			//uFun(float64, (negate<float64>));

			table = &core->op.prefix[T("&")];
			uFun(name, (getReference<1>));

			table = &core->op.prefix[T("*")];
			uFun(name, (getValue<1>));

			table = &core->op.prefix[T(">")];
			uFun(all, allArrayInclusive);

			table = &core->op.prefix[T("^")];
			uFun(int64, getNamespace);

			table = &core->op.prefix[T("@")];
			uFun(int64, atContextByIndex);
			uFun(name, atContextByName);
			uFun(string, atContextByName);
			uFun(arr, renameArrayContext);

			table = &core->op.prefix[T(".")];
			uFun(int64, contextMethod);
		}
		core->op.postfix = Table<String, Table<ValueType, Procedure>>();
		{
			Table<ValueType, Procedure>* table;

			table = &core->op.postfix[T(",")];
			uFun(all, commaPostfix);
			
			table = &core->op.postfix[T("*")];
			uFun(all, getValueProcedure);

			table = &core->op.postfix[T(">")];
			uFun(all, allArrayExclusive);

			table = &core->op.postfix[T("^")];
			uFun(all, allGroupExclusive);

			table = &core->op.postfix[T(":")];
			uFun(dict, allContext);
			uFun(name, allContext);
			//uFun(type, allocDynamicArrayStack);

			table = &core->op.postfix[T("*:")];
			//uFun(dict, allocDynamicArrayHeap);
		}

		core->op.binary = Table<String, Table<ValueTypeBinary, Procedure>>();
		{
			Table<ValueTypeBinary, Procedure>* table;

			table = &core->op.binary[T("@")];
			bFun(unfunction, arr, callWithContext);

			table = &core->op.binary[T(",")];
			bFun(all, all, commaBinary);

			table = &core->op.binary[T("+")];
			bFunInterfaceMatch(int64, int64, int64, add);
			bFunInterfaceMatch(float64, int64, float64, add);
			bFunInterfaceMatch(int64, float64, float64, add);
			bFunInterfaceMatch(float64, float64, float64, add);

			table = &core->op.binary[T("-")];
			bFunInterfaceMatch(int64, int64, int64, sub);
			bFunInterfaceMatch(float64, int64, float64, sub);
			bFunInterfaceMatch(int64, float64, float64, sub);
			bFunInterfaceMatch(float64, float64, float64, sub);

			table = &core->op.binary[T("*")];
			bFunInterfaceMatch(int64, int64, int64, mul);
			bFunInterfaceMatch(float64, int64, float64, mul);
			bFunInterfaceMatch(int64, float64, float64, mul);
			bFunInterfaceMatch(float64, float64, float64, mul);
			//bFun(type, none, allocHeap);

			table = &core->op.binary[T("/")];
			bFunInterfaceMatch(int64, int64, int64, div);
			bFunInterfaceMatch(float64, int64, float64, div);
			bFunInterfaceMatch(int64, float64, float64, div);
			bFunInterfaceMatch(float64, float64, float64, div);

			table = &core->op.binary[T("=")];
			bFun(name, all, assign);
			bFun(pointer, all, assign);

			table = &core->op.binary[T(".")];
			bFun(int64, int64, (createFloat<int64, int64, float64, ValueType::float64>));
			bFun(all, name, getValue<0>);
			bFun(all, unprocedure, callThis);

			table = &core->op.binary[T(":")];
			bFun(name, name, getChild);
			bFun(dict, name, getChild);
			//bFun(type, int64, allocArrayStack);

			table = &core->op.binary[T("*:")];
			//bFun(type, int64, allocArrayHeap);
		}

		core->op.coalescing = Table<ValueTypeBinary, Procedure>();
		{
			Table<ValueTypeBinary, Procedure>* table = &core->op.coalescing;

			bFun(name, all, (getValue<1>)); //arr
			bFun(reference, arr, (getValue<1>));
			bFun(unprocedure, all, invokeProcedure); //arr
			bFun(parameter_pattern, arr, invokeFunction);
			bFun(unfunction, arr, invokeNativeFunction);

			bFun(string, string, concatenate);

			//bFun(type, none, allocStack);
		}

		return core;
	}

	Table<String, ValueType*> initCoreData() {
		Table<String, ValueType*> data = Table<String, ValueType*>();
		ValueType* ptr;

		ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*));
		if (!ptr)
			throw std::bad_alloc();
		*ptr = ValueType::unprocedure;
		*(Procedure*)(ptr + 1) = print;
		data[T("print")] = ptr;

		ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*)); 
		if (!ptr)
			throw std::bad_alloc();
		*ptr = ValueType::unprocedure;
		*(Procedure*)(ptr + 1) = scan;
		data[T("scan")] = ptr;

		ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*));
		if (!ptr)
			throw std::bad_alloc();
		*ptr = ValueType::unprocedure;
		*(Procedure*)(ptr + 1) = conditional;	//Should be pattern matching for truth and lie types.
		data[T("if")] = ptr;

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

	String toStringGlobal(ValueType* value) {
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
			return String((charT*)(value+3), (*(uint16*)(value+1)));
		case ValueType::dict:
			result.append(T("{\n"));
			for (auto var : *(Table<String, ValueType*>*)(value + 1))
			{
				result.append(var.first);
				result.append(T(" : "));
				result.append(toStringGlobal(var.second));
				result.append(T("; "));
			}
			result.append(T("}"));
			return result;
		case ValueType::arr:
			result.append(T("["));
			for (int i = 0; i < (*(Array<ValueType*>*)(value + 1)).max_index; ++i)
			{
				result.append(toStringGlobal((*(Array<ValueType*>*)(value + 1)).get(i)));
				result.append(T("; "));
			}
			result.append(T("]"));
			return result;
		default:
			result.append((charT*)(value + 1), specification->type.size[*value]);
			return result;
		}
	}

	String toStringLocal(Instruction instruction) {
		String result = String();

		void* tmp;

		switch (instruction.value) {
		case ValueType::int8:
			return fromInt(*(int8*)(memory.content + instruction.shift));
		case ValueType::int16:
			return fromInt(*(int16*)(memory.content + instruction.shift));
		case ValueType::int32:
			return fromInt(*(int32*)(memory.content + instruction.shift));
		case ValueType::int64:
			return fromInt(*(int64*)(memory.content + instruction.shift));
		case ValueType::uint8:
			return fromUint(*(uint8*)(memory.content + instruction.shift));
		case ValueType::uint16:
			return fromUint(*(uint16*)(memory.content + instruction.shift));
		case ValueType::uint32:
			return fromUint(*(uint32*)(memory.content + instruction.shift));
		case ValueType::uint64:
			return fromUint(*(uint64*)(memory.content + instruction.shift));
		case ValueType::string:
		case ValueType::name:
		case ValueType::link:
			return String((charT*)(memory.content + instruction.shift), instruction.modifier);
		case ValueType::dict:
			result.append(T("{\n"));
			for (auto var : *(Table<String, ValueType*>*)(memory.content + instruction.shift))
			{
				result.append(var.first);
				result.append(T(" : "));
				result.append(toStringGlobal(var.second));
				result.append(T("; "));
			}
			result.append(T("}"));
			return result;
		case ValueType::arr:
			result.append(T("["));
			for (int i = 0; i < stack.arrays.at(instruction.shift).max_index; ++i)
			{
				result.append(toStringLocal(stack.arrays.at(instruction.shift).at(i)));
				result.append(T("; "));
			}
			result.append(toStringLocal(stack.arrays.at(instruction.shift).at_r(0)));
			result.append(T("]"));
			return result;
		case ValueType::reference:
			//result.append(fromInt(*(uintptr_t*)(memory.content + instruction.shift)));
			//result.append("(");
			//result.append("ref(");
			result.append(toStringGlobal(**(ValueType***)(memory.content + instruction.shift)));
			//result.append(")");
			return result;
		default:
			result.append((charT*)(memory.content + instruction.shift), specification->type.size[instruction.value]);
			return result;
		}
	}

	String toStringLocalArray(Instruction instruction) {
		String result = String();
		auto arr = stack.arrays.get(instruction.shift);

		result.append(T("[ "));
		for (int i = 0; i < arr.max_index; ++i) {
			instruction = arr.get(i);
			result.append(specification->type.stringLocal[instruction.value](instruction));
			result.append(T(", "));
		}
		result.append(T("] "));
		return result;
	}

	void ignore() {
		//TODO: add script to 
	}


	void callThis() {
		++stack.arrays.max_index;
		stack.arrays.at_r(0).init(); //allocate new array

		void* method = memory.at<void*>(stack.instructions.get_r(0).shift);
		memory.max_index -= sizeof(void*) + sizeof(charT); //erase operator and method pointer.

		memory.move_relative(stack.instructions.get_r(2).shift, stack.arrays.max_index, sizeof(void*));
		memory.at<void*>(stack.instructions.at_r(2).shift) = method;
		stack.instructions.at_r(0).shift = stack.instructions.get_r(2).shift;
		stack.instructions.at_r(2).shift += sizeof(void*); // Move method pointer to object position, and shift object

		stack.arrays.at_r(0).add(stack.instructions.get_r(2));

		stack.instructions.at_r(2) = stack.instructions.get_r(0);
		stack.instructions.at_r(1) = Instruction::pos(InstructionType::start_array, stack.arrays.max_index);
		stack.instructions.at_r(0) = Instruction::atom(InstructionType::ignore_array_start);
	}

	void contextMethod() {

	}


	void contextAtIndex() {
		if (context.value == ValueType::arr) {
			Instruction instr = stack.arrays.at(context.shift).at(memory.at<int64>(stack.instructions.at_r(1).shift));
			//if(instr.value == ValueType::
		}
	}

	void contextAtName() {
		if (context.value == ValueType::dict) {
			auto var = &(*memory.at<Table<String, ValueType*>*>(context.shift))[String(memory.at<charT*>(stack.instructions.at_r(0).shift))];
			memory.max_index = stack.instructions.at_r(1).shift;
			memory.add<ValueType**>(var);
		}
	}

	void concatenate() {
		stack.instructions.at_r(1).modifier += stack.instructions.at_r(0).modifier;
		--stack.instructions.max_index;
	}

	void print() {
		Instruction instruction_r0 = stack.instructions.get_r(0);
		Instruction instruction_r1 = stack.instructions.get_r(1);
		ToStringLocal function = 0;
		if (specification->type.stringLocal.count(instruction_r0.value))
			function = specification->type.stringLocal.at(instruction_r0.value);
		if (specification->type.stringLocal.count(ValueType::all))
			function = specification->type.stringLocal.at(ValueType::all);
#if ENCODING == 8
		std::cout << function(instruction_r0);
#elif ENCODING == 16
		std::wcout << function(program, instruction_r0);
#endif
		memory.move_absolute(instruction_r0.shift, memory.max_index, instruction_r1.shift);
		stack.instructions.at_r(1) = instruction_r0;
		--stack.instructions.max_index;
	}

	void scan() {
		String input = String();
#if ENCODING == 8
		std::getline(std::cin, input);
#elif ENCODING == 16
		std::getline(std::wcin, input); //Doesn't work yet
#endif
		Instruction instruction_r0 = stack.instructions.get_r(0);
		Instruction instruction_r1 = stack.instructions.get_r(1);

		ValueType*** ref = (ValueType***)(memory.content + instruction_r0.shift);

		ValueType* ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(uint16) + input.size()-1);
		
		*ptr = ValueType::string;
		*(uint16*)(ptr + 1) = input.size();
		memcpy(ptr + sizeof(ValueType) + sizeof(uint16), input.data(), input.size());

		if (!specification->type.destructor.count(***ref))
			if (!specification->type.destructor.count(ValueType::all))
				return;
			else
				specification->type.destructor[ValueType::all](**ref);
		else
			specification->type.destructor[***ref](**ref);

		**ref = ptr;

		memory.move_absolute(instruction_r0.shift, memory.max_index, instruction_r1.shift);
		//memory.move_relative(instruction_r0.shift, specification->type.size[instruction_r0.value], -(int64)sizeof(String**));
		stack.instructions.at_r(1) = instruction_r0;
		--stack.instructions.max_index;
	}

	void conditional() {
		size_t size = specification->type.size[stack.instructions.at_r(0).value];

		uint8* ptr = (memory.content + stack.instructions.at_r(0).shift);

		while(size)
			if (*(ptr + --size) != 0)
			{
				memory.max_index = stack.instructions.at_r(1).shift;
				stack.instructions.max_index -= 1;
				stack.instructions.at_r(0).instr = InstructionType::skip_after_next;
				stack.instructions.at_r(0).modifier = 0;
				return;
			}

		memory.max_index = stack.instructions.at_r(1).shift;
		stack.instructions.max_index -= 1;
		stack.instructions.at_r(0).instr = InstructionType::skip_next;
		stack.instructions.at_r(0).modifier = 0;
	}

	void conditionalTrue() {
		memory.max_index = stack.instructions.at_r(1).shift;
		stack.instructions.max_index -= 1;
		stack.instructions.at_r(0).instr = InstructionType::skip_after_next;
		stack.instructions.at_r(0).modifier = 0;
	}

	void conditionalFalse() {
		memory.max_index = stack.instructions.at_r(1).shift;
		stack.instructions.max_index -= 1;
		stack.instructions.at_r(0).instr = InstructionType::skip_next;
		stack.instructions.at_r(0).modifier = 0;
	}

	void loopWhile() { //TODO
		size_t size = specification->type.size[stack.instructions.at_r(0).value];

		uint8* ptr = (memory.content + stack.instructions.at_r(0).shift);

		while (size)
			if (*(ptr + --size) != 0)
			{
				memory.max_index = stack.instructions.at_r(1).shift;
				stack.instructions.max_index -= 1;
				stack.instructions.at_r(0).instr = InstructionType::skip_after_next;
				stack.instructions.at_r(0).modifier = 0;
				return;
			}
	}

	void commaPrefix() { //TODO: redo or prohibit prefix comma.
		if (stack.instructions.at_r(2).instr == InstructionType::spacing) {
			Instruction instruction_r0 = stack.instructions.get_r(0);
			memory.move_relative(instruction_r0.shift, memory.max_index, -(int64)sizeof(charT));

			stack.instructions.at_r(3) = stack.instructions.at_r(0);
			stack.instructions.max_index -= 3;
		} else {
			stack.instructions.at_r(1) = stack.instructions.at_r(0);
			stack.instructions.max_index -= 1;
		}
	}
	void commaPostfix() { 
		memory.max_index = stack.instructions.at_r(2).shift;
		stack.instructions.at_r(2) = stack.instructions.at_r(0);
		stack.instructions.max_index -= 2;
	}
	void commaBinary() {
		memory.max_index = stack.instructions.at_r(2).shift;
		stack.instructions.at_r(2) = stack.instructions.at_r(0);
		stack.instructions.max_index -= 2;
	}

	void getNamespace() {}
	void atContextByIndex() {}
	void atContextByName() {}

	void callWithContext() {}
	void renameArrayContext() {}


	void invokeResolve() {}

	void invokeProcedure() {
		memory.at<Procedure>(stack.instructions.at_r(1).shift)();
	}

	void invokeFunction() {
		Instruction parameter = stack.instructions.get_r(0);
		Instruction function = stack.instructions.get_r(2);

		/*if (context.value == ValueType::dll)
		{
			GetProcAddress(memory.get<HMODULE>(context.shift), memory.get<String>(function.shift).c_str());
			stack.arrays.get_r(parameter.modifier).content;
		}*/

		stack.instructions.max_index -= 2;

																									//TODO: add a function to add value
		stack.instructions.add(Instruction::call(parameter.value, parameter.shift));	//TODO: make a 'call' instruction that specifies change of executable string
		stack.instructions.add(Instruction::context(parameter.value, parameter.shift));


	}

	void invokeNativeFunction() {

	}


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType type, _TypeResult (*function) (_TypeLeft, _TypeRight)>
	void binaryFunctionInterface() {
		_TypeLeft left = memory.get<_TypeLeft>(stack.instructions.get_r(2).shift);
		_TypeRight right = memory.get<_TypeRight>(stack.instructions.get_r(0).shift);

		memory.max_index -= sizeof(_TypeLeft);
		memory.max_index -= sizeof(void*);
		memory.max_index -= sizeof(_TypeRight);

		memory.add<_TypeResult>(((_TypeResult(*) (_TypeLeft, _TypeRight))function)(left, right));

		stack.instructions.max_index -= 2;
		stack.instructions.at_r(0).value = type;
	}

	template<size_t _index_r>
	void getPointer() {
		Instruction& name = stack.instructions.at_r(_index_r);
		auto table = data.at_r(0);

		if (table.count(*memory.at<String*>(name.shift)))
		{
			ValueType* ptr = table[*memory.at<String*>(name.shift)];

			delete (String**)memory.at<String*>(name.shift);

			memory.at<ValueType*>(name.shift) = (ValueType*&)ptr;
		}
		else
		{
			ValueType* ptr = (ValueType*)malloc(sizeof(ValueType));
			*ptr = ValueType::none;
			data.at_r(0)[*memory.at<String*>(name.shift)] = ptr;

			delete memory.at<String*>(name.shift);

			memory.at<ValueType*>(name.shift) = ptr;
		}
		name.value = ValueType::pointer;
	}

	template<size_t _index_r>
	void getReference() {
		Instruction& name = stack.instructions.at_r(_index_r);
		Table<String, ValueType*>* table = &data.at_r(0);
		String str = String((charT*)(memory.content + name.shift), name.modifier);

		if (table->count(str))
		{
			ValueType** ptr = &(*table)[str];
			memory.at<ValueType**>(name.shift) = (ValueType**)ptr;
			memory.max_index = name.shift + sizeof(void*);
		}
		else
		{
			ValueType* ptr = (ValueType*)malloc(sizeof(ValueType));
			*ptr = ValueType::none;
			auto ref = &data.at_r(0)[str];
			*ref = ptr;
			memory.at<ValueType**>(name.shift) = ref;
			memory.max_index = name.shift + sizeof(void*);
		}
		name.value = ValueType::reference;
	}


	template<size_t _index_r>
	void getValue() {
		Instruction& name = stack.instructions.at_r(_index_r);
		auto table = data.at_r(0);
		String str = String((charT*)(memory.content + name.shift), name.modifier);

		if (table.count(str))
		{
			ValueType* ptr = data.at_r(0)[str];

			switch (*ptr) {
			case ValueType::string:
				//TODO: decide whether I use null terminated string or not.
				return;
			default:
				memory.replace(ptr + 1, specification->type.size[*ptr], name.shift, sizeof(void*));
				name.value = *ptr;
				return;
			}
		}
		else
		{
			memory.max_index -= sizeof(void*);
			name.value = ValueType::none;
		}
	}


	void assign() {
		auto left = stack.instructions.get_r(2);
		auto right = stack.instructions.get_r(0);

		ValueType* ptr = (ValueType*)malloc(sizeof(ValueType) + specification->type.size[right.value]);
		*ptr = right.value;
		memcpy(ptr + 1, memory.content + right.shift, specification->type.size[right.value]);

		namespaces.get_r(0)[memory.get<String>(left.shift)] = ptr;
	}

	void assignToReference() {
		auto left = stack.instructions.get_r(2);
		auto right = stack.instructions.get_r(0);

		ValueType* ptr = (ValueType*)malloc(sizeof(ValueType) + specification->type.size[right.value]);
		*ptr = right.value;
		memcpy(ptr + 1, memory.content + right.shift, specification->type.size[right.value]);

		namespaces.get_r(0)[memory.get<String>(left.shift)] = ptr;
	}


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType _valueType>
	void createFloat() {
		_TypeResult number = 0;
		_TypeLeft left = memory.get<_TypeLeft>(stack.instructions.get_r(2).shift);
		_TypeRight right = memory.get<_TypeLeft>(stack.instructions.get_r(0).shift);

		if (right == 0)
			number = left * 1.0;
		else
			number = left * 1.0 + ((right*1.0) / pow(10, floor(log10(right))+1));

		memory.max_index -= sizeof(void*) + sizeof(_TypeLeft) + sizeof(_TypeRight);

		stack.instructions.max_index -= 2;
		stack.instructions.at_r(0).value = _valueType;
		memory.add<_TypeResult>(number);
	}

	void getValueProcedure() { // Unordered map saves keys by address
		/*uint8* memory = memory.data + stack.instructions[stack.instructions.max_index].shift;
		uint8* value = (uint8*)(data.at_r(0)[**(String**)(memory)]);

		int l = specification.type.size[*(ValueType*)value];

		stack.instructions[stack.instructions.max_index].value = *(ValueType*)value;

		value = (uint8*)((ValueType*)value + 1);

		for (int i = 0; i < l; ++i)
			*(memory + i) = *(value + i);

		memory.currentSize -= sizeof(void*);
		memory.currentSize += l;

		stack.instructions.at_r(1) = stack.instructions.at_r(0);
		stack.instructions.max_index -= 1;*/
	}
	void allArrayInclusive() {

	}
	void allArrayExclusive() {

	}
	void allGroupInclusive() {

	}
	void allGroupExclusive() {

	}
	void allContext() {

	}

	void comma() {

	}

	void getChild() {
		/*String* right = *(String**)(memory.data + stack.instructions.at_r(0).shift);
		stack.instructions.max_index -= 2;
		String* left = *(String**)(memory.data + stack.instructions.at_r(0).shift);

		auto ptr = data.at_r(0)[*left];

		if (*ptr == ValueType::dict)
		{
			ptr = (**(std::unordered_map<String, ValueType*>**)(ptr + 1))[*right];
			//++ptr;
			//std::unordered_map<String, ValueType*>* map = *(std::unordered_map<String, ValueType*>**)ptr;
			//ptr = (*map)[*right];

			stack.instructions.at_r(0).value = *ptr;

			memory.currentSize -= sizeof(void*) + sizeof(void*) + sizeof(void*);
			for (int i = 0; i < specification.type.size[*ptr]; ++i)
				*(memory.data + memory.currentSize + i) = *((uint8*)ptr + sizeof(ValueType) + i);
			memory.currentSize += specification.type.size[*ptr];

			delete left,
			delete right;
		}*/
	}
}