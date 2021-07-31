#include "interpreter.h"
#include "core.h"
#include <iostream>
#include <functional>

#define char_number T('0'): case T('1'): case T('2'): case T('3'): case T('4'): case T('5'): case T('6'): case T('7'): case T('8'): case T('9')

#define char_operator 										\
	     T('<'): case T('>'): case T('='):					\
	case T('@'): case T('?'): case T(','):					\
	case T('-'): case T('+'): case T('*'): case T(':'):		\
	case T('/'): case T('^'): case T('\\'): case T('%'):	\
	case T('&'): case T('|'):								\
	case T('.'): case T('~')								\


#define char_space_character								\
	     T('\n'): case T('\r'): case T('\t'): case T(' ')	\


#define char_special_character								\
		 T(';'):											\
	case T('['): case T('{'): case T('('):					\
	case T(']'): case T('}'): case T(')')					\

// I don't trust inlines.
#define alloc_guard(SIZE)														\
	if (memory.maxSize - memory.currentSize < SIZE)								\
		if (tmpPtr = (uint8*)realloc(memory.data, memory.maxSize <<= 1))		\
			memory.data = tmpPtr;												\
		else																	\
			goto error_memory_allocation	// TODO: error stack memory allocation.

int main()
{		
	//int res = sayHello();

	const charT* script = T("[123;name;\"str\"]%");

	uint8* allocator;

	std::unordered_map<String, ValueType*>* memory = new std::unordered_map<String, ValueType*>();

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

	std::unordered_map<String, ValueType*> mry = std::unordered_map<String, ValueType*>();

	allocator = (uint8*)malloc(sizeof(void*) + sizeof(ValueType));
	*(ValueType*)allocator = ValueType::dict;
	*(std::unordered_map<String, ValueType*>**)(allocator+1) = memory;

	auto val = **(std::unordered_map<String, ValueType*>**)(allocator+1);

	mry.insert_or_assign(String(T("config")), (ValueType*)allocator);//(ValueType*)(new containerPtr(ValueType::dict, (void*)memory)));

	std::vector<std::unordered_map<String, ValueType*>> namespaces = std::vector<std::unordered_map<String, ValueType*>>();
	namespaces.push_back(mry);

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
	*/
	void* address = test0;

	//reinterpret_cast<void (*)(int64)>(address) ((int64)(uintptr_t)memory[T("string")]);

	//int32 result = reinterpret_cast<uint16 (*)(uint16, uint16)>(address) (65535,1);
	//int32 result = reinterpret_cast<int32(*)(struct alignas(2) {}) > (address)(123);		//	So, if you pass a bigger type as an argument
	//																						//	then it's possible to correctly call the function

	//print(mry[T("config")]);

	uint32 result = reinterpret_cast<uint32 (*)(uint16, uint32, uint16, float32, float64)>(address) (0, 1, 2, 3, 4);
	//*(float64*)(_alloca(8)) = 4.0;
	//uint32 result = reinterpret_cast<uint32(*)(uint16, uint32, uint16, float32)>(address) (0, 1, 2, 3);

	result = test0(0, 1, 2, 3, 4);

	auto mem = std::vector<std::unordered_map<String, ValueType*>>();
	mem.push_back(mry);

	Program program = Program(mem);

	return (int)program.run(script); /**/
}

uint32 test0(uint16 arg0, uint32 arg1, uint16 arg2, float32 arg3, float64 arg4)
{
	return arg0 + arg1 + arg2 + arg3 + arg4;
}



Program::Program() {
	data = std::vector<std::unordered_map<String, ValueType*>>();
	data[0] = std::unordered_map<String, ValueType*>();
	stackInstructions = std::vector<Instruction>();
	stackInstructions.push_back(Instruction::atom(InstructionType::start));
	context = ValueLocation{ 0, ValueType::none };
	specification = Core::getCore();
};

Program::Program(std::vector<std::unordered_map<String, ValueType*>> data) {
	this->data = data;
	stackInstructions = std::vector<Instruction>();
	stackInstructions.push_back(Instruction::atom(InstructionType::start));
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
	int iterator = 0;

	if (!memory.data)
		if (!(memory.data = (uint8*)malloc(256)))
			goto error_memory_allocation;
		else
			memory.maxSize = 256;

	parse: {
		if (script[++scriptIndex] == 0)
		{
			if (stackInstructions.back().instr != InstructionType::end)
			{
				--scriptIndex;
				stackInstructions.push_back(Instruction::atom(InstructionType::end));
				goto evaluate;
			}
			else
				goto ending;
		}

		switch (script[scriptIndex])
		{
		case T(';'):
			stackInstructions.push_back(Instruction::atom(InstructionType::separator));
			goto evaluate;

		case T('('):
			stackInstructions.push_back(Instruction::atom(InstructionType::start_group));
			goto evaluate;

		case T('['):
			stackInstructions.push_back(Instruction::atom(InstructionType::start_array));
			goto evaluate;

		case T('{'):
			stackInstructions.push_back(Instruction::atom(InstructionType::start_context));
			goto evaluate;

		case T(')'):
			stackInstructions.push_back(Instruction::atom(InstructionType::end_group));
			goto evaluate;

		case T(']'):
			stackInstructions.push_back(Instruction::atom(InstructionType::end_array));
			goto evaluate;

		case T('}'):
			stackInstructions.push_back(Instruction::atom(InstructionType::end_context));
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
				stackInstructions.push_back(Instruction::atom(InstructionType::spacing));

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

				alloc_guard(sizeof(void*));

				stackInstructions.push_back(Instruction::pos(InstructionType::op, memory.currentSize));

				*(uintptr_t*)(memory.data + memory.currentSize) = (uintptr_t)buffer;
				memory.currentSize += sizeof(void*);

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

				alloc_guard(sizeof(void*));

				stackInstructions.push_back(Instruction::val(ValueType::name, memory.currentSize));

				*(uintptr_t*)(memory.data + memory.currentSize) = (uintptr_t)buffer;
				memory.currentSize += sizeof(void*);

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

				alloc_guard(sizeof(int64));

				stackInstructions.push_back(Instruction::val(ValueType::int64, memory.currentSize));

				//*(int*)(memory.data + memory.currentSize) = 5;// = std::stoi(*buffer);

				int64 number = 0;
				for (int i = 0; i < buffer->length(); ++i)
					number *= 10,
					number += ((*buffer)[i] - T('0'));

				*(int64*)(memory.data + memory.currentSize) = number;

				memory.currentSize += sizeof(int64);

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

				alloc_guard(sizeof(void*));

				stackInstructions.push_back(Instruction::val(ValueType::string, memory.currentSize));// (ValueType::string, memory.currentSize));

				*(uintptr_t*)(memory.data + memory.currentSize) = (uintptr_t)buffer;
				memory.currentSize += sizeof(void*);

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
        iterator = stackInstructions.size();						          //    ___________
                                                                              //   |   |   |   |
        switch (stackInstructions[iterator - 1].instr) {
        case InstructionType::start: goto parse;                              //   |   |   |s t| :parse
        case InstructionType::op: goto parse;                                 //   |   |   |o p| :parse
		case InstructionType::separator:
			switch (stackInstructions[iterator - 2].instr) {                  //   |   | x | ; |
			case InstructionType::separator: goto eval_array_add_empty;       //   |   | ; | ; | :eval_array_add_empty
			case InstructionType::op: goto eval_postfix;                      //   |   |o p| ; | :eval_postfix		//TODO: Better to do another level to check for val
			case InstructionType::value:                                      
				switch (stackInstructions[iterator - 3].instr) {              //   | x |val| ; |
				case InstructionType::op: goto eval_prefix;                   //   |o p|val| ; | :eval_prefix
				case InstructionType::start_array: goto eval_array_add;       //   | [ |val| ; | :eval_array_add
				default: goto error_syntax; }
			default: goto error_syntax; }
        case InstructionType::start_group: goto parse;                        //   |   |   | ( | :parse
		case InstructionType::start_array: goto eval_array_start;             //   |   |   | [ | :eval_array_start
		case InstructionType::end_array:                                      
			switch (stackInstructions[iterator - 2].instr) {                  //   |   | x | ] |
			case InstructionType::start_array: goto eval_array_empty;         //   |   | [ | ] | :eval_array_empty
			case InstructionType::value: goto eval_array_end;                 //   |   |val| ] | :eval_array_end
			default: goto error_syntax;}
		case InstructionType::start_context:                                  
            switch (stackInstructions[iterator - 2].instr) {                  //   |   | x | { |
            case InstructionType::spacing: goto eval_context_new;             //   |   | _ | { | :eval_context_new
            case InstructionType::value: goto eval_context_of;                //   |   |val| { | :eval_context_of
			case InstructionType::context: goto parse;                        //   |   |ctx| { | :parse
            default: goto error_syntax; }
        case InstructionType::end_context:                                    //   |   | x | } |
            switch (stackInstructions[iterator - 2].instr) {
            case InstructionType::value: goto eval_context_finish;            //   | x |val| } |
                switch (stackInstructions[iterator - 3].instr) {
                case InstructionType::op: goto eval_prefix;                   //   |o p|val| } | :eval_prefix
                case InstructionType::start_context: goto eval_context_finish;//   | { |val| } | :eval_context_finish
                default: goto error_syntax; }
            case InstructionType::op:                                         //   | x |o p| } |
                switch (stackInstructions[iterator - 3].instr) {
                case InstructionType::value: goto eval_postfix;               //   |val|o p| } | :eval_postfix
                default: goto error_syntax; }
            default: goto error_syntax; }
        case InstructionType::end:                                            //   |   | x |end|  !!!
        case InstructionType::spacing:                                        //   |   | x | _ |
            switch (stackInstructions[iterator - 2].instr) {
            case InstructionType::start: goto parse;                          //   |   |s t| _ | :parse
            case InstructionType::op:                                         //   | x |o p| _ |
                switch (stackInstructions[iterator - 3].instr) {
                case InstructionType::spacing: goto parse;                    //   | _ |o p| _ | :parse
                case InstructionType::value: goto eval_postfix;               //   |val|o p| _ | :eval_postfix
                default: goto error_syntax; }
            case InstructionType::value:                                      //   | x |val| _ | 
                switch (stackInstructions[iterator - 3].instr) {
                case InstructionType::start: goto parse;                      //   |s t|val| _ | :parse
                case InstructionType::spacing: goto eval_binary_check;        //   | _ |val| _ | :eval_binary_check
                case InstructionType::op: goto eval_prefix; }                 //   |o p|val| _ | :eval_prefix
                default: goto error_syntax; }
        case InstructionType::value:                                          //   |   | x |val|
            switch (stackInstructions[iterator - 2].instr) {
			case InstructionType::start_context: goto parse;                  //   |   | { |val| :parse
			case InstructionType::start_array: goto parse;                    //   |   | [ |val| :parse
            case InstructionType::start: goto parse;                          //   |   |s t|val| :parse
            case InstructionType::value: goto eval_binary_coalescing_short;   //   |   |val|val| :eval_binary_coalescing_short
            case InstructionType::spacing:                                    //   | x | _ |val|
                switch (stackInstructions[iterator - 3].instr) {
                case InstructionType::value: goto eval_binary_coalescing;     //   |val| _ |val| :eval_binary_coalescing
                case InstructionType::op: goto parse;                         //   |o p| _ |val| :parse
                default: goto error_syntax; }
            case InstructionType::op:                                         //   | x |o p|val|
                switch (stackInstructions[iterator - 3].instr) {
                case InstructionType::value: goto eval_binary_short;          //   |val|o p|val| :eval_binary_short
                case InstructionType::start:                                  //   |s t|o p|val| :eval_prefix
                case InstructionType::start_group:                            //   | ( |o p|val| :eval_prefix
                case InstructionType::start_array:                            //   | [ |o p|val| :eval_prefix
                case InstructionType::start_context:                          //   | { |o p|val| :eval_prefix
                case InstructionType::spacing:                                //   | { |o p|val| :eval_prefix
                case InstructionType::separator: goto eval_prefix;            //   | ; |o p|val| :eval_prefix
                default: goto error_syntax; }
            default: goto error_syntax; }
        default: goto error_syntax; }


		eval_context_new: {
			stackInstructions.push_back(Instruction::context(context.value, context.shift));

			*(void**)(memory.data + memory.currentSize) = new std::unordered_map<String, ValueType*>();
			memory.currentSize += sizeof(void*);

			stackInstructions.push_back(Instruction::atom(InstructionType::start_context));

			goto parse;
		}

		eval_context_of: {
			stackInstructions.back() = Instruction::context(context.value, context.shift);

			context.shift = stackInstructions[iterator - 2].shift;
			context.value = stackInstructions[iterator - 2].value;

			stackInstructions.push_back(Instruction::atom(InstructionType::start_context));

			goto parse;
		}

		eval_context_finish: {
			/*context = stack[iterator - 2].location;
			stack[iterator - 2] = stack[iterator - 1];
			stack.pop_back();*/
			context = stackInstructions[iterator - 4].location;
			stackInstructions[iterator - 4] = stackInstructions[iterator - 2];
			stackInstructions.resize(iterator - 4);
			goto parse;
		}

		eval_array_start: {
			stackArrays.push_back(std::vector<Instruction>());
			stackArrays.back().reserve(16);
			stackInstructions.back().modifier = stackArrays.size() - 1;
			goto parse;
		}

		eval_array_add: {
			stackArrays[stackInstructions[iterator - 3].modifier].push_back(stackInstructions[iterator - 2]);
			stackInstructions.pop_back();
			stackInstructions.pop_back();
			goto parse;
		}

		eval_array_add_empty: {
			stackArrays[stackInstructions[iterator - 3].modifier].push_back(Instruction::val(ValueType::arr, 0));
			stackInstructions.pop_back();
			goto parse;
		}

		eval_array_empty: {
			stackInstructions.pop_back();
			stackInstructions.back().instr = InstructionType::value;
			goto parse;
		}

		eval_array_end: {
			stackArrays[stackInstructions[iterator - 3].modifier].push_back(stackInstructions[iterator - 2]);
			stackInstructions.pop_back();
			stackInstructions.pop_back();
			stackInstructions.back().instr = InstructionType::value;
			stackInstructions.back().value = ValueType::arr;
			goto parse;
		}

		eval_prefix: {
			tmpPtr = memory.data + stackInstructions[iterator - 2].shift;

			if (this->specification.prefix.count(**(String**)tmpPtr)) {
				SubroutinePatternMatching* arr = this->specification.prefix[**(String**)tmpPtr];
				eval_prefix_loop:
					switch (arr->patternType) {
					case SubroutinePatternMatchingType::None:
						goto error_syntax; // TODO: error can't match function
					case SubroutinePatternMatchingType::Argument:
						break;
					}
				goto eval_prefix_loop;
			pref:
				goto parse;
			}
			goto error_syntax;	// TODO: error operator was not found
		}

		eval_postfix: {
			tmpPtr = memory.data + stackInstructions[iterator - 2].shift;

			if (this->specification.postfix.count(**(String**)tmpPtr)) {
				SubroutinePatternMatching* arr = this->specification.postfix[**(String**)tmpPtr];
				switch (arr->patternType) {
				case SubroutinePatternMatchingType::ProcNative:
					reinterpret_cast<void (*) (Program&)>(arr->pointer)(*this);
					goto parse;
				case SubroutinePatternMatchingType::None:
					goto error_syntax;
				}
			}

			goto error_syntax;
				/*for (; arr->patternType == UnaryType::None; ++arr) {
					switch (arr->patternType & (UnaryType)0b11111000) {
					case UnaryType::AllAll:
						goto eval_post_loopbreaker;
					case UnaryType::TypeAll:
					case UnaryType::TypeType:
						if (arr->parameterType == stack[iterator - 3].value)
							goto eval_post_loopbreaker;
						else continue;
					case UnaryType::None: //means array reached its end.
						goto error_syntax;
					}
				}*//*
			eval_post_loopbreaker:
				switch (arr->patternType & (UnaryType)0b00000111) {
				case UnaryType::funcTarget:
					stack.push_back(Instruction::pos(InstructionType::call, memory.currentSize));

					*(uint64*)(memory.data + memory.currentSize) = scriptIndex;
					memory.currentSize += sizeof(uint64);
					*(const charT**)(memory.data + memory.currentSize) = script;
					memory.currentSize += sizeof(void*);

					stack.push_back(Instruction::context(context.value, context.shift));

					script = (const charT*)arr->function;
					scriptIndex = 0;

					context = stack[iterator - 3].location;
					break;
				case (UnaryType::methTarget):
					break;
				case (UnaryType::procTarget):
					break;
				case (UnaryType::funcNative):
					break;
				case (UnaryType::methNative):
					break;
				case (UnaryType::procNative):
					reinterpret_cast<void (*) (Program&)>(arr->function)(*this);
					break;
				}
				goto evaluate; //in case end context (before was parse)
			}
			else goto error_syntax; //operator not found.*/
		}

		eval_binary_check: {}

		eval_binary_short: {/*
			tmpPtr = memory.data + stack[iterator - 2].shift;

			if (this->specification.binary.count(**(String**)tmpPtr)) {
				Binary* arr = this->specification.binary[**(String**)tmpPtr];

				for (; arr->patternType == BinaryType::None; ++arr) {
					switch (arr->patternType & (BinaryType)0b11111000) {
					case BinaryType::AllAllAll:
					case BinaryType::AllAllType:
						goto eval_binary_loopbreaker;
					case BinaryType::TypeAllAll:
					case BinaryType::TypeAllType:
						if (arr->leftParameterType == stack[iterator - 3].value)
							goto eval_binary_loopbreaker;
						else continue;
					case BinaryType::AllTypeAll:
					case BinaryType::AllTypeType:
						if (arr->rightParameterType == stack[iterator - 1].value)
							goto eval_binary_loopbreaker;
						else continue;
					case BinaryType::TypeTypeAll:
					case BinaryType::TypeTypeType:
						if ((arr->leftParameterType == stack[iterator - 3].value) & (arr->rightParameterType == stack[iterator - 1].value))
							goto eval_binary_loopbreaker;
						else continue;
					case BinaryType::None: //means array reached its end.
						goto error_syntax;
					}
				}

			eval_binary_loopbreaker:
				switch (arr->patternType & (BinaryType)0b00000111) {
				case BinaryType::funcTarget:
					stack.push_back(Instruction::pos(InstructionType::call, memory.currentSize));

					*(uint64*)(memory.data + memory.currentSize) = scriptIndex;
					memory.currentSize += sizeof(uint64);
					*(const charT**)(memory.data + memory.currentSize) = script;
					memory.currentSize += sizeof(void*);

					stack.push_back(Instruction::context(context.value, context.shift));

					script = (const charT*)arr->function;
					scriptIndex = 0;

					context = stack[iterator - 3].location;
					break;
				case (BinaryType::methTarget):
					break;
				case (BinaryType::procTarget):
					break;
				case (BinaryType::funcNative):

					//reinterpret_cast<_s16(*)(_s64)>(memory.data)(*(_s64*)memory.data);
					//std::function<void> f;

					break;
				case (SubroutinePatternMatchingType::MethNative):
					break;
				case (SubroutinePatternMatchingType::ProcNative):
					reinterpret_cast<void (*) (Program&)>(arr->function)(*this);
					break;
				}
				goto parse;
			}
			else goto error_syntax; //operator not found.*/
		}

		eval_binary_coalescing_short: {}

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

void functionCall(Program& program, void* function, int32 argument) {

	struct parameter {};
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
		for(auto var: **((std::unordered_map<String, ValueType*>**)(memory + 1)))
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