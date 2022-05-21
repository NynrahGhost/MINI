#include "core.h"
#include <sstream>

namespace Core {

	void doNothing() {}

	void instantiate() {
		Instruction value = g_stack_instruction.get_r(0);
		ValueType type = g_val_mem.at<ValueType>(value.shift);

		value.value = type;
		g_val_mem.max_index = value.shift;
		g_stack_instruction.at_r(1) = value;
		g_stack_instruction.max_index -= 1;

		switch (type) {
		case ValueType::int8:
		case ValueType::uint8:
			g_val_mem.max_index += 1;
			break;
		case ValueType::int16:
		case ValueType::uint16:
			g_val_mem.max_index += 2;
			break;
		case ValueType::int32:
		case ValueType::uint32:
		case ValueType::float32:
			g_val_mem.max_index += 4;
			break;
		case ValueType::int64:
		case ValueType::uint64:
		case ValueType::float64:
			g_val_mem.max_index += 8;
			break;
		case ValueType::table:
			g_val_mem.add(new Table<String, ValueType*>());
			//*(Table<String, ValueType*>**)(g_memory.content + instr.shift) = new Table<String, ValueType*>();
			//g_memory.max_index += sizeof(void*);
			break;
		case ValueType::native_operator:
			g_val_mem.max_index += sizeof(Operation*);
			//g_memory.add(new Array<Instruction>());
			//g_memory.add(new Span(512)); //Last;
		default:
			break;
		}
	}

	void instantiateObject() {
		Instruction table = g_stack_instruction.get_r(0);
		
		//g_val_mem.max_index = table.shift + sizeof(void*);

		//table.instr = InstructionType::value;
		table.value = ValueType::object;
		table.modifier = 0;

		g_stack_instruction.at_r(1) = table;

		g_stack_instruction.max_index -= 1;
	}

	//For postfix instatiation
	/*void instantiate() {
		Instruction value = g_stack_instruction.get_r(1);
		ValueType type = g_memory.at<ValueType>(value.shift);

		Instruction postValue = g_stack_instruction.get_r(0);
		switch (postValue.instr) {
		case InstructionType::value:
		case InstructionType::op:
			//TODO: consider shifting last element.
		default:
			g_memory.max_index = g_stack_instruction.get_r(2).shift + g_specification->type.size[(size_t)type];
			Instruction& instr = g_stack_instruction.at_r(2);
			instr.instr = InstructionType::value;
			instr.modifier = value.modifier;
			instr.value = type;
			g_stack_instruction.max_index -= 2;
			break;
		}

		switch (type) {
		case ValueType::table:
			g_memory.at<void*>(value.shift - sizeof(charT)) = new std::unordered_map<String, ValueType*>();
			break;
		default:
			break;
		}
	}*/

	void declareVariable() {
		uint32 contextIndex = 0;
		Instruction context = g_stack_context.get_r(contextIndex);
		Instruction name = g_stack_instruction.get_r(0);
		Instruction type = g_stack_instruction.get_r(2);

		ValueType* pointer;

		switch (g_val_mem.get<ValueType>(type.shift)) {
		case ValueType::type:
			pointer = (ValueType*)malloc(sizeof(ValueType) + sizeof(ValueType)); break;
		case ValueType::uint8:
		case ValueType::int8:
			pointer = (ValueType*)malloc(sizeof(ValueType) + 1); break;
		case ValueType::uint16:
		case ValueType::int16:
			pointer = (ValueType*)malloc(sizeof(ValueType) + 2); break;
		case ValueType::uint32:
		case ValueType::int32:
		case ValueType::float32:
			pointer = (ValueType*)malloc(sizeof(ValueType) + 4); break;
		case ValueType::uint64:
		case ValueType::int64:
		case ValueType::float64:
			pointer = (ValueType*)malloc(sizeof(ValueType) + 8); break;
		case ValueType::pointer:
		case ValueType::table:
		case ValueType::tuple:
			pointer = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*)); break;
		//case ValueType::string:
			//pointer = (ValueType
		default:
			pointer = NULL;
		}

		*pointer = g_val_mem.get<ValueType>(type.shift);

		loop_context:
		switch (context.value) {
		case ValueType::reference:
			(**(Table<String, ValueType*>**)(*(ValueType**)(g_val_mem.content + context.shift) + 1))
				[String((charT*)(g_val_mem.content + name.shift), name.modifier)] = pointer;

			g_val_mem.max_index = type.shift;
			g_val_mem.add(pointer);

			//g_memory.move_relative(name.shift, name.modifier, -sizeof(ValueType));
			//g_memory.max_index -= sizeof(ValueType);

			name.shift = type.shift;
			name.value = ValueType::reference;
			name.modifier = (int16)*pointer;

			g_stack_instruction.max_index -= 2;
			g_stack_instruction.at_r(0) = name;
			return;
		case ValueType::autotable:
		case ValueType::table:
			(*g_val_mem.get<Table<String, ValueType*>*>(context.shift))
				[String((charT*)(g_val_mem.content + name.shift), name.modifier)] = pointer;

			g_val_mem.max_index = type.shift;
			g_val_mem.add(pointer);

			//g_memory.move_relative(name.shift, name.modifier, -sizeof(ValueType));
			//g_memory.max_index -= sizeof(ValueType);

			name.shift = type.shift;
			name.value = ValueType::reference;
			name.modifier = (int16)*pointer;

			g_stack_instruction.max_index -= 2;
			g_stack_instruction.at_r(0) = name;
			return;
		default:
			if (g_stack_context.max_index >= contextIndex)
			{
				g_stack_instruction.add(Instruction::atom(InstructionType::error));
				delete pointer;
				return;
			}
			context = g_stack_context.get_r(++contextIndex);
			goto loop_context;
		}

		
	}

	void declareAssign() {
		auto left = g_stack_instruction.get_r(2);
		auto right = g_stack_instruction.get_r(0);
		String strLeft = String((charT*)(g_val_mem.content + left.shift), left.modifier);

		Table<String, ValueType*>* table = g_stack_namespace.get_r(0);
		if (table->count(strLeft))
		{
			ValueType* value = (*table)[strLeft];
			g_specification->type.destructorGlobal[ValueType::reference](value);
			//TODO: implement destruction
		}

		ValueType* ptr;

		switch (right.value)
		{
		case ValueType::type:
			ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(ValueType));	//TODO: check for memory
			memcpy(ptr + 1, g_val_mem.content + right.shift, sizeof(ValueType));
			break;
		case ValueType::int8:
		case ValueType::uint8:
			ptr = (ValueType*)malloc(sizeof(ValueType) + 1);	//TODO: check for memory
			memcpy(ptr + 1, g_val_mem.content + right.shift, 1);
			break;
		case ValueType::int16:
		case ValueType::uint16:
			ptr = (ValueType*)malloc(sizeof(ValueType) + 2);	//TODO: check for memory
			memcpy(ptr + 1, g_val_mem.content + right.shift, 2);
			break;
		case ValueType::int32:
		case ValueType::uint32:
			ptr = (ValueType*)malloc(sizeof(ValueType) + 4);	//TODO: check for memory
			memcpy(ptr + 1, g_val_mem.content + right.shift, 4);
			break;
		case ValueType::int64:
		case ValueType::uint64:
			ptr = (ValueType*)malloc(sizeof(ValueType) + 8);	//TODO: check for memory
			memcpy(ptr + 1, g_val_mem.content + right.shift, 8);
			break;
		case ValueType::reference:
		case ValueType::table:
		case ValueType::expression:
			ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*));	//TODO: check for memory
			memcpy(ptr + 1, g_val_mem.content + right.shift, sizeof(void*));
			break;
		case ValueType::name:
		{
			String strRight = String((charT*)(g_val_mem.content + right.shift), right.modifier);
			ValueType* val = findName(strRight);
			ptr = (ValueType*)malloc(sizeof(ValueType) + g_specification->type.size[(size_t)*val]);
			memcpy(ptr, val, g_specification->type.size[(size_t)*val] + sizeof(ValueType));
			(*table)[strLeft] = ptr;

			g_stack_instruction.max_index -= 2;
			left.value = ValueType::reference;
			left.modifier = (uint16)*val;
			g_val_mem.max_index = left.shift + sizeof(void*);
			return;
		}
		default:
			throw "Can't assign type";
		}
		*ptr = right.value;
		(*table)[strLeft] = ptr;

		g_stack_instruction.max_index -= 2;
		left.value = ValueType::reference;
		left.modifier = (uint16)right.value;
		g_val_mem.max_index = left.shift + sizeof(void*); //TODO: decide whether should return reference or jut clear
	}

	void assignToName() {
		auto left = g_stack_instruction.get_r(2);
		auto right = g_stack_instruction.get_r(0);
		String str = String((charT*)(g_val_mem.content + left.shift), left.modifier);
		ValueType* ptr;

		if (ptr = findName(str))
		{
			if (*ptr == ValueType::instruction)
				left = *(Instruction*)(ptr + 1);

			if (assign(ptr+sizeof(ValueType), *ptr, g_val_mem.content + right.shift, right.value)) {
				g_val_mem.max_index = right.shift;
				g_stack_instruction.max_index -= 2;
				return;
			}
		}
		g_stack_instruction.add(Instruction::atom(InstructionType::error));
	}

	void assignToReference() {
		auto left = g_stack_instruction.get_r(2);
		auto right = g_stack_instruction.get_r(0);
		
		//(g_memory.get<int8*>(left.shift) + 1)
		//(*(ValueType**)(g_memory.content + left.shift))+1

		if (assign((g_val_mem.get<ValueType*>(left.shift) + 1), (ValueType)left.modifier, g_val_mem.content + right.shift, right.value)) {
			g_val_mem.max_index = right.shift;
			g_stack_instruction.max_index -= 2;
		} else {
			g_stack_instruction.add(Instruction::atom(InstructionType::error));
		}
	}

	bool assign(void* toValue, ValueType toType, void* fromValue, ValueType fromType) {
		switch (fromType) {
		case ValueType::int64:
			switch (toType) {
			case ValueType::int8:
			case ValueType::uint8:
				*(int64*)toValue = *(int8*)fromValue; return true;
			case ValueType::int16:
			case ValueType::uint16:
				*(int64*)toValue = *(int16*)fromValue; return true;
			case ValueType::int32:
			case ValueType::uint32:
				*(int64*)toValue = *(int32*)fromValue; return true;
			case ValueType::int64:
			case ValueType::uint64:
				*(int64*)toValue = *(int64*)fromValue; return true;
			//case ValueType::string:
				//*(charT**)toValue = malloc(
			}
		case ValueType::table:
			switch (toType) {
			case ValueType::table:
				*(Table<String, ValueType*>**)toValue = *(Table<String, ValueType*>**)fromValue; return true;
			}
		}
		return false;
	}


	void swapInstructionsRxRy(Instruction* a, Instruction* b) {
		Instruction* c = a;
		*b = *a;
		*a = *c;
	}


	void getPointerR0() {
		Instruction& name = g_stack_instruction.at_r(0);
		auto table = g_data;

		if (table.count(*g_val_mem.at<String*>(name.shift)))
		{
			ValueType* ptr = table[*g_val_mem.at<String*>(name.shift)];

			delete (String**)g_val_mem.at<String*>(name.shift);

			g_val_mem.at<ValueType*>(name.shift) = (ValueType*&)ptr;
		}
		else
		{
			ValueType* ptr = (ValueType*)malloc(sizeof(ValueType));
			*ptr = ValueType::none;
			g_data[*g_val_mem.at<String*>(name.shift)] = ptr;

			delete g_val_mem.at<String*>(name.shift);

			g_val_mem.at<ValueType*>(name.shift) = ptr;
		}
		name.value = ValueType::pointer;
	}

	void getPointerR1() {
		Instruction& name = g_stack_instruction.at_r(1);
		auto table = g_data;

		if (table.count(*g_val_mem.at<String*>(name.shift)))
		{
			ValueType* ptr = table[*g_val_mem.at<String*>(name.shift)];

			delete (String**)g_val_mem.at<String*>(name.shift);

			g_val_mem.at<ValueType*>(name.shift) = (ValueType*&)ptr;
		}
		else
		{
			ValueType* ptr = (ValueType*)malloc(sizeof(ValueType));
			*ptr = ValueType::none;
			g_data[*g_val_mem.at<String*>(name.shift)] = ptr;

			delete g_val_mem.at<String*>(name.shift);

			g_val_mem.at<ValueType*>(name.shift) = ptr;
		}
		name.value = ValueType::pointer;
	}


	void getReferenceR0() {
		Instruction name = g_stack_instruction.get_r(0);
		String str = String((charT*)(g_val_mem.content + name.shift), name.modifier);
		ValueType* ptr;

		if (ptr = findName(str))
		{
			g_val_mem.at<ValueType*>(name.shift) = (ValueType*)ptr;
			g_val_mem.max_index = name.shift + sizeof(void*);
			name.value = ValueType::reference;
			name.modifier = (uint16)*ptr;
			g_stack_instruction.at_r(0) = name;
		}
		else
		{
			g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_invalid_op }); //TODO: add custom exception
		}
	}

	void getReferenceR1() {
		Instruction name = g_stack_instruction.get_r(1);
		String str = String((charT*)(g_val_mem.content + name.shift), name.modifier);
		ValueType* ptr;

		if (ptr = findName(str))
		{
			memcpy(g_val_mem.content + name.shift + sizeof(void*), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
			*(void**)(g_val_mem.content + name.shift) = (void*)(ptr + 1);
			g_stack_instruction.at_r(0).shift += sizeof(void*) - name.modifier;
			g_val_mem.max_index += sizeof(void*) - name.modifier;

			name.value = ValueType::reference;
			name.shift += sizeof(void*) - name.modifier;
			name.modifier = (uint16)*ptr;
			g_stack_instruction.at_r(1) = name;
		}
		else
		{
			g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_invalid_op }); //TODO: add custom exception
		}
	}

	void getValueR0() {
		Instruction ref = g_stack_instruction.get_r(0);
		ValueType* ptr = *(ValueType**)(g_val_mem.content + ref.shift);

		switch (*ptr) {
		case ValueType::int8:
		case ValueType::uint8:
		case ValueType::type:
			g_val_mem.max_index = ref.shift;
			g_val_mem.add<int8>(*(int8*)(ptr + 1));
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(0) = ref;
			return;
		case ValueType::int16:
		case ValueType::uint16:
			g_val_mem.max_index = ref.shift;
			g_val_mem.add<int16>(*(int16*)(ptr + 1));
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(0) = ref;
			return;
		case ValueType::int32:
		case ValueType::uint32:
		case ValueType::float32:
			g_val_mem.max_index = ref.shift;
			g_val_mem.add<int32>(*(int32*)(ptr + 1));
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(0) = ref;
			return;
		case ValueType::int64:
		case ValueType::uint64:
		case ValueType::float64:
			g_val_mem.max_index = ref.shift;
			g_val_mem.add<int64>(*(int64*)(ptr + 1));
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(0) = ref;
			return;

		case ValueType::string:
			return;
		case ValueType::tuple:
		case ValueType::table:
		case ValueType::native_operator:
			g_val_mem.max_index = ref.shift;
			g_val_mem.add<void*>(*(Operation**)(ptr + 1));
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(0) = ref;
			return;
		default:
			g_val_mem.replace(ptr + 1, g_specification->type.size[(uint8)*ptr], ref.shift, sizeof(void*));
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(0) = ref;
			return;
		}
	}

	void getValueR1() {
		Instruction ref = g_stack_instruction.get_r(1);
		Instruction right = g_stack_instruction.get_r(0);
		ValueType* ptr = *(ValueType**)(g_val_mem.content + ref.shift);

		switch (*ptr) {
		case ValueType::int8:
		case ValueType::uint8:
		case ValueType::type:
			//memcpy(g_memory.content + name.shift + sizeof(int8), g_memory.content + name.shift + name.modifier, g_memory.max_index - name.shift + name.modifier);
			g_val_mem.move_relative(right.shift, g_val_mem.max_index, -(int)sizeof(void*) + 1);

			*(int8*)(g_val_mem.content + ref.shift) = *(int8*)(ptr + 1);
			g_val_mem.max_index += -(int)sizeof(void*) + 1;

			right.shift += -(int)sizeof(void*) + 1;
			g_stack_instruction.at_r(0) = right;
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(1) = ref;
			return;
		case ValueType::int16:
		case ValueType::uint16:
			memcpy(g_val_mem.content + ref.shift + sizeof(int16), g_val_mem.content + ref.shift + sizeof(void*), g_val_mem.max_index - ref.shift + sizeof(void*));
			*(int16*)(g_val_mem.content + ref.shift) = *(int16*)(ptr + 1);
			g_val_mem.max_index += -(int)sizeof(void*) + 2;
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(1) = ref;
			return;
		case ValueType::int32:
		case ValueType::uint32:
		case ValueType::float32:
			memcpy(g_val_mem.content + ref.shift + sizeof(int32), g_val_mem.content + ref.shift + sizeof(void*), g_val_mem.max_index - ref.shift + sizeof(void*));
			*(int32*)(g_val_mem.content + ref.shift) = *(int32*)(ptr + 1);
			g_val_mem.max_index += -(int)sizeof(void*) + 4;
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(1) = ref;
			return;
		case ValueType::int64:
		case ValueType::uint64:
		case ValueType::float64:
			memcpy(g_val_mem.content + ref.shift + sizeof(int64), g_val_mem.content + ref.shift + sizeof(void*), g_val_mem.max_index - ref.shift + sizeof(void*));
			*(int64*)(g_val_mem.content + ref.shift) = *(int64*)(ptr + 1);
			g_val_mem.max_index += -(int)sizeof(void*) + 8;
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(1) = ref;
			return;
		case ValueType::string:
			return;
		case ValueType::tuple:
		case ValueType::table:
			*(Procedure*)(g_val_mem.content + ref.shift) = *(Procedure*)(ptr + 1);
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(1) = ref;
			return;
		case ValueType::native_operator:
			*(Operation*)(g_val_mem.content + ref.shift) = *(Operation*)(ptr + 1);
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(1) = ref;
			return;
		default:
			g_val_mem.replace(ptr + 1, g_specification->type.size[(uint8)*ptr], ref.shift, sizeof(void*));
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(1) = ref;
			return;
		}
	}

	void getValueR2() {
		Instruction ref = g_stack_instruction.get_r(2);
		Instruction right = g_stack_instruction.get_r(1);
		Instruction rightmost = g_stack_instruction.get_r(0);
		ValueType* ptr = *(ValueType**)(g_val_mem.content + ref.shift);

		switch (*ptr) {
		case ValueType::int8:
		case ValueType::uint8:
		case ValueType::type:
			//memcpy(g_memory.content + name.shift + sizeof(int8), g_memory.content + name.shift + name.modifier, g_memory.max_index - name.shift + name.modifier);
			g_val_mem.move_relative(right.shift, g_val_mem.max_index, -(int)sizeof(void*) + 1);

			*(int8*)(g_val_mem.content + ref.shift) = *(int8*)(ptr + 1);
			g_val_mem.max_index += -(int)sizeof(void*) + 1;

			rightmost.shift += -(int)sizeof(void*) + 1;
			g_stack_instruction.at_r(0) = rightmost;
			right.shift += -(int)sizeof(void*) + 1;
			g_stack_instruction.at_r(1) = right;
			ref.value = *ptr;
			g_stack_instruction.at_r(2) = ref;
			return;
		case ValueType::int16:
		case ValueType::uint16:
			memcpy(g_val_mem.content + ref.shift + sizeof(int16), g_val_mem.content + ref.shift + sizeof(void*), g_val_mem.max_index - ref.shift + sizeof(void*));
			*(int16*)(g_val_mem.content + ref.shift) = *(int16*)(ptr + 1);
			g_val_mem.max_index += -(int)sizeof(void*) + 2;

			rightmost.shift += -(int)sizeof(void*) + 2;
			g_stack_instruction.at_r(0) = rightmost;
			right.shift += -(int)sizeof(void*) + 2;
			g_stack_instruction.at_r(1) = right;
			ref.value = *ptr;
			g_stack_instruction.at_r(2) = ref;
			return;
		case ValueType::int32:
		case ValueType::uint32:
		case ValueType::float32:
			memcpy(g_val_mem.content + ref.shift + sizeof(int32), g_val_mem.content + ref.shift + sizeof(void*), g_val_mem.max_index - ref.shift + sizeof(void*));
			*(int32*)(g_val_mem.content + ref.shift) = *(int32*)(ptr + 1);
			g_val_mem.max_index += -(int)sizeof(void*) + 4;

			rightmost.shift += -(int)sizeof(void*) + 4;
			g_stack_instruction.at_r(0) = rightmost;
			right.shift += -(int)sizeof(void*) + 4;
			g_stack_instruction.at_r(1) = right;
			ref.value = *ptr;
			g_stack_instruction.at_r(2) = ref;
			return;
		case ValueType::int64:
		case ValueType::uint64:
		case ValueType::float64:
			memcpy(g_val_mem.content + ref.shift + sizeof(int64), g_val_mem.content + ref.shift + sizeof(void*), g_val_mem.max_index - ref.shift + sizeof(void*));
			*(int64*)(g_val_mem.content + ref.shift) = *(int64*)(ptr + 1);
			g_val_mem.max_index += -(int)sizeof(void*) + 8;

			rightmost.shift += -(int)sizeof(void*) + 8;
			g_stack_instruction.at_r(0) = rightmost;
			right.shift += -(int)sizeof(void*) + 8;
			g_stack_instruction.at_r(1) = right;
			ref.value = *ptr;
			g_stack_instruction.at_r(2) = ref;
			return;
		case ValueType::string:
			return;
		case ValueType::tuple:
		case ValueType::table:
		case ValueType::expression:
			*(Procedure*)(g_val_mem.content + ref.shift) = *(Procedure*)(ptr + 1);

			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(2) = ref;
			return;
		case ValueType::native_operator:
			*(Operation*)(g_val_mem.content + ref.shift) = *(Operation*)(ptr + 1);
			ref.value = *ptr;
			ref.modifier = 0;
			g_stack_instruction.at_r(2) = ref;
			return;
		default:
			g_val_mem.replace(ptr + 1, g_specification->type.size[(uint8)*ptr], ref.shift, sizeof(void*));
			ref.value = *ptr;
			return;
		}
	}

	void getValueR0R2() { //TODO: this
		getValueR2();
		getValueR0();
	}


	void findValueR0() {
		Instruction name = g_stack_instruction.get_r(0);
		String str = String((charT*)(g_val_mem.content + name.shift), name.modifier);
		ValueType* ptr;

		if (ptr = findName(str))
		{
			switch (*ptr) {
			case ValueType::int8:
			case ValueType::uint8:
			case ValueType::type:
				g_val_mem.max_index = name.shift;
				g_val_mem.add<int8>(*(int8*)(ptr + 1));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(0) = name;
				return;
			case ValueType::int16:
			case ValueType::uint16:
				g_val_mem.max_index = name.shift;
				g_val_mem.add<int16>(*(int16*)(ptr + 1));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(0) = name;
				return;
			case ValueType::int32:
			case ValueType::uint32:
			case ValueType::float32:
				g_val_mem.max_index = name.shift;
				g_val_mem.add<int32>(*(int32*)(ptr + 1));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(0) = name;
				return;
			case ValueType::int64:
			case ValueType::uint64:
			case ValueType::float64:
				g_val_mem.max_index = name.shift;
				g_val_mem.add<int64>(*(int64*)(ptr + 1));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(0) = name;
				return;

			case ValueType::string:
				return;
			case ValueType::tuple:
			case ValueType::table:
				g_val_mem.max_index = name.shift;
				g_val_mem.add<void*>(*(void**)(ptr + 1));
				//g_memory.add<uintptr_t>((uintptr_t)(ptr + 1));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(0) = name;
				return;
			case ValueType::native_operator:
				g_val_mem.max_index = name.shift;
				g_val_mem.add<Operation*>(*(Operation**)(ptr + 1));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(0) = name;
				return;
			default:
				g_val_mem.replace(ptr + 1, g_specification->type.size[(uint8)*ptr], name.shift, sizeof(void*));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(0) = name;
				return;
			}
		}
		else
		{
			g_val_mem.max_index -= sizeof(void*);
			name.value = ValueType::none;
			name.modifier = 0;
			g_stack_instruction.at_r(0) = name;
		}
	}

	void findValueR1() {
		Instruction name = g_stack_instruction.get_r(1);
		Instruction right = g_stack_instruction.get_r(0);
		String str = String((charT*)(g_val_mem.content + name.shift), name.modifier);
		ValueType* ptr;

		if (ptr = findName(str))
		{
			switch (*ptr) {
			case ValueType::int8:
			case ValueType::uint8:
			case ValueType::type:
				//memcpy(g_memory.content + name.shift + sizeof(int8), g_memory.content + name.shift + name.modifier, g_memory.max_index - name.shift + name.modifier);
				g_val_mem.move_relative(right.shift, g_val_mem.max_index, -name.modifier + 1);

				*(int8*)(g_val_mem.content + name.shift) = *(int8*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;

				right.shift += -name.modifier + 1;
				g_stack_instruction.at_r(0) = right;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(1) = name;
				return;
			case ValueType::int16:
			case ValueType::uint16:
				memcpy(g_val_mem.content + name.shift + sizeof(int16), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				*(int16*)(g_val_mem.content + name.shift) = *(int16*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;
				//name.shift += sizeof(void*) - name.modifier;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(1) = name;
				return;
			case ValueType::int32:
			case ValueType::uint32:
			case ValueType::float32:
				memcpy(g_val_mem.content + name.shift + sizeof(int32), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				*(int32*)(g_val_mem.content + name.shift) = *(int32*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;
				//name.shift += sizeof(void*) - name.modifier;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(1) = name;
				return;
			case ValueType::int64:
			case ValueType::uint64:
			case ValueType::float64:
				memcpy(g_val_mem.content + name.shift + sizeof(int64), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				*(int64*)(g_val_mem.content + name.shift) = *(int64*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;
				//name.shift += sizeof(void*) - name.modifier;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(1) = name;
				return;
			case ValueType::string:
				return;
			case ValueType::tuple:
			case ValueType::table:
				memcpy(g_val_mem.content + name.shift + sizeof(void*), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				*(Procedure*)(g_val_mem.content + name.shift) = *(Procedure*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;
				//name.shift += sizeof(void*) - name.modifier;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(1) = name;
				return;
				//case ValueType::native_operator:
			default:
				g_val_mem.replace(ptr + 1, g_specification->type.size[(uint8)*ptr], name.shift, sizeof(void*));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(1) = name;
				return;
			}
		}
		else
		{
			g_val_mem.max_index -= sizeof(void*);
			name.value = ValueType::none;
			name.modifier = 0;
			g_stack_instruction.at_r(1) = name;
		}
	}

	void findValueR2() {
		Instruction name = g_stack_instruction.get_r(2);
		//Instruction right = g_stack_instruction.get_r(1);
		Instruction rightmost = g_stack_instruction.get_r(0);
		String str = String((charT*)(g_val_mem.content + name.shift), name.modifier);
		ValueType* ptr;

		if (ptr = findName(str))
		{
			switch (*ptr) {
			case ValueType::int8:
			case ValueType::uint8:
			case ValueType::type:
				memcpy(g_val_mem.content + name.shift + sizeof(int8), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				//g_val_mem.move_relative(right.shift, g_val_mem.max_index, -name.modifier + 1);
				*(int8*)(g_val_mem.content + name.shift) = *(int8*)(ptr + 1);

				*(int8*)(g_val_mem.content + name.shift) = *(int8*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;

				rightmost.shift += -name.modifier + 1;
				g_stack_instruction.at_r(0) = rightmost;
				//right.shift += -name.modifier + 1;
				//g_stack_instruction.at_r(1) = right;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(2) = name;
				return;
			case ValueType::int16:
			case ValueType::uint16:
				memcpy(g_val_mem.content + name.shift + sizeof(int16), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				*(int16*)(g_val_mem.content + name.shift) = *(int16*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;
				
				rightmost.shift += -name.modifier + 2;
				g_stack_instruction.at_r(0) = rightmost;
				//right.shift += -name.modifier + 2;
				//g_stack_instruction.at_r(1) = right;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(2) = name;
				return;
			case ValueType::int32:
			case ValueType::uint32:
			case ValueType::float32:
				memcpy(g_val_mem.content + name.shift + sizeof(int32), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				*(int32*)(g_val_mem.content + name.shift) = *(int32*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;
				
				rightmost.shift += -name.modifier + 4;
				g_stack_instruction.at_r(0) = rightmost;
				//right.shift += -name.modifier + 4;
				//g_stack_instruction.at_r(1) = right;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(2) = name;
				return;
			case ValueType::int64:
			case ValueType::uint64:
			case ValueType::float64:
				memcpy(g_val_mem.content + name.shift + sizeof(int64), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				*(int64*)(g_val_mem.content + name.shift) = *(int64*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;
				
				rightmost.shift += -name.modifier + 8;
				g_stack_instruction.at_r(0) = rightmost;
				//right.shift += -name.modifier + 8;
				//g_stack_instruction.at_r(1) = right;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(2) = name;
				return;
			case ValueType::string:
				return;
			case ValueType::tuple:
			case ValueType::table:
			//case ValueType::native_operator:
			case ValueType::expression:
				memcpy(g_val_mem.content + name.shift + sizeof(void*), g_val_mem.content + name.shift + name.modifier, g_val_mem.max_index - name.shift + name.modifier);
				*(Procedure*)(g_val_mem.content + name.shift) = *(Procedure*)(ptr + 1);
				g_val_mem.max_index += sizeof(void*) - name.modifier;
				
				rightmost.shift += -name.modifier + sizeof(void*);
				g_stack_instruction.at_r(0) = rightmost;
				//right.shift += -name.modifier + sizeof(void*);
				//g_stack_instruction.at_r(1) = right;
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(2) = name;
				return;
			default:
				g_val_mem.replace(ptr + 1, g_specification->type.size[(uint8)*ptr], name.shift, sizeof(void*));
				name.value = *ptr;
				name.modifier = 0;
				g_stack_instruction.at_r(2) = name;
				return;
			}
		}
		else
		{
			g_val_mem.max_index -= sizeof(void*);
			name.value = ValueType::none;
			name.modifier = 0;
			g_stack_instruction.at_r(2) = name;
		}
	}

	void findValueR0R2() { //TODO: this
		findValueR2();
		findValueR0();
	}

	ValueType* findName(String name) {
		Table<String, ValueType*> table = *g_stack_namespace.get(0);

		if (table.count(name))
			return table[name];

		for (int index = g_stack_namespace.max_index; index > 0; --index)
		{
			table = *g_stack_namespace.get(index);
			if (table.count(name))
				return table[name];
		}
		return NULL;
	}


	void conditional() {
		size_t size = g_specification->type.size[(uint8)g_stack_instruction.at_r(0).value];

		uint8* ptr = (g_val_mem.content + g_stack_instruction.at_r(0).shift);

		while (size)
			if (*(ptr + --size) != 0)
			{
				g_val_mem.max_index = g_stack_instruction.at_r(1).shift;
				g_stack_instruction.max_index -= 1;
				g_stack_instruction.at_r(0).instr = InstructionType::skip_after_next;
				g_stack_instruction.at_r(0).modifier = 0;
				return;
			}

		g_val_mem.max_index = g_stack_instruction.at_r(1).shift;
		g_stack_instruction.max_index -= 1;
		g_stack_instruction.at_r(0).instr = InstructionType::skip_next;
		g_stack_instruction.at_r(0).modifier = 0;
	}

	/*void conditional() {
		size_t size = g_specification->type.size[(uint8)g_stack_instruction.at_r(0).value];

		uint8* ptr = (g_val_mem.content + g_stack_instruction.at_r(0).shift);

		while (size)
			if (*(ptr + --size) != 0)
			{
				g_val_mem.max_index = g_stack_instruction.at_r(1).shift;
				g_stack_instruction.max_index -= 1;
				g_stack_instruction.at_r(0).instr = InstructionType::skip_after_next;
				g_stack_instruction.at_r(0).modifier = 0;
				return;
			}

		g_val_mem.max_index = g_stack_instruction.at_r(1).shift;
		g_stack_instruction.max_index -= 1;
		g_stack_instruction.at_r(0).instr = InstructionType::skip_next;
		g_stack_instruction.at_r(0).modifier = 0;
	}*/

	void conditionalTrue() {
		g_val_mem.max_index = g_stack_instruction.at_r(1).shift;
		g_stack_instruction.max_index -= 1;
		g_stack_instruction.at_r(0).instr = InstructionType::skip_after_next;
		g_stack_instruction.at_r(0).modifier = 0;
	}

	void conditionalFalse() {
		g_val_mem.max_index = g_stack_instruction.at_r(1).shift;
		g_stack_instruction.max_index -= 1;
		g_stack_instruction.at_r(0).instr = InstructionType::skip_next;
		g_stack_instruction.at_r(0).modifier = 0;
	}


	void loopWhilePrefix(Instruction arg, Instruction op) {
		g_stack_instruction.add(arg);
		g_stack_instruction.add(op);
		g_op_mem.add(T('.'));
		g_stack_instruction.at_r(3) = Instruction::atom(InstructionType::spacing);
		g_stack_instruction.at_r(4) = Instruction::vs(ValueType::none, 0);
	}
	/*void loopWhilePostfix(Instruction arg, Instruction op) {
		g_stack_instruction.add(arg);
		g_stack_instruction.add(op);
		g_stack_instruction.at_r(3) = Instruction::atom(InstructionType::spacing);
		g_stack_instruction.at_r(4) = Instruction::vs(ValueType::none, 0);
	}*/
	void loopWhile(Instruction condition, Instruction expr) {
		Instruction result = g_stack_instruction.at_r(4);
		ValueType* value;

		switch (condition.value) {
		case ValueType::name:
			value = findName(String((charT*)(g_val_mem.content + condition.shift), condition.modifier));
			switch (g_specification->type.size[(uint32) * value]) {
			case -1:
			case 0:
			case 1:
				if (*(uint8*)(value + 1) == 0)
					goto failed;
				goto succeeded;
			case 2:
				if (*(uint16*)(value + 1) == 0)
					goto failed;
				goto succeeded;
			case 4:
				if (*(uint32*)(value + 1) == 0)
					goto failed;
				goto succeeded;
			case 8:
				if (*(uint64*)(value + 1) == 0)
					goto failed;
				goto succeeded;
			}
		case ValueType::reference:
		case ValueType::object:
			return; //TODO: looks up some overloaded op from base class
		}

	succeeded:

		if (g_specification->type.destructor.count(result.value)) {
			((Destructor)g_specification->type.destructor[result.value])(g_val_mem.content + result.shift);
		}

		g_val_mem.add(expr);

		g_stack_instruction.at_r(0) = Instruction::ivms(InstructionType::call, (ValueType)InstructionCallType::return_value, 0, g_val_mem.max_index);
		g_val_mem.add(g_script);
		g_val_mem.add(g_script_index);
		return;

	failed:

		if (g_specification->type.destructor.count(condition.value)) {
			((Destructor)g_specification->type.destructor[condition.value])(g_val_mem.content + condition.shift);
		}

		g_val_mem.move_absolute(expr.shift, condition.shift);
		g_val_mem.max_index -= result.shift - condition.shift;
		result.shift = condition.shift;
		g_stack_instruction.at_r(4) = result;
		return;

	}
	void loopWhileMoveValue(Instruction value) {
		g_stack_instruction.at_r(4) = value;
		g_stack_instruction.at_r(0) = g_val_mem.get<Instruction>(value.shift - sizeof(Instruction));
	}


	/*void tableGet(Instruction& instr_table, charT* name) {
		ValueType* res = (*g_val_mem.get<Table<String, ValueType*>*>(instr_table.shift))[String(name)];

	}*/
	ValueType* tableAt(Instruction& instr_table, charT* name) {
		auto table = g_val_mem.get<Table<String, ValueType*>*>(instr_table.shift);
		if (table->count(name)) {
			return (*table)[name];
		}
		//table->insert(std::pair<String, ValueType*>(str, &Core::none));
		//g_val_mem.at<void*>(instr_table.shift) = &Core::none;
	}

	void tablePrefix(Instruction& name, Instruction& operation) {
		operation.modifier = 2;
		name.modifier += 1;
		*(g_val_mem.content + name.shift + 1) = ((charT)EntryPrefix::prefix);
	}

	void tablePostfix(Instruction& name, Instruction& operation) {
		operation.modifier = 2;
		name.modifier += 1;
		*(g_val_mem.content + name.shift + 1) = ((charT)EntryPrefix::postfix);
	}


	void getTableEntryLiteral(String str) {
		Instruction& instr_table = g_stack_instruction.at_r(2);
		Table<String, ValueType*>* table = g_val_mem.get<Table<String, ValueType*>*>(instr_table.shift);

		instr_table.value = ValueType::reference;

		if (table->count(str)) {
			*(void**)(g_val_mem.content + instr_table.shift) = (*table)[str];
			//g_memory.at<void*>(instr_table.shift) = (*table)[str];
		}
		else
		{
			table->insert(std::pair<String, ValueType*>(str, &Core::none));
			g_val_mem.at<void*>(instr_table.shift) = &Core::none;
		}
		g_stack_instruction.max_index -= 2;
	}

	void getTableEntry() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getTableEntryLiteral(str);
	}

	void getTablePrefix() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str += ((charT)EntryPrefix::prefix);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getTableEntryLiteral(str);
	}

	void getTablePostfix() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str += ((charT)EntryPrefix::postfix);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getTableEntryLiteral(str);
	}

	void getTableBinaryLeft() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str += ((charT)EntryPrefix::binaryLeft);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getTableEntryLiteral(str);
	}

	void getTableBinaryRight() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str += ((charT)EntryPrefix::binaryRight);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getTableEntryLiteral(str);
	}


	void getNamespaceEntryLiteral(String str) {
		Table<String, ValueType*>* table = g_stack_namespace.get_r(0);

		Instruction last = g_stack_instruction.get_r(0);

		if (table->count(str)) {
			last.value = ValueType::reference;
			auto val = (*table)[str];
			last.modifier = (uint16)*val;
			*(void**)(g_val_mem.content + last.shift) = val;
			g_val_mem.max_index = last.shift + sizeof(void*);

			g_stack_instruction.at_r(1) = last;
			g_stack_instruction.max_index -= 1;
			return;
		}
		else
		{
			Instruction instrOp = g_stack_instruction.get_r(1);
			g_val_mem.move_relative(last.shift, last.modifier, sizeof(charT));
			g_val_mem.max_index += sizeof(charT);
			g_val_mem.at<charT>(last.shift) = (charT)EntryPrefix::prefix;
			last.value = ValueType::name;
			last.modifier += sizeof(charT);

			g_stack_instruction.at_r(1) = last;
			g_stack_instruction.max_index -= 1;
			return;
		}
	}

	void getNamespaceEntry() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getNamespaceEntryLiteral(str);
	}

	void getNamespacePrefix() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str += ((charT)EntryPrefix::prefix);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getNamespaceEntryLiteral(str);
	}

	void getNamespacePostfix() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str += ((charT)EntryPrefix::postfix);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getNamespaceEntryLiteral(str);
	}

	void getNamespaceBinaryLeft() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str += ((charT)EntryPrefix::binaryLeft);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getNamespaceEntryLiteral(str);
	}

	void getNamespaceBinaryRight() {
		Instruction instr_str = g_stack_instruction.get_r(0);
		String str = String();
		str += ((charT)EntryPrefix::binaryRight);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);
		getNamespaceEntryLiteral(str);
	}

	void callObjectPrefix(Instruction obj, Instruction method) {
		Table<String, ValueType*>* table = g_val_mem.get<Table<String, ValueType*>*>(obj.shift);
		String str = String();
		str += ((charT)EntryPrefix::prefix);
		str.append((charT*)(g_op_mem.content + method.shift), method.modifier);

		if (table->count(str))
		{
			ValueType* value = table->at(str);
			switch (*value)
			{
			case ValueType::native_operator:
				Operation op = (*((Operation*)(table->at(str) + 1)));
				eval_prefix(op, obj, method);
				break;
			//case ValueType::ufunction:
			
			}
		}
		else
		{
			g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_invalid_op });
		}
	}

	void callObjectPostfix() {
		Table<String, ValueType*>* table = g_val_mem.get<Table<String, ValueType*>*>(g_stack_instruction.get_r(0).shift);
		//Table<String, ValueType*>* table = (*(Table<String, ValueType*>**)(g_memory.content + g_stack_instruction.get_r(0).shift));
		Instruction instr_str = g_stack_instruction.get_r(1);
		String str = String();
		str += ((charT)EntryPrefix::postfix);
		str.append((charT*)(g_val_mem.content + instr_str.shift), instr_str.modifier);

		if (table->count(str))
		{
			(*((Procedure*)(table->at(str) + 1)))();
		}
		else
		{
			g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_invalid_op });
		}
	}

	void callObjectBinaryLeft() {

	}

	void callObjectBinaryRight() {

	}

	void invokeResolve() {

	}
	void invokeProcedure() {
		((Procedure)g_val_mem.get<void*>(g_stack_instruction.get_r(1).shift))();
	}
	void invokeFunction() {
		Instruction parameter = g_stack_instruction.get_r(0);
		Instruction function = g_stack_instruction.get_r(2);
		
		g_stack_instruction.max_index -= 2;

		g_stack_instruction.add(Instruction::call((ValueType)InstructionCallType::return_value_and_push_slot, parameter.shift));	//TODO: make a 'call' instruction that specifies change of executable string
		g_stack_instruction.add(Instruction::context(parameter.value, parameter.shift));


	}
	/*
	void invokeFunction() {
		Instruction parameter = g_stack_instruction.get_r(0);
		Instruction function = g_stack_instruction.get_r(2);

		/*if (context.value == ValueType::dll)
		{
			GetProcAddress(memory.get<HMODULE>(context.shift), memory.get<String>(function.shift).c_str());
			g_stack_array.get_r(parameter.modifier).content;
		}*/
		/*

		g_stack_instruction.max_index -= 2;

		//TODO: add a function to add value
		g_stack_instruction.add(Instruction::call(parameter.value, parameter.shift));	//TODO: make a 'call' instruction that specifies change of executable string
		g_stack_instruction.add(Instruction::context(parameter.value, parameter.shift));


	}*/
	void invokeNativeFunction() {

	}

	void invokeNativeFunctionUnary() {

	}

	void callFunction(charT* function) {
		Instruction val = g_stack_instruction.get_r(2);
		*(void**)(g_val_mem.content + val.shift) = g_script;
		*(int*)(g_val_mem.content + val.shift + sizeof(void*)) = g_script_index;
		g_val_mem.max_index = val.shift + sizeof(void*) + 4;
		g_script = function;
		val.instr = InstructionType::call;
		val.value = (ValueType)InstructionCallType::return_value_and_push_slot;
		g_stack_instruction.at_r(1) = val;
		g_stack_instruction.at_r(2) = g_stack_instruction.get_r(0);
		g_stack_instruction.max_index -= 1;
		g_script_index = -1;
	}

	/*void callFunction() {
		Instruction val = g_stack_instruction.get_r(2);
		charT* function = *(charT**)(g_val_mem.content + val.shift);
		*(void**)(g_val_mem.content + val.shift) = g_script;
		*(int*)(g_val_mem.content + val.shift + sizeof(void*)) = g_script_index;
		g_val_mem.max_index = val.shift + sizeof(void*) + 4;
		g_script = function;
		val.instr = InstructionType::call;
		g_stack_instruction.at_r(1) = val;
		g_stack_instruction.at_r(2) = g_stack_instruction.get_r(0);
		g_stack_instruction.max_index -= 1;
		g_script_index = -1;
	}*/


	void commaPrefix() { //TODO: redo or prohibit prefix comma.
		if (g_stack_instruction.at_r(2).instr == InstructionType::spacing) {
			Instruction instruction_r0 = g_stack_instruction.get_r(0);
			g_val_mem.move_relative(instruction_r0.shift, g_val_mem.max_index, -(int64)sizeof(charT));

			g_stack_instruction.at_r(3) = g_stack_instruction.at_r(0);
			g_stack_instruction.max_index -= 3;
		}
		else {
			g_stack_instruction.at_r(1) = g_stack_instruction.at_r(0);
			g_stack_instruction.max_index -= 1;
		}
	}

	void commaPostfix() {
		g_val_mem.max_index = g_stack_instruction.at_r(2).shift;
		g_stack_instruction.at_r(2) = g_stack_instruction.at_r(0);
		g_stack_instruction.max_index -= 2;
	}

	void commaBinary() {
		g_val_mem.max_index = g_stack_instruction.at_r(2).shift;
		g_stack_instruction.at_r(2) = g_stack_instruction.at_r(0);
		g_stack_instruction.max_index -= 2;
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
}