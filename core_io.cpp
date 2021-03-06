#include "core.h"
#include <iostream>

namespace Core {
	String fromInt(int64 num) {
		if (num == 0)
			return "0";

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
		if (num == 0)
			return "0";

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
			return fromUint(*(uint8*)(value + 1));
		case ValueType::uint16:
			return fromUint(*(uint16*)(value + 1));
		case ValueType::uint32:
			return fromUint(*(uint32*)(value + 1));
		case ValueType::uint64:
			return fromUint(*(uint64*)(value + 1));
		case ValueType::string:
		case ValueType::name:
			return String((charT*)(value + 3), (*(uint16*)(value + 1)));
		case ValueType::expression:
			result.append(*(charT**)(value + 1));
			return result;
		case ValueType::table:
			result.append(T("{\n"));
			for (auto var : **(Table<String, ValueType*>**)(value + 1))
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
			return fromInt(*(int8*)(g_val_mem.content + instruction.shift));
		case ValueType::int16:
			return fromInt(*(int16*)(g_val_mem.content + instruction.shift));
		case ValueType::int32:
			return fromInt(*(int32*)(g_val_mem.content + instruction.shift));
		case ValueType::int64:
			return fromInt(*(int64*)(g_val_mem.content + instruction.shift));
		case ValueType::uint8:
			return fromUint(*(uint8*)(g_val_mem.content + instruction.shift));
		case ValueType::uint16:
			return fromUint(*(uint16*)(g_val_mem.content + instruction.shift));
		case ValueType::uint32:
			return fromUint(*(uint32*)(g_val_mem.content + instruction.shift));
		case ValueType::uint64:
			return fromUint(*(uint64*)(g_val_mem.content + instruction.shift));
		case ValueType::string:
		case ValueType::name:
			return String((charT*)(g_val_mem.content + instruction.shift), instruction.modifier);
		case ValueType::table:
		case ValueType::object:
			result.append(T("{\n"));
			for (auto var : **(Table<String, ValueType*>**)(g_val_mem.content + instruction.shift))
			{
				result.append(var.first);
				result.append(T(" : "));
				result.append(toStringGlobal(var.second));
				result.append(T("; "));
			}
			result.append(T("\n}"));
			return result;
		case ValueType::tuple:
			result.append(T("["));
			{
				Array<Instruction> tuple = *(Array<Instruction>*)g_val_mem.get<void*>(instruction.shift);
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

			result.append(toStringGlobal(*(ValueType**)(g_val_mem.content + instruction.shift)));

			//result.append(fromInt(*(uintptr_t*)(memory.content + instruction.shift)));
			//result.append("(");
			//result.append("ref(");
			//result.append(toStringGlobal(**(ValueType***)(g_memory.content + instruction.shift)));
			//result.append(")");
			return result;
		case ValueType::native_operator:
			result.append("Native procedure 0x");
			result.append((charT*)(g_val_mem.content + instruction.shift), g_specification->type.size[(uint8)instruction.value]);
			return result;
		default:
			result.append((charT*)(g_val_mem.content + instruction.shift), g_specification->type.size[(uint8)instruction.value]);
			return result;
		}
	}

	String toStringLocalArray(Instruction instruction) {
		String result = String();

		result.append(T("["));
		{
			Array<Instruction> tuple = *(Array<Instruction>*)g_val_mem.get<void*>(instruction.shift);
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

	void print(Instruction instr) {
		ToStringLocal function = 0;
		if (g_specification->type.stringLocal.count(instr.value))
			function = g_specification->type.stringLocal.at(instr.value);
		else if (g_specification->type.stringLocal.count(ValueType::all))
			function = g_specification->type.stringLocal.at(ValueType::all);
#if ENCODING == 8
		std::cout << function(instr);
#elif ENCODING == 16
		std::wcout << function(program, instr);
#endif
	}

	void scan() {
		String input = String();

		Instruction instruction_r0 = g_stack_instruction.get_r(0);
		Instruction instruction_r1 = g_stack_instruction.get_r(1);

		ValueType* ref = *(ValueType**)(g_val_mem.content + instruction_r0.shift);

#if ENCODING == 8
		switch (*ref) {
		case ValueType::int64: {
			int64 result;
			std::cin >> result;
			*(int64*)(ref + 1) = result;
			break;}
		case ValueType::string: {
			String result;
			std::cin >> result;
			*(String*)(ref + 1) = result;
			break;}
		}
#elif ENCODING == 16
#error Not implemented yet.
#elif ENCODING == 32
#error Not implemented yet.
#endif
		instruction_r0.shift -= instruction_r1.modifier;
		g_stack_instruction.at_r(1) = instruction_r0;
		--g_stack_instruction.max_index;
	}

	//std::ifstream getFile(Instruction& path) {

	//}
}