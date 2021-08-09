#include "interpreter.h"
#include "core.h"
#include <iostream>
#include <functional>
#include "common_types.h"
#include "common_utils.h"

int main()
{	
	//std::cout << murmurHash64(new uint8[7]{ 0,1,2,3,4,5 }, 7, 0);
	
	const charT* script;

	//script = T("%[123;name;\"str\"];");
	script = T("%123;");
	//script = T("123+133");

	Program program = Program();

	int result = (int)program.run(script);
	return result;
	/**/
}

uint32 test0(uint16 arg0, uint32 arg1, uint16 arg2, float32 arg3, float64 arg4)
{
	return arg0 + arg1 + arg2 + arg3 + arg4;
}


/*
uint8* allocator;

Table<String, ValueType*>* memory = new Table<String, ValueType*>();

allocator = (uint8*)malloc(sizeof(long long) + sizeof(ValueType));
*(ValueType*)allocator = ValueType::int64;
*((long long*)(allocator + sizeof(ValueType))) = 1234l;
//memory.insert_or_assign(String(T("number")).substr(0,6), (ValueType*)allocator);
//memory.insert_or_assign(String(T("config")), (ValueType*)(uint8*)malloc(sizeof(void*) + 1));

allocator = (uint8*)malloc(sizeof(long long) + sizeof(ValueType));
*(ValueType*)allocator = ValueType::int64;
*((long long*)(allocator + sizeof(ValueType))) = 1234l;
memory->insert_or_assign(String(T("number")), (ValueType*)allocator);

allocator = (uint8*)malloc(sizeof(void*) + sizeof(ValueType));
*(ValueType*)allocator = ValueType::string;
*((String**)(allocator + sizeof(ValueType))) = new String(T("Hello, World!"));
memory->insert_or_assign(String(T("msg")), (ValueType*)allocator);

Table<String, ValueType*> mry = Table<String, ValueType*>();

allocator = (uint8*)malloc(sizeof(void*) + sizeof(ValueType));
*(ValueType*)allocator = ValueType::dict;
*(Table<String, ValueType*>**)(allocator+1) = memory;

auto val = **(Table<String, ValueType*>**)(allocator+1);

mry.insert_or_assign(String(T("config")), (ValueType*)allocator);//(ValueType*)(new containerPtr(ValueType::dict, (void*)memory)));

Array<Table<String, ValueType*>> namespaces = Array<Table<String, ValueType*>>();
namespaces.add(mry);

auto mem = Array<Table<String, ValueType*>>();
mem.add(mry);
*/

/*
	struct tmp0 {
		int64 _0 = 56;
		int64 _1 = 1;
	} tmp0;

	struct tmp1 {
		int32 _0 = 1;
		int64 _1;
		int32 _2;
	} tmp1;
	void* address = test0;

	//reinterpret_cast<void (*)(int64)>(address) ((int64)(uintptr_t)memory[T("string")]);

	//int32 result = reinterpret_cast<uint16 (*)(uint16, uint16)>(address) (65535,1);
	//int32 result = reinterpret_cast<int32(*)(struct alignas(2) {}) > (address)(123);		//	So, if you pass a bigger type as an argument
	//																						//	then it's possible to correctly call the function

	//print(mry[T("config")]);

	uint32 result = reinterpret_cast<uint32 (*)(uint16, uint32, uint16, float32, float64)>(address) (0, 1, 2, 3, 4);
	//*(float64*)(_alloca(8)) = 4.0;
	//uint32 result = reinterpret_cast<uint32(*)(uint16, uint32, uint16, float32)>(address) (0, 1, 2, 3);

	result = test0(0, 1, 2, 3, 4);*/


Program::Program() {
	data.init();// = *(new Array<Table<String, ValueType*>>());
	data[0] = Table<String, ValueType*>();
	stackInstructions.init();// = *(new Array<Instruction>());
	stackInstructions.add(Instruction::atom(InstructionType::start));
	context = ValueLocation{ 0, ValueType::none };
	specification = Core::getCore();
};

Program::Program(Array<Table<String, ValueType*>> data) {
	this->data = data;
	stackInstructions = Array<Instruction>();
	stackInstructions.add(Instruction::atom(InstructionType::start));
	context = ValueLocation{ 0, ValueType::none };
	specification = Core::getCore();
};

Status Program::run (const charT* script)
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


	if (!memory.content)
		memory.init(256);

	/*if (!memory.data)
		if (!(memory.data = (uint8*)malloc(256)))
			goto error_memory_allocation;
		else
			memory.maxSize = 256;*/

	parse: {
		if (script[++scriptIndex] == 0)
		{
			if (stackInstructions.get_r(0).instr != InstructionType::end)
			{
				--scriptIndex;
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

				//alloc_guard(sizeof(void*));

				stackInstructions.add(Instruction::val(ValueType::name, memory.max_index));

				memory.add<uintptr_t>((uintptr_t)buffer);

				//*(uintptr_t*)(memory.data + memory.currentSize) = (uintptr_t)buffer;
				//memory.currentSize += sizeof(void*);

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

				//alloc_guard(sizeof(void*));

				stackInstructions.add(Instruction::val(ValueType::string, memory.max_index));// (ValueType::string, memory.currentSize));

				memory.add<uintptr_t>((uintptr_t)buffer);
				//*(uintptr_t*)(memory.data + memory.currentSize) = (uintptr_t)buffer;
				//memory.currentSize += sizeof(void*);

				buffer = new String();

				goto evaluate;

			case T('\0'):
				goto error_syntax;

			default:
				buffer += script[scriptIndex];
				goto string;
			}
		}

		link: {}

		expression: {}
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
				case InstructionType::op: goto eval_prefix;                   //   |o p|val| ; | :eval_prefix
				case InstructionType::start_array: goto eval_array_add;       //   | [ |val| ; | :eval_array_add
				default: goto error_syntax; }
			default: goto error_syntax; }
        case InstructionType::start_group: goto parse;                        //   |   |   | ( | :parse
		case InstructionType::start_array: goto eval_array_start;             //   |   |   | [ | :eval_array_start
		case InstructionType::end_array:                                      
			switch (instruction_r1.instr) {                                   //   |   | x | ] |
			case InstructionType::start_array: goto eval_array_empty;         //   |   | [ | ] | :eval_array_empty
			case InstructionType::value: goto eval_array_end;                 //   |   |val| ] | :eval_array_end
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
        case InstructionType::end:                                            //   |   | x |end|  !!!
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
                case InstructionType::spacing: goto eval_binary_check;        //   | _ |val| _ | :eval_binary_check
                case InstructionType::op: goto eval_prefix; }                 //   |o p|val| _ | :eval_prefix	//Possibly unreachable
                default: goto error_syntax; }
        case InstructionType::value:                                          //   |   | x |val|
            switch (instruction_r1.instr) {
			case InstructionType::start_context: goto parse;                  //   |   | { |val| :parse
			case InstructionType::start_array: goto parse;                    //   |   | [ |val| :parse
            case InstructionType::start: goto parse;                          //   |   |s t|val| :parse
            case InstructionType::value: goto eval_binary_coalescing_short;   //   |   |val|val| :eval_binary_coalescing_short
            case InstructionType::spacing:                                    //   | x | _ |val|
                switch (instruction_r2.instr) {
                case InstructionType::value: goto eval_binary_coalescing;     //   |val| _ |val| :eval_binary_coalescing
                case InstructionType::op: goto parse;                         //   |o p| _ |val| :parse
                default: goto error_syntax; }
            case InstructionType::op:                                         //   | x |o p|val|
                switch (instruction_r2.instr) {
                case InstructionType::value: goto eval_binary_short;          //   |val|o p|val| :eval_binary_short
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
			stackInstructions.add(Instruction::atom(InstructionType::start_array));
			stackInstructions.at_r(0).modifier = stackArrays.max_index;
			goto parse;
		}

		eval_array_add: {
			stackArrays[instruction_r2.modifier].add(instruction_r1);
			stackInstructions.max_index -= 2;
			goto parse;
		}

		eval_array_add_empty: {
			stackArrays[instruction_r2.modifier].add(Instruction::val(ValueType::arr, 0));
			stackInstructions.max_index -= 1;
			goto parse;
		}

		eval_array_empty: {
			stackInstructions.max_index -= 1;
			stackInstructions.at_r(0).instr = InstructionType::value;
			goto parse;
		}

		eval_array_end: {
			stackArrays[instruction_r2.modifier].add(instruction_r1);
			stackInstructions.max_index -= 2;
			stackInstructions.at_r(0).instr = InstructionType::value;
			stackInstructions.at_r(0).value = ValueType::arr;
			goto parse;
		}

		eval_prefix: {
			tmpPtr = memory.at<uint8*>(instruction_r1.shift);

			if (!this->specification.prefix.count(*(String*)tmpPtr)) 
				goto error_syntax;

			ValueType* lc_value;

			if (!this->specification.prefix[(*(String*)tmpPtr)].count(instruction_r0.value))
				if (this->specification.prefix[(*(String*)tmpPtr)].count(ValueType::all))
					lc_value = this->specification.prefix[(*(String*)tmpPtr)][ValueType::all];
				else
					goto error_syntax;
			else
				lc_value = this->specification.prefix[(*(String*)tmpPtr)][instruction_r0.value];


			switch (*lc_value) {
			case ValueType::unprocedure:
				reinterpret_cast<void (*) (Program&)>(*(void**)(lc_value+1))(*this);
				goto parse;
			}

			goto error_syntax;	// TODO: error operator was not found
		}

		eval_postfix: {
			tmpPtr = memory.at<uint8*>(instruction_r1.shift);

			if (!this->specification.postfix.count(*(String*)tmpPtr))
				goto error_syntax;

			ValueType* lc_value;

			if (!this->specification.postfix[(*(String*)tmpPtr)].count(instruction_r2.value))
				if (this->specification.postfix[(*(String*)tmpPtr)].count(ValueType::all))
					lc_value = this->specification.postfix[(*(String*)tmpPtr)][ValueType::all];
				else
					goto error_syntax;
			else
				lc_value = this->specification.postfix[(*(String*)tmpPtr)][instruction_r2.value];

			switch (*lc_value) {
			case ValueType::unprocedure:
				reinterpret_cast<void (*) (Program&)>(*(void**)(lc_value + 1))(*this);
				goto parse;
			}

			goto error_syntax;
		}

		eval_binary_check: {}

		eval_binary_short: {

			tmpPtr = memory.at<uint8*>(instruction_r1.shift);

			if (!this->specification.binary.count(*(String*)tmpPtr))
				goto error_syntax;

			auto table = this->specification.binary[*(String*)tmpPtr];
			
			auto lc_binaryValueType = ValueTypeBinary(instruction_r2.value, instruction_r0.value);

			if(!table.count(lc_binaryValueType))
				goto error_syntax;
			/*
			SubroutineParameterMatching lc_subroutinePatternMatching = table[lc_binaryValueType];

			uint64 hash = 

			lc_subroutinePatternMatching.count(
			*/

			/*eval_binary_short_loop: {
				switch (arr->patternType) {
				//case SubroutinePatternMatchingType::Parameter:
				//	if (arr->parameter)														// Contents of both left and right side must be extracted to
				case SubroutinePatternMatchingType::ProcNative:								// same array, possibly separated by special ValueType,
					reinterpret_cast<void (*) (Program&)>(arr->pointer)(*this);				// and then iterated until match.
					goto parse;																// Also need to think how/whether context will be taken in account as well
				case SubroutinePatternMatchingType::None:
					goto error_syntax;
				}
			}*/

			goto error_syntax;
		}

		eval_binary_coalescing_short: {
			tmpPtr = memory.at<uint8*>(instruction_r1.shift);
			/*
			if (this->specification.binary.count(*(String*)tmpPtr)) {
				SubroutinePatternMatching* arr = this->specification.postfix[**(String**)tmpPtr];
				switch (arr->patternType) {
				case SubroutinePatternMatchingType::ProcNative:
					reinterpret_cast<void (*) (Program&)>(arr->pointer)(*this);
					goto parse;
				case SubroutinePatternMatchingType::None:
					goto error_syntax;
				}
			}*/

			goto error_syntax;
		}

		eval_binary_coalescing: {}
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