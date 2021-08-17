#include "interpreter.h"
#include "core.h"
#include <iostream>
#include <functional>
#include "common_types.h"
#include "common_utils.h"

int main()
{	
	const charT* script;

	//script = T("%[123;name;\"str\"];");
	//script = T("2 + 4");
	script = T("print [3+4;'link'];");

	Program program = Program();

	int result = (int)program.run(script);
	return result;
	/**/
}

String* const Program::EMPTY_STRING = new String(T(""));

Program::Program() {
	data.init();// = *(new Array<Table<String, ValueType*>>());
	data.add(Core::initCoreData());//Table<String, ValueType*>();
	stackInstructions.init();// = *(new Array<Instruction>());
	stackInstructions.add(Instruction::atom(InstructionType::start));
	context = ValueLocation{ 0, ValueType::none };
	specification = Core::initCore();
};

Program::Program(Array<Table<String, ValueType*>> data) {
	this->data = data;
	stackInstructions = Array<Instruction>();
	stackInstructions.add(Instruction::atom(InstructionType::start));
	context = ValueLocation{ 0, ValueType::none };
	specification = Core::initCore();
};

Status Program::run(const charT* script)
{
	String* buffer = new String();
	buffer->reserve(256);
	
	int scriptIndex = -1;

	//temporary variables
	uint8* tmpPtr;
	uint8* findResult;

	Instruction instruction_r0; //Top instruction
	Instruction instruction_r1; //Pre-top instruction
	Instruction instruction_r2; //Pre-pre-top instruction
	Instruction instruction_r3; //Pre-pre-pre-top instruction


	if (!memory.content)
		memory.init(256);

	parse: {
		if (script[++scriptIndex] == 0)
		{
			if (stackInstructions.get_r(0).instr != InstructionType::end)
			{
				//--scriptIndex;
				stackInstructions.add(Instruction::atom(InstructionType::end));
				goto evaluate;
			}
			else
				goto ending;
		}

		switch (script[scriptIndex])
		{
		case T(';'):
			stackInstructions.add(Instruction::atom(InstructionType::separator));
			goto evaluate;

		case T('('):
			stackInstructions.add(Instruction::atom(InstructionType::start_group));
			goto evaluate;

		case T('['):
			stackInstructions.add(Instruction::atom(InstructionType::start_array));
			goto evaluate;

		case T('{'):
			stackInstructions.add(Instruction::atom(InstructionType::start_context));
			goto evaluate;

		case T(')'):
			stackInstructions.add(Instruction::atom(InstructionType::end_group));
			goto evaluate;

		case T(']'):
			stackInstructions.add(Instruction::atom(InstructionType::end_array));
			goto evaluate;

		case T('}'):
			stackInstructions.add(Instruction::atom(InstructionType::end_context));
			goto evaluate;

		case char_space_character:
			--scriptIndex;
			goto spacing;

		case char_operator:
			--scriptIndex;
			goto operation;

		case char_number:
			--scriptIndex;
			goto number;

		case T('"'):
			//Since string starts with " we can omit decrements
			goto string;

		case T('\''):
			//Ditto
			goto link;

		case T('`'):
			//Ditto
			goto expression;

		default:
			--scriptIndex;
			goto name;
		}

		spacing: {
			++scriptIndex;
			switch (script[scriptIndex])
			{
			case char_space_character:
				//Currently MoreINI doesn't specify any behaviour depending on spacing signature, though it's possible to make one.
				goto spacing;

			default:
				--scriptIndex;
				stackInstructions.add(Instruction::atom(InstructionType::spacing));

				goto evaluate;
			}
		}

		operation: {
			++scriptIndex;
			switch (script[scriptIndex])
			{
			case char_operator:
				*buffer += script[scriptIndex];
				goto operation;

			default:
				--scriptIndex;

				//alloc_guard(sizeof(void*));

				stackInstructions.add(Instruction::pos(InstructionType::op, memory.max_index));

				memory.add<uintptr_t>((uintptr_t)buffer);

				//*(uintptr_t*)(memory.data + memory.currentSize) = (uintptr_t)buffer;
				//memory.currentSize += sizeof(void*);

				buffer = new String();

				goto evaluate;
			}
		}

		name: {
			switch (script[++scriptIndex])
			{	// if special character then name finished.
			case char_space_character:
			case char_operator:
			case char_special_character:
			case T('\0'):
				--scriptIndex;

				stackInstructions.add(Instruction::val(ValueType::name, memory.max_index));
				memory.add<uintptr_t>((uintptr_t)buffer);
				buffer = new String();

				goto evaluate;

			default:
				*buffer += script[scriptIndex];
				goto name;
			}
		}

		number: {
			++scriptIndex;
			switch (script[scriptIndex])
			{
			case char_number:
				*buffer += script[scriptIndex];
				goto number;

			default:
				--scriptIndex;

				//alloc_guard(sizeof(int64));

				stackInstructions.add(Instruction::val(ValueType::int64, memory.max_index));

				//*(int*)(memory.data + memory.currentSize) = 5;// = std::stoi(*buffer);

				int64 number = 0;
				for (int i = 0; i < buffer->length(); ++i)
					number *= 10,
					number += ((*buffer)[i] - T('0'));

				memory.add<int64>(number);

				//*(int64*)(memory.data + memory.currentSize) = number;
				//memory.currentSize += sizeof(int64);

				buffer = new String();

				goto evaluate;
			}
		}

		string: {
			++scriptIndex;
			if (script[scriptIndex] == 0)
				goto error_string_missing_closing_quote;

			switch (script[scriptIndex])
			{
			case T('"'):
				stackInstructions.add(Instruction::val(ValueType::string, memory.max_index));
				memory.add<uintptr_t>((uintptr_t)buffer);
				buffer = new String();
				goto evaluate;

			case T('\0'):
				goto error_syntax;

			default:
				*buffer += script[scriptIndex];
				goto string;
			}
		}

		link: {
			++scriptIndex;
			if (script[scriptIndex] == 0)
				goto error_string_missing_closing_quote;

			switch (script[scriptIndex])
			{
			case T('\''):
				stackInstructions.add(Instruction::val(ValueType::link, memory.max_index));
				memory.add<uintptr_t>((uintptr_t)buffer);
				buffer = new String();
				goto evaluate;

			case T('\0'):
				goto error_syntax;

			default:
				*buffer += script[scriptIndex];
				goto link;
			}
		}

		expression: {
			++scriptIndex;
			if (script[scriptIndex] == 0)
				goto error_string_missing_closing_quote;

			switch (script[scriptIndex])
			{
			case T('`'):
				stackInstructions.add(Instruction::val(ValueType::expression, memory.max_index));
				memory.add<uintptr_t>((uintptr_t)buffer);
				buffer = new String();
				goto evaluate;

			case T('\0'):
				goto error_syntax;

			default:
				*buffer += script[scriptIndex];
				goto expression;
			}
		}
	}

    evaluate: {
		instruction_r0 = *(stackInstructions.content + stackInstructions.max_index);
		instruction_r1 = *(stackInstructions.content + stackInstructions.max_index - 1);
		instruction_r2 = *(stackInstructions.content + stackInstructions.max_index - 2);

                                                                              //   Decision tree 
        switch (instruction_r0.instr) {                                       //    ___________
        case InstructionType::start: goto parse;                              //   |   |   |s t| :parse
        case InstructionType::op: goto parse;                                 //   |   |   |o p| :parse
		case InstructionType::separator:
			switch (instruction_r1.instr) {                                   //   |   | x | ; |
			case InstructionType::separator: goto eval_array_add_empty;       //   |   | ; | ; | :eval_array_add_empty
			case InstructionType::start_array: goto eval_array_add_empty;     //   |   | [ | ; | :eval_array_add_empty
			case InstructionType::op: goto eval_postfix;                      //   |   |o p| ; | :eval_postfix		//TODO: Better to do another level to check for val
			case InstructionType::value:                                      
				switch (instruction_r2.instr) {                               //   | x |val| ; |
				case InstructionType::start: goto parse;                      //   |s t|val| ; | :parse
				case InstructionType::op: goto eval_prefix;                   //   |o p|val| ; | :eval_prefix
				case InstructionType::start_array: goto eval_array_add;       //   | [ |val| ; | :eval_array_add
				case InstructionType::spacing: goto eval_binary_long;         //   | _ |val| ; | :eval_array_add
				default: goto error_syntax; }
			default: goto error_syntax; }
        case InstructionType::start_group: goto parse;                        //   |   |   | ( | :parse
		case InstructionType::end_group:                                      //   |   | x | ) |
			switch (instruction_r1.instr) {
			case InstructionType::value:                                      //   | x |val| ) |
				switch (instruction_r2.instr) {
				case InstructionType::start_group: goto eval_group;           //   | ( |val| ) | :eval_group
				case InstructionType::spacing: goto eval_binary_long;         //   | _ |val| ) | :eval_binary_long
				default: goto error_syntax; }
			default: goto error_syntax; }
		case InstructionType::start_array: goto eval_array_start;             //   |   |   | [ | :eval_array_start
		case InstructionType::end_array:                                      
			switch (instruction_r1.instr) {                                   //   |   | x | ] |
			case InstructionType::start_array: goto eval_array_empty;         //   |   | [ | ] | :eval_array_empty
			case InstructionType::value: goto eval_array_end;                 //   |   |val| ] | :eval_array_end	//TODO: eval_binary_long
			default: goto error_syntax;}
		case InstructionType::start_context:                                  
            switch (instruction_r1.instr) {                                   //   |   | x | { |
            case InstructionType::spacing: goto eval_context_new;             //   |   | _ | { | :eval_context_new
            case InstructionType::value: goto eval_context_of;                //   |   |val| { | :eval_context_of
			case InstructionType::context: goto parse;                        //   |   |ctx| { | :parse
            default: goto error_syntax; }
        case InstructionType::end_context:                                    //   |   | x | } |
            switch (instruction_r1.instr) {
            case InstructionType::value: goto eval_context_finish;            //   | x |val| } |
                switch (instruction_r2.instr) {
                case InstructionType::op: goto eval_prefix;                   //   |o p|val| } | :eval_prefix
                case InstructionType::start_context: goto eval_context_finish;//   | { |val| } | :eval_context_finish
                default: goto error_syntax; }
            case InstructionType::op:                                         //   | x |o p| } |
                switch (instruction_r2.instr) {
                case InstructionType::value: goto eval_postfix;               //   |val|o p| } | :eval_postfix
                default: goto error_syntax; }
            default: goto error_syntax; }
        case InstructionType::end:                                            //   |   | x |end|
			switch (instruction_r1.instr) {
			case InstructionType::start: goto parse;                          //   |   |s t|end| :eval_add_none
			case InstructionType::separator: goto eval_cleanup_separator;     //   |   | ; |end| :eval_cleanup_separator //TODO: implement
			case InstructionType::op:                                         //   | x |o p|end|
				switch (instruction_r2.instr) {
				case InstructionType::spacing: goto parse;                    //   | _ |o p|end| :parse
				case InstructionType::value: goto eval_postfix;               //   |val|o p|end| :eval_postfix
				default: goto error_syntax;	}
			case InstructionType::value:                                      //   | x |val|end|
				switch (instruction_r2.instr) {
				case InstructionType::start: goto ending;                     //   |s t|val|end| :ending
				case InstructionType::spacing: goto eval_binary_long;         //   | _ |val|end| :eval_binary_long
				case InstructionType::op: goto eval_prefix;	}                 //   |o p|val|end| :eval_prefix	//Possibly unreachable
			default: goto error_syntax;
			}
        case InstructionType::spacing:                                        //   |   | x | _ |
            switch (instruction_r1.instr) {
            case InstructionType::start: goto parse;                          //   |   |s t| _ | :parse
            case InstructionType::op:                                         //   | x |o p| _ |
                switch (instruction_r2.instr) {
                case InstructionType::spacing: goto parse;                    //   | _ |o p| _ | :parse
                case InstructionType::value: goto eval_postfix;               //   |val|o p| _ | :eval_postfix
                default: goto error_syntax; }
            case InstructionType::value:                                      //   | x |val| _ | 
                switch (instruction_r2.instr) {
                case InstructionType::start: goto parse;                      //   |s t|val| _ | :parse
				case InstructionType::start_context: goto parse;              //   | { |val| _ | :parse
				case InstructionType::start_array: goto parse;                //   | [ |val| _ | :parse
				case InstructionType::start_group: goto parse;                //   | ( |val| _ | :parse
                case InstructionType::spacing: goto eval_binary_long;         //   | _ |val| _ | :eval_binary_long
                case InstructionType::op: goto eval_prefix; }                 //   |o p|val| _ | :eval_prefix	//Possibly unreachable
                default: goto error_syntax; }
        case InstructionType::value:                                          //   |   | x |val|
            switch (instruction_r1.instr) {
			case InstructionType::start_context: goto parse;                  //   |   | { |val| :parse
			case InstructionType::start_array: goto parse;                    //   |   | [ |val| :parse
			case InstructionType::start_group: goto parse;                    //   |   | ( |val| :parse
            case InstructionType::start: goto parse;                          //   |   |s t|val| :parse
            case InstructionType::value: goto eval_coalescing;                //   |   |val|val| :eval_coalescing
			case InstructionType::spacing: goto parse;                        //   |   | _ |val| :parse
            //    switch (instruction_r2.instr) {
            //    case InstructionType::value: goto eval_binary_coalescing;     //   |val| _ |val| :eval_binary_coalescing //TODO: Priority at the right operand
            //    case InstructionType::op: goto parse;                         //   |o p| _ |val| :parse //Possibly unreachable
            //    default: goto error_syntax; }
            case InstructionType::op:                                         //   | x |o p|val|
                switch (instruction_r2.instr) {
                case InstructionType::value: goto eval_binary;                //   |val|o p|val| :eval_binary
                case InstructionType::start:                                  //   |s t|o p|val| :eval_prefix
                case InstructionType::start_group:                            //   | ( |o p|val| :eval_prefix
                case InstructionType::start_array:                            //   | [ |o p|val| :eval_prefix
                case InstructionType::start_context:                          //   | { |o p|val| :eval_prefix
                case InstructionType::spacing:                                //   | { |o p|val| :eval_prefix
                case InstructionType::separator: goto eval_prefix;            //   | ; |o p|val| :eval_prefix   //Possibly unreachable
                default: goto error_syntax; }
            default: goto error_syntax; }
        default: goto error_syntax; }


		eval_context_new: {
			stackInstructions.add(Instruction::context(context.value, context.shift));

			memory.add<void*>((void*)new Table<String, ValueType*>());
			//*(void**)(memory.data + memory.currentSize) = new Table<String, ValueType*>();
			//memory.currentSize += sizeof(void*);

			stackInstructions.add(Instruction::atom(InstructionType::start_context));

			goto parse;
		}

		eval_context_of: {
			stackInstructions.at_r(0) = Instruction::context(context.value, context.shift);

			context.shift = instruction_r1.shift;
			context.value = instruction_r2.value;

			stackInstructions.add(Instruction::atom(InstructionType::start_context));

			goto parse;
		}

		eval_context_finish: {
			/*context = stack[iterator - 2].location;
			stack[iterator - 2] = stack[iterator - 1];
			stack.pop_back();*/
			context = stackInstructions.get_r(3).location;
			stackInstructions.get_r(3) = instruction_r1;

			stackInstructions.max_index -= 3;// resize(iterator - 4);
			goto parse;
		}

		eval_array_start: {
			++stackArrays.max_index;
			stackArrays.at_r(0).init();//.add(*(new Array<Instruction>()));
			//stackArrays.getr_u(0).reserve(16); //TODO: initial capacity
			//stackInstructions.add(Instruction::atom(InstructionType::start_array));
			stackInstructions.at_r(0).shift = stackArrays.max_index;
			goto parse;
		}

		eval_array_add: {
			stackArrays[instruction_r2.shift].add(instruction_r1);
			//memory.max_index -= specification.typeSize[instruction_r1.value];
			stackInstructions.max_index -= 2;
			goto parse;
		}

		eval_array_add_empty: {
			stackArrays[instruction_r2.shift].add(Instruction::val(ValueType::arr, 0));
			stackInstructions.max_index -= 1;
			goto parse;
		}

		eval_array_empty: {
			stackInstructions.max_index -= 1;
			stackInstructions.at_r(0).instr = InstructionType::value;
			goto parse;
		}

		eval_array_end: {
			stackArrays[instruction_r2.shift].add(instruction_r1);
			//memory.max_index -= specification.typeSize[instruction_r1.value];
			stackInstructions.max_index -= 2;
			stackInstructions.at_r(0).instr = InstructionType::value;
			stackInstructions.at_r(0).value = ValueType::arr;
			goto evaluate; //goto parse;
		}

		eval_group: {
			stackInstructions.at_r(2) = instruction_r1;
			stackInstructions.max_index -= 2;

			goto evaluate;
		}

		eval_prefix: {
			tmpPtr = memory.at<uint8*>(instruction_r1.shift);

			if (!this->specification.opPrefix.count(*(String*)tmpPtr)) 
				goto error_syntax;

			Procedure lc_value;

			if (!this->specification.opPrefix[(*(String*)tmpPtr)].count(instruction_r0.value))
				if (this->specification.opPrefix[(*(String*)tmpPtr)].count(ValueType::all))
					goto error_syntax;
				else
					lc_value = this->specification.opPrefix[(*(String*)tmpPtr)][ValueType::all];
			else
				lc_value = this->specification.opPrefix[(*(String*)tmpPtr)][instruction_r0.value];

			lc_value(*this);

			goto error_syntax;	// TODO: error operator was not found
		}

		eval_postfix: {
			tmpPtr = memory.at<uint8*>(instruction_r1.shift);

			if (!this->specification.opPostfix.count(*(String*)tmpPtr))
				goto error_syntax;

			Procedure lc_value;

			if (!this->specification.opPostfix[(*(String*)tmpPtr)].count(instruction_r2.value))
				if (!this->specification.opPostfix[(*(String*)tmpPtr)].count(ValueType::all))
					goto error_syntax;
				else
					lc_value = this->specification.opPostfix[(*(String*)tmpPtr)][ValueType::all];
			else
				lc_value = this->specification.opPostfix[(*(String*)tmpPtr)][instruction_r2.value];

			lc_value(*this);

			goto parse;
		}

		eval_binary: {
			tmpPtr = memory.at<uint8*>(instruction_r1.shift); //Pointer to pointer

			if (!this->specification.opBinary.count(*(String*)tmpPtr))
				goto error_syntax;

			if (!this->specification.opBinary[(*(String*)tmpPtr)].count(ValueTypeBinary(instruction_r2.value, instruction_r0.value)))
				goto error_syntax;

			Procedure lc_value = this->specification.opBinary[(*(String*)tmpPtr)][ValueTypeBinary(instruction_r2.value, instruction_r0.value)];


			if (!this->specification.opBinary[(*(String*)tmpPtr)].count(ValueTypeBinary(instruction_r2.value, instruction_r0.value)))
				if (!this->specification.opBinary[(*(String*)tmpPtr)].count(ValueTypeBinary(instruction_r2.value, ValueType::all)))
					if (!this->specification.opBinary[(*(String*)tmpPtr)].count(ValueTypeBinary(ValueType::all, instruction_r0.value)))
						if (!this->specification.opBinary[(*(String*)tmpPtr)].count(ValueTypeBinary(ValueType::all, ValueType::all)))
							goto error_syntax;
						else
							lc_value = this->specification.opBinary[(*(String*)tmpPtr)][ValueTypeBinary(ValueType::all, ValueType::all)];
					else
						lc_value = this->specification.opBinary[(*(String*)tmpPtr)][ValueTypeBinary(ValueType::all, instruction_r0.value)];
				else
					lc_value = this->specification.opBinary[(*(String*)tmpPtr)][ValueTypeBinary(instruction_r2.value, ValueType::all)];
			else
				lc_value = this->specification.opBinary[(*(String*)tmpPtr)][ValueTypeBinary(instruction_r2.value, instruction_r0.value)];

			lc_value(*this);

			goto evaluate;
		}

		eval_binary_long: {
			
			instruction_r3 = stackInstructions.get_r(3);

			--scriptIndex;

			switch (instruction_r3.instr) {
			case InstructionType::value:

				// val sp val x
				stackInstructions.at_r(2) = instruction_r1;
				stackInstructions.max_index -= 2;

				instruction_r0 = instruction_r1;
				instruction_r1 = instruction_r3;

				goto eval_coalescing;

			case InstructionType::op:
				// [val sp] op sp val x
				stackInstructions.at_r(4) = instruction_r3;
				stackInstructions.at_r(3) = instruction_r1;
				stackInstructions.max_index -= 3;

				instruction_r0 = instruction_r1;
				instruction_r1 = instruction_r3;
				instruction_r2 = stackInstructions.at_r(2);

				goto eval_binary;
			default:
				goto error_syntax;
			}
		}

		eval_coalescing: {

			//if(left == arr, right == arr)
			//	max_index
			//if(left == arr, right != arr)
			//	max_index
			//if(left != arr, right == arr)
			//	right.shift
			//if(left != arr, right != arr)
			//	right.shift, right>>
			
			Procedure lc_value;// = this->specification.binary[(*(String*)tmpPtr)][ValueTypeBinary(instruction_r2.value, instruction_r0.value)];

			if (!this->specification.opCoalescing.count(ValueTypeBinary(instruction_r1.value, instruction_r0.value)))
				if (!this->specification.opCoalescing.count(ValueTypeBinary(instruction_r1.value, ValueType::all)))
					if (!this->specification.opCoalescing.count(ValueTypeBinary(ValueType::all, instruction_r0.value)))
						if (!this->specification.opCoalescing.count(ValueTypeBinary(ValueType::all, ValueType::all)))
							goto error_syntax;
						else
							lc_value = this->specification.opCoalescing[ValueTypeBinary(ValueType::all, ValueType::all)];
					else
						lc_value = this->specification.opCoalescing[ValueTypeBinary(ValueType::all, instruction_r0.value)];
				else
					lc_value = this->specification.opCoalescing[ValueTypeBinary(instruction_r1.value, ValueType::all)];
			else
				lc_value = this->specification.opCoalescing[ValueTypeBinary(instruction_r1.value, instruction_r0.value)];

			lc_value(*this);

			goto evaluate;

		/*
			if (instruction_r0.value == ValueType::arr) 
				if (instruction_r1.value == ValueType::arr)
				{
					stackInstructions.at_r(0) = Instruction::pos(InstructionType::op, memory.max_index); // +
					memory.insert<String*>(Program::EMPTY_STRING, memory.max_index);
				}
				else 
				{
					stackInstructions.at_r(0) = Instruction::pos(InstructionType::op, memory.max_index); //+
					memory.insert<String*>(Program::EMPTY_STRING, memory.max_index);
				}
			else
				if (instruction_r1.value == ValueType::arr) 
				{
					memory.insert<String*>(Program::EMPTY_STRING, instruction_r0.shift);
					stackInstructions.at_r(0) = Instruction::pos(InstructionType::op, instruction_r0.shift);
					instruction_r0.shift += sizeof(String*);	// +
				}
				else
				{
					memory.insert<String*>(Program::EMPTY_STRING, instruction_r0.shift);
					stackInstructions.at_r(0) = Instruction::pos(InstructionType::op, instruction_r0.shift);
					instruction_r0.shift += sizeof(String*);	// +
				}

			stackInstructions.add(instruction_r0);

			instruction_r2 = instruction_r1;
			instruction_r1 = stackInstructions.get_r(1);

			goto eval_binary;
			*/
		}

		eval_coalescing_long: {
		/*
			this->memory.insert<String*>(Program::EMPTY_STRING, instruction_r0.shift);
			instruction_r1.instr = InstructionType::op;
			instruction_r1.shift = instruction_r0.shift;
			this->stackInstructions.at_r(0).shift += sizeof(void*);
			this->stackInstructions.at_r(1) = instruction_r1;
			goto eval_binary;*/
			stackInstructions.at_r(1) = instruction_r0;
			--stackInstructions.max_index;
			
			goto eval_coalescing;
		}

		eval_cleanup_separator: {
			stackInstructions.at_r(1) = instruction_r0;
			--stackInstructions.max_index;

			goto evaluate;
		}
	}

	//Errors
	{
	error_memory_allocation: //TODO: Before throwing error should try freeing up memory
		return Status::error_memory_allocation;

	error_string_missing_closing_quote:
		return Status::error_string_missing_closing_quote;

	error_operator_name_exceeded:
		return Status::error_operator_name_exceeded;

	error_syntax:
		return Status::error_syntax;
	}

ending:
	return Status::success;
}

void print(ValueType* memory)
{
	//std::unordered_map<String, ValueType*> dict = std::unordered_map<String, ValueType*>();

	switch (*memory)
	{
	case ValueType::dict:
		//std::unordered_map<String, ValueType*> dict = **((std::unordered_map<String, ValueType*>**)(memory + 1));
		std::cout << T("{ ");
		//for (auto var = dict.begin(); var != dict.end(); ++var)
		for(auto var: **((Table<String, ValueType*>**)(memory + 1)))
		{
			std::cout << var.first.c_str() << T(": ");
			print(var.second);
			std::cout << T("; ");
		}
		std::cout << T("} ");
		return;

	case ValueType::int64:
		std::cout << *((__int64*)(memory + 1));
		return;

	case ValueType::float64:
		std::cout << *((double*)(memory + 1));
		return;

	case ValueType::string:
		//std::cout << *(std::wstring**)(memory + 1);
		std::cout << (*(String*)(*(String**)(memory + 1))).c_str();
		return;
	}
}