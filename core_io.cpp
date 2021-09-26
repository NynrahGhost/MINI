#include "core.h"
#include <iostream>

namespace Core {
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
			return String((charT*)(value + 3), (*(uint16*)(value + 1)));
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
		case ValueType::tuple:
			result.append(T("["));
			for (int i = 0; i < (*(Array<ValueType*>*)(value + 1)).max_index; ++i)
			{
				result.append(toStringGlobal((*(Array<ValueType*>*)(value + 1)).get(i)));
				result.append(T("; "));
			}
			result.append(T("]"));
			return result;
		default:
			result.append((charT*)(value + 1), g_specification->type.size[(uint8)*value]);
			return result;
		}
	}

	String toStringLocal(Instruction instruction) {
		String result = String();

		void* tmp;

		switch (instruction.value) {
		case ValueType::int8:
			return fromInt(*(int8*)(g_memory.content + instruction.shift));
		case ValueType::int16:
			return fromInt(*(int16*)(g_memory.content + instruction.shift));
		case ValueType::int32:
			return fromInt(*(int32*)(g_memory.content + instruction.shift));
		case ValueType::int64:
			return fromInt(*(int64*)(g_memory.content + instruction.shift));
		case ValueType::uint8:
			return fromUint(*(uint8*)(g_memory.content + instruction.shift));
		case ValueType::uint16:
			return fromUint(*(uint16*)(g_memory.content + instruction.shift));
		case ValueType::uint32:
			return fromUint(*(uint32*)(g_memory.content + instruction.shift));
		case ValueType::uint64:
			return fromUint(*(uint64*)(g_memory.content + instruction.shift));
		case ValueType::string:
		case ValueType::name:
		case ValueType::link:
			return String((charT*)(g_memory.content + instruction.shift), instruction.modifier);
		case ValueType::dict:
			result.append(T("{\n"));
			for (auto var : *(Table<String, ValueType*>*)(g_memory.content + instruction.shift))
			{
				result.append(var.first);
				result.append(T(" : "));
				result.append(toStringGlobal(var.second));
				result.append(T("; "));
			}
			result.append(T("}"));
			return result;
		case ValueType::tuple:
			result.append(T("["));
			{
				Array<Instruction> tuple = *(Array<Instruction>*)g_memory.get<void*>(instruction.shift);
				Instruction element;
				for (int i = 0; i < tuple.max_index; ++i)
				{
					element = tuple.get(i);
					element.shift += instruction.shift + sizeof(void*);
					result.append(toStringLocal(element));
					result.append(T("; "));
				}
				element = tuple.get_r(0);
				element.shift += instruction.shift + sizeof(void*);
				result.append(toStringLocal(element));
			}
			result.append(T("]"));
			return result;
		case ValueType::reference:
			//result.append(fromInt(*(uintptr_t*)(memory.content + instruction.shift)));
			//result.append("(");
			//result.append("ref(");
			result.append(toStringGlobal(**(ValueType***)(g_memory.content + instruction.shift)));
			//result.append(")");
			return result;
		default:
			result.append((charT*)(g_memory.content + instruction.shift), g_specification->type.size[(uint8)instruction.value]);
			return result;
		}
	}

	String toStringLocalArray(Instruction instruction) {
		String result = String();

		result.append(T("["));
		{
			Array<Instruction> tuple = *(Array<Instruction>*)g_memory.get<void*>(instruction.shift);
			for (int i = 0; i < tuple.max_index; ++i)
			{
				result.append(toStringLocal(tuple.get(i)));
				result.append(T("; "));
			}
			result.append(toStringLocal(tuple.get_r(0)));
		}
		result.append(T("]"));

		return result;
	}

	void print() {
		Instruction instruction_r0 = g_stack_instruction.get_r(0);
		Instruction instruction_r1 = g_stack_instruction.get_r(1);
		ToStringLocal function = 0;
		if (g_specification->type.stringLocal.count(instruction_r0.value))
			function = g_specification->type.stringLocal.at(instruction_r0.value);
		else if (g_specification->type.stringLocal.count(ValueType::all))
			function = g_specification->type.stringLocal.at(ValueType::all);
#if ENCODING == 8
		std::cout << function(instruction_r0);
#elif ENCODING == 16
		std::wcout << function(program, instruction_r0);
#endif
		g_memory.move_absolute(instruction_r0.shift, g_memory.max_index, instruction_r1.shift);
		g_stack_instruction.at_r(1) = instruction_r0;
		--g_stack_instruction.max_index;
	}

	void scan() {
		String input = String();
#if ENCODING == 8
		std::getline(std::cin, input);
#elif ENCODING == 16
		std::getline(std::wcin, input); //Doesn't work yet
#elif ENCODING == 32
#error Not implemented yet.
#endif
		Instruction instruction_r0 = g_stack_instruction.get_r(0);
		Instruction instruction_r1 = g_stack_instruction.get_r(1);

		ValueType*** ref = (ValueType***)(g_memory.content + instruction_r0.shift);

		ValueType* ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(uint16) + input.size() - 1);

		*ptr = ValueType::string;
		*(uint16*)(ptr + 1) = input.size();
		memcpy(ptr + sizeof(ValueType) + sizeof(uint16), input.data(), input.size());

		if (!g_specification->type.destructor.count(***ref))
			if (!g_specification->type.destructor.count(ValueType::all))
				return;
			else
				((Destructor)g_specification->type.destructor[ValueType::all])(**ref);
		else
			((Destructor)g_specification->type.destructor[***ref])(**ref);

		**ref = ptr;

		g_memory.move_absolute(instruction_r0.shift, g_memory.max_index, instruction_r1.shift);
		//memory.move_relative(instruction_r0.shift, specification->type.size[instruction_r0.value], -(int64)sizeof(String**));
		g_stack_instruction.at_r(1) = instruction_r0;
		--g_stack_instruction.max_index;
	}
}