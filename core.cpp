#include "core.h"


//String representation of heap memory
#define toStrGlobal(TYPE, FUNCTION) \
core->type.stringGlobal[ValueType::TYPE] = FUNCTION;

//String representation of stack memory
#define toStrLocal(TYPE, FUNCTION) \
core->type.stringLocal[ValueType::TYPE] = FUNCTION;

//Destructor
#define destr(TYPE, FUNCTION) \
core->type.destructor[ValueType::TYPE] = FUNCTION;

//Pointer in data
#define data_pointer(TYPE, NAME, POINTER) \
	ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*)); \
	if (!ptr) \
	throw std::bad_alloc(); \
	*ptr = ValueType::TYPE; \
	*(void*)(ptr + 1) = POINTER; \
	data[T(NAME)] = ptr;

//Function pointer in data
#define data_1(TYPE, NAME, POINTER) \
	ptr = (ValueType*)malloc(sizeof(ValueType) + 1); \
	if (!ptr) \
	throw std::bad_alloc(); \
	*ptr = ValueType::TYPE; \
	*(Procedure*)(ptr + 1) = POINTER; \
	data[T(NAME)] = ptr;

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

#define bFunInterfaceNameRefDowncast(TYPE) \
	bFun(name, name, findValueR0R2); \
	bFun(name, TYPE, findValueR2); \
	bFun(TYPE, name, findValueR0); \
	bFun(reference, reference, getValueR0R2); \
	bFun(reference, TYPE, getValueR2); \
	bFun(TYPE, reference, getValueR0); \
	bFun(reference, name, findValueR0); \
	bFun(name, reference, getValueR0);

#define bFunInterfaceNameRefDowncastTricketry \
	bFunInterfaceNameRefDowncast(int8); \
	bFunInterfaceNameRefDowncast(int16); \
	bFunInterfaceNameRefDowncast(int32); \
	bFunInterfaceNameRefDowncast(int64); \
	bFunInterfaceNameRefDowncast(uint8); \
	bFunInterfaceNameRefDowncast(uint16); \
	bFunInterfaceNameRefDowncast(uint32); \
	bFunInterfaceNameRefDowncast(uint64); \
	bFunInterfaceNameRefDowncast(float32); \
	bFunInterfaceNameRefDowncast(float64);


namespace Core {
	ValueType none = ValueType::none;

	Module* initCore() {
		//if (Core::core != 0)
		//	return *Core::core;
		Module* core = new Module();
		core->type.count = 45;
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
			{T("array"), ValueType::tuple},
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
			{ValueType::tuple, T("array")},
			{ValueType::expression, T("expression")},
		});
		core->type.size = new uint8[48]{
			0,
			(uint8)-1,
			1,
			1,
			0, 0,
			1, 2, 3, 4,
			1, 2, 3, 4,
			2, 4, 8, 16,
			(uint8)-1, (uint8)-1, (uint8)-1, (uint8)-1,
			(uint8)-1, (uint8)-1, (uint8)-1, (uint8)-1,
			(uint8)-1,
			8, 8, 8,
			8, 8, 8,
			8, 8, 8,
			8, 8, 8,
			(uint8)-1,
			8,
			8,
			8,
			8, 8,
			8,
			8,
			8
		};
		/*core->type.size = Table<ValueType, size_t>({	//typeID : type.size
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
		});*/
		core->type.stringGlobal = Table<ValueType, ToStringGlobal>();
		{
			toStrGlobal(all, toStringGlobal);
		}
		core->type.stringLocal = Table<ValueType, ToStringLocal>();
		{
			toStrLocal(all, toStringLocal);
			//toStrLocal(tuple, toStringLocalArray);
		}
		core->type.destructor = Table<ValueType, DestructorProcedure>();
		{
			destr(all, (DestructorProcedure)doNothing); //TODO: Fill with string, table and stuff
			destr(tuple, (DestructorProcedure)destructorArr);
			destr(autoptr, (DestructorProcedure)free);
		}
		core->type.destructorGlobal = Table<ValueType, Destructor>();
		{
			destr(all, (DestructorProcedure)doNothing); //TODO: Fill with string, table and stuff
			destr(tuple, (DestructorProcedure)destructorArr);
			destr(autoptr, (DestructorProcedure)free);
		}
		core->op.prefixForward = Table<String, Table<ValueType, Procedure>>();
		{
			Table<ValueType, Procedure>* table;

			table = &core->op.prefixForward[T("//")];
			uFun(all, ignore);
			
			table = &core->op.prefixForward[T("?")];
			uFun(all, conditional);
			uFun(name, findValueR0);
			uFun(truth, conditionalTrue);
			uFun(lie, conditionalFalse);

			table = &core->op.prefixForward[T("?.")];
			//uFun(all, conditionalShort);
			
			table = &core->op.prefixForward[T("?|")];
			//uFun(all, loopWhile);

			table = &core->op.prefixForward[T("?:")];
			//uFun(all, conditionalSwitch);
			//uFun(tuple, conditionalSwitch);

			table = &core->op.prefixForward[T("?<")];
			//uFun(all, conditionalComparator);

			table = &core->op.prefixForward[T("?:<")];
			//uFun(all, conditionalSwitchPredicate);

			table = &core->op.prefixForward[T("!<<")];
			uFun(name, getReferenceR0);
			uFun(all, print);

			table = &core->op.prefixForward[T("!>>")];
			uFun(name, getReferenceR0);
			uFun(reference, scan);

			table = &core->op.prefixForward[T(",")];
			uFun(all, commaPrefix);

			table = &core->op.prefixForward[T("+")];
			uFun(name, findValueR0);
			uFun(type, instantiate);
			uFun(table, instantiateObject);

			//table = &core->op.prefix[T("-")];
			//u uFun(int64, (negate<int64>));
			//uFun(float64, (negate<float64>));

			table = &core->op.prefixForward[T("&")];
			uFun(name, (getReferenceR1));

			table = &core->op.prefixForward[T("*")];
			uFun(name, findValueR0);

			table = &core->op.prefixForward[T("*&")];
			uFun(name, (findValueR0));
			uFun(reference, (getValueR0));

			table = &core->op.prefixForward[T(">")];
			uFun(all, allArrayInclusive);

			table = &core->op.prefixForward[T("^")];
			uFun(int64, getNamespace);

			table = &core->op.prefixForward[T("@")];
			uFun(int64, atContextByIndex);
			uFun(name, atContextByName);
			uFun(string, atContextByName);
			uFun(tuple, renameArrayContext);

			table = &core->op.prefixForward[T(".")];
			uFun(int64, contextMethod);

			table = &core->op.prefixForward[T("::")];
			uFun(string, getNamespaceEntry);
			//uFun(tuple, getApplication);

			table = &core->op.prefixForward[T("::<")];
			uFun(string, getNamespacePrefix);

			table = &core->op.prefixForward[T("::>")];
			uFun(string, getNamespacePostfix);

			table = &core->op.prefixForward[T("::.^")];
			uFun(string, getNamespaceBinaryRight);

			table = &core->op.prefixForward[T("::^.")];
			uFun(string, getNamespaceBinaryLeft);
		}
		core->op.postfixForward = Table<String, Table<ValueType, Procedure>>();
		{
			Table<ValueType, Procedure>* table;

			table = &core->op.postfixForward[T(",")];
			uFun(all, commaPostfix);
			
			table = &core->op.postfixForward[T("*")];
			//uFun(all, getValueProcedure);

			table = &core->op.postfixForward[T(">")];
			uFun(all, allArrayExclusive);

			table = &core->op.postfixForward[T("^")];
			uFun(all, allGroupExclusive);

			table = &core->op.postfixForward[T(":")];
			uFun(table, allContext);
			uFun(name, allContext);
			//uFun(type, allocDynamicArrayStack);

			table = &core->op.postfixForward[T("*:")];
			//uFun(dict, allocDynamicArrayHeap);

			table = &core->op.postfixForward[T(".")];
			uFun(name, findValueR2);
			uFun(reference, getValueR2);
			uFun(expression, callFunction);
		}
		core->op.binaryForward = Table<String, Table<ValueTypeBinary, Procedure>>();
		{
			Table<ValueTypeBinary, Procedure>* table;

			table = &core->op.binaryForward[T("?,")]; //Null coalesceing

			table = &core->op.binaryForward[T("?=")]; //Null conditional assignment

			table = &core->op.binaryForward[T("?.")]; //Null conditional method call

			table = &core->op.binaryForward[T("@")];
			bFun(unfunction, tuple, callWithContext);

			table = &core->op.binaryForward[T(",")];
			bFun(all, all, commaBinary);

			table = &core->op.binaryForward[T("+")];
			bFunInterfaceNameRefDowncastTricketry
			bFunInterfaceMatch(int64, int64, int64, add);
			bFunInterfaceMatch(float64, int64, float64, add);
			bFunInterfaceMatch(int64, float64, float64, add);
			bFunInterfaceMatch(float64, float64, float64, add);

			table = &core->op.binaryForward[T("-")];
			bFunInterfaceNameRefDowncastTricketry
			bFunInterfaceMatch(int64, int64, int64, sub);
			bFunInterfaceMatch(float64, int64, float64, sub);
			bFunInterfaceMatch(int64, float64, float64, sub);
			bFunInterfaceMatch(float64, float64, float64, sub);

			table = &core->op.binaryForward[T("*")];
			bFunInterfaceNameRefDowncastTricketry
			bFunInterfaceMatch(int64, int64, int64, mul);
			bFunInterfaceMatch(float64, int64, float64, mul);
			bFunInterfaceMatch(int64, float64, float64, mul);
			bFunInterfaceMatch(float64, float64, float64, mul);
			//bFun(type, none, allocHeap);

			table = &core->op.binaryForward[T("/")];
			bFunInterfaceNameRefDowncastTricketry
			bFunInterfaceMatch(int64, int64, float64, div);
			bFunInterfaceMatch(float64, int64, float64, div);
			bFunInterfaceMatch(int64, float64, float64, div);
			bFunInterfaceMatch(float64, float64, float64, div);

			table = &core->op.binaryForward[T("/.")];
			bFunInterfaceNameRefDowncastTricketry
			bFunInterfaceMatch(int64, int64, int64, div);
			bFunInterfaceMatch(float64, int64, int64, div);
			bFunInterfaceMatch(int64, float64, int64, div);
			bFunInterfaceMatch(float64, float64, int64, div);

			table = &core->op.binaryForward[T("%")];
			bFunInterfaceNameRefDowncastTricketry
			bFunInterfaceMatch(int64, int64, int64, mod);

			table = &core->op.binaryForward[T("=")];
			bFun(name, all, assignToName);
			bFun(reference, all, assignToReference);
			//bFun(pointer, all, assignToName);

			table = &core->op.binaryForward[T("<=")];
			bFun(name, all, assignToName);
			bFun(reference, all, assignToReference);

			table = &core->op.binaryForward[T(":<=")];
			//bFun(name, name, findValueR0);
			bFun(name, all, declareAssign);

			table = &core->op.binaryForward[T(":=")];
			bFun(name, all, declareAssign);

			table = &core->op.binaryForward[T(".")];
			bFun(int64, int64, (createFloat<int64, int64, float64, ValueType::float64>));
			bFun(all, name, findValueR0);
			bFun(all, unprocedure, callThis);

			table = &core->op.binaryForward[T("::")];
			bFun(name, name, findValueR2);
			bFun(name, string, findValueR2);
			bFun(table, name, getTableEntry);
			//bFun(table, tuple, getTable);
			bFun(table, string, getTableEntry);
			//bFun(type, int64, allocArrayStack);

			table = &core->op.binaryForward[T("::<")];
			bFun(name, name, findValueR0R2);
			bFun(table, string, getTablePrefix);

			table = &core->op.binaryForward[T("::>")];
			bFun(name, name, findValueR0R2);
			bFun(table, string, getTablePostfix);

			table = &core->op.binaryForward[T("::.^")];
			bFun(name, name, findValueR0R2);
			bFun(table, string, getTableBinaryRight);

			table = &core->op.binaryForward[T("::^.")];
			bFun(name, name, findValueR0R2);
			bFun(table, string, getTableBinaryLeft);

			table = &core->op.binaryForward[T("*:")];
			//bFun(type, int64, allocArrayHeap);
		}
		core->op.prefixGeneral = Table<ValueType, Procedure>();
		{
			Table<ValueType, Procedure>* table = &core->op.prefixGeneral;

			uFun(object, callObjectPrefix);
		}
		core->op.postfixGeneral = Table<ValueType, Procedure>();
		{
			Table<ValueType, Procedure>* table = &core->op.postfixGeneral;

			uFun(object, callObjectPostfix);
		}
		core->op.binaryGeneral = Table<ValueTypeBinary, Procedure>();
		{
			Table<ValueTypeBinary, Procedure>* table = &core->op.binaryGeneral;

			bFun(object, all, callObjectBinaryLeft);  
			bFun(all, object, callObjectBinaryRight);
		}
		core->op.coalescing = Table<ValueTypeBinary, Procedure>();
		{
			Table<ValueTypeBinary, Procedure>* table = &core->op.coalescing;

			bFun(name, all, (findValueR1)); //tuple
			bFun(reference, tuple, (findValueR1));
			bFun(unprocedure, all, invokeProcedure); //tuple
			bFun(parameter_pattern, tuple, invokeFunction);
			bFun(unfunction, tuple, invokeNativeFunction);

			bFun(type, name, declareVariable);

			bFun(string, string, concatenate);

			//bFun(type, none, allocStack);
		}

		core->context.onEnter = Table<ValueType, Procedure>();
		{
			Table<ValueType, Procedure>* table = &core->context.onEnter;

			uFun(all, doNothing);
			uFun(reference, onEnterContextReference);
			uFun(table, onEnterContextNamespace);
		}

		core->context.onExit = Table<ValueType, Procedure>();
		{
			Table<ValueType, Procedure>* table = &core->context.onExit;

			uFun(all, doNothing);
			uFun(table, onExitContextNamespace);
		}

		return core;
	}

	Table<String, ValueType*> initCoreData() {
		Table<String, ValueType*> data = Table<String, ValueType*>();
		ValueType* ptr;

		auto allocPtr = [](ValueType valueType, void* ptr) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*));
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(void**)(valuePtr + 1) = ptr;
			return valuePtr;
		};
		auto alloc8 = [](ValueType valueType, uint64 value) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + 8);
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(uint64*)(valuePtr + 1) = value;
			return valuePtr;
		};
		auto alloc4 = [](ValueType valueType, uint32 value) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + 4);
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(uint32*)(valuePtr + 1) = value;
			return valuePtr;
		};
		auto alloc2 = [](ValueType valueType, uint16 value) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + 2);
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(uint16*)(valuePtr + 1) = value;
			return valuePtr;
		};
		auto alloc1 = [](ValueType valueType, uint8 value) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + 1);
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(uint8*)(valuePtr + 1) = value;
			return valuePtr;
		};

		data["int64"] = alloc1(ValueType::type, (uint8)ValueType::int64);
		data["int32"] = alloc1(ValueType::type, (uint8)ValueType::int32);
		data["int16"] = alloc1(ValueType::type, (uint8)ValueType::int16);
		data["int8"] = alloc1(ValueType::type, (uint8)ValueType::int8);
		data["uint64"] = alloc1(ValueType::type, (uint8)ValueType::uint64);
		data["uint32"] = alloc1(ValueType::type, (uint8)ValueType::uint32);
		data["uint16"] = alloc1(ValueType::type, (uint8)ValueType::uint16);
		data["uint8"] = alloc1(ValueType::type, (uint8)ValueType::uint8);
		data["float32"] = alloc1(ValueType::type, (uint8)ValueType::float32);
		data["float64"] = alloc1(ValueType::type, (uint8)ValueType::float64);
		
		data["string"] = alloc1(ValueType::type, (uint8)ValueType::string);
		data["group"] = alloc1(ValueType::type, (uint8)ValueType::tuple);
		data["table"] = alloc1(ValueType::type, (uint8)ValueType::table);
		data["auto"] = alloc1(ValueType::type, (uint8)ValueType::table);
		

		data["print"] = allocPtr(ValueType::unprocedure, print);
		data["scan"] = allocPtr(ValueType::unprocedure, scan);
		data["if"] = allocPtr(ValueType::unprocedure, conditional);
		data["dll_load"] = allocPtr(ValueType::unprocedure, loadLibrary);
		data["dll_free"] = allocPtr(ValueType::unprocedure, freeLibrary);

		//data_pointer(unprocedure, "print", print);
		//data_pointer(unprocedure, "scan", scan);
		//data_pointer(unprocedure, "if", conditional); //Should be pattern matching for truth and lie types.
		//data_pointer(unprocedure, "dll_load", loadLibrary);
		//data_pointer(unprocedure, "dll_free", freeLibrary);

		return data;
	}

	void ignore() {
		//TODO: add script to 
	}


	void callThis() {
		g_memory.max_index = g_stack_instruction.get_r(1).shift; //We know that the right value is procedure pointer, so no special destructor should be.
		//g_memory.add<void*>(*g_memory.get<void**>(g_stack_instruction.get_r(0).shift));
		g_memory.add<void*>(g_memory.get<void*>(g_stack_instruction.get_r(0).shift));

		Array<Instruction>* tuple = new Array<Instruction>();
		Instruction instruction = g_stack_instruction.get_r(2);
		instruction.shift -= g_memory.max_index + sizeof(void*);
		tuple->add(instruction);

		instruction = g_stack_instruction.get_r(0);
		//instruction.shift = g_memory.max_index - (sizeof(void*) + 1);	//Fixes shift of procedure after it's moving
		instruction.shift -= sizeof(charT);
		g_stack_instruction.at_r(2) = instruction;
		g_stack_instruction.at_r(1) = Instruction::pos(InstructionType::start_array, g_memory.max_index);
		g_memory.add<Array<Instruction>*>(tuple);
		g_stack_instruction.at_r(0) = Instruction::atom(InstructionType::ignore_array_start);
	}

	void contextMethod() {

	}


	void contextAtIndex() {
		Instruction top = g_stack_context.get_r(0);
		if (top.value == ValueType::tuple) {
			Array<Instruction> arr = g_memory.get<Array<Instruction>>(top.shift);
			//arr.max_index;

			//Instruction instr = g_stack_array.at(g_context.shift).at(g_memory.at<int64>(g_stack_instruction.at_r(1).shift));
			//if(instr.value == ValueType::
		}
	}

	void contextAtName() {	//TODO: delete or think whether it's still needed.
		if (g_stack_context.get(0).value == ValueType::table) {
			auto var = &(*g_memory.at<Table<String, ValueType*>*>(g_stack_context.get(0).shift))[String(g_memory.at<charT*>(g_stack_instruction.at_r(0).shift))];
			g_memory.max_index = g_stack_instruction.at_r(1).shift;
			g_memory.add<ValueType**>(var);
		}
	}

	void concatenate() {
		g_stack_instruction.at_r(1).modifier += g_stack_instruction.at_r(0).modifier;
		--g_stack_instruction.max_index;
	}


	void getNamespace() {}
	void atContextByIndex() {}
	void atContextByName() {}

	void callWithContext() {}
	void renameArrayContext() {}


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType type, _TypeResult (*function) (_TypeLeft, _TypeRight)>
	void binaryFunctionInterface() {
		_TypeLeft left = g_memory.get<_TypeLeft>(g_stack_instruction.get_r(2).shift);
		_TypeRight right = g_memory.get<_TypeRight>(g_stack_instruction.get_r(0).shift);

		_TypeResult result = ((_TypeResult(*) (_TypeLeft, _TypeRight))function)(left, right);

		Instruction resultInstruction = g_stack_instruction.get_r(2);
		resultInstruction.value = type;

		g_memory_delete_span_r(3);
		g_memory.add<_TypeResult>(result);

		g_stack_instruction.add(resultInstruction);

		/*g_memory.max_index -= sizeof(_TypeLeft);
		g_memory.max_index -= sizeof(void*);
		g_memory.max_index -= sizeof(_TypeRight);

		g_memory.add<_TypeResult>(((_TypeResult(*) (_TypeLeft, _TypeRight))function)(left, right));

		g_stack_instruction.max_index -= 2;
		g_stack_instruction.at_r(0).value = type;*/
	}


	void getChild() {
		/*String* right = *(String**)(memory.data + g_stack_instruction.at_r(0).shift);
		g_stack_instruction.max_index -= 2;
		String* left = *(String**)(memory.data + g_stack_instruction.at_r(0).shift);

		auto ptr = data.at_r(0)[*left];

		if (*ptr == ValueType::dict)
		{
			ptr = (**(std::unordered_map<String, ValueType*>**)(ptr + 1))[*right];
			//++ptr;
			//std::unordered_map<String, ValueType*>* map = *(std::unordered_map<String, ValueType*>**)ptr;
			//ptr = (*map)[*right];

			g_stack_instruction.at_r(0).value = *ptr;

			memory.currentSize -= sizeof(void*) + sizeof(void*) + sizeof(void*);
			for (int i = 0; i < specification.type.size[*ptr]; ++i)
				*(memory.data + memory.currentSize + i) = *((uint8*)ptr + sizeof(ValueType) + i);
			memory.currentSize += specification.type.size[*ptr];

			delete left,
			delete right;
		}*/
	}
}