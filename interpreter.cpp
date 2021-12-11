#include "interpreter.h"
#include "core.h"
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include <functional>
#include "common_types.h"
#include "common_utils.h"
#include <intrin.h>

Table<String, ValueType*> g_data = Table<String, ValueType*>();
thread_local std::istream* stream;
thread_local charT* script = new charT[string_buffer_init_size + 1];
thread_local int scriptIndex = -1;
thread_local Array<Table<String, ValueType*>*> g_stack_namespace = Array<Table<String, ValueType*>*>(16);
thread_local Module* g_specification = Core::initCore();
thread_local Array<Instruction> g_stack_instruction = Array<Instruction>(16);
thread_local Array<Table<String, ValueType*>> g_stack_local = Array<Table<String, ValueType*>>(16);
thread_local Array<Instruction> g_stack_context = Array<Instruction>(16);
thread_local Span g_memory = Span(1024*1024);

Status run()
{
	//int scriptIndex = -1;
	//charT* script = new charT[string_buffer_init_size+1];
	charT substitute;

	script[string_buffer_init_size] = T('\1');

	if (!g_memory.content)
		g_memory.init(memory_init_size);

	{
		auto table = new Table<String, ValueType*>();
		g_stack_namespace.add(table);
		g_memory.add<Table<String, ValueType*>*>(table);
	}

	stream->read(script, string_buffer_init_size);
	if (stream->eof())
		script[stream->gcount()] = T('\0');


	//temporary variables
	uint8* tmpPtr;
	uint8* findResult;

	Instruction instruction_r0; //Top instruction
	Instruction instruction_r1; //Pre-top instruction
	Instruction instruction_r2; //Pre-pre-top instruction
	Instruction instruction_r3; //Pre-pre-pre-top instruction


	parse: {
		switch (script[++scriptIndex])
		{
		case T('\0'):
			if (g_stack_instruction.get_r(0).instr != InstructionType::end)
			{
				--scriptIndex;
				g_stack_instruction.add(Instruction::atom(InstructionType::end));
				goto evaluate;
			}
			else
				goto ending;

		case T('\1'):
			stream->read(script, string_buffer_init_size);
			if (stream->eof())
				script[stream->gcount()] = T('\0');
			scriptIndex = -1;
			goto parse;

		case T(';'):
			g_stack_instruction.add(Instruction::atom(InstructionType::separator));
			goto evaluate;

		case T('('):
			g_stack_instruction.add(Instruction::atom(InstructionType::start_group));
			goto evaluate;

		case T('['):
			g_stack_instruction.add(Instruction::atom(InstructionType::start_array));
			goto evaluate;

		case T('{'):
			g_stack_instruction.add(Instruction::atom(InstructionType::start_context));
			goto evaluate;

		case T(')'):
			g_stack_instruction.add(Instruction::atom(InstructionType::end_group));
			goto evaluate;

		case T(']'):
			g_stack_instruction.add(Instruction::atom(InstructionType::end_array));
			goto evaluate;

		case T('}'):
			g_stack_instruction.add(Instruction::atom(InstructionType::end_context));
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
			g_stack_instruction.add(Instruction::atom(InstructionType::spacing));
		spacing_loop:
			switch (script[++scriptIndex])
			{
			case T('\1'):
				stream->read(script, string_buffer_init_size);
				if (stream->eof())
					script[stream->gcount()] = T('\0');
				scriptIndex = -1;
				goto spacing_loop;

			case char_space_character:
				//Currently MoreINI doesn't specify any behaviour depending on spacing signature, though it's possible to make one.
				goto spacing_loop;

			default:
				--scriptIndex;
				goto evaluate;
			}
		}

		operation: {
			g_stack_instruction.add(Instruction::pos(InstructionType::op, g_memory.max_index));
		operation_loop:
			switch (script[++scriptIndex])
			{
			case T('\1'):
				stream->read(script, string_buffer_init_size);
				if (stream->eof())
					script[stream->gcount()] = T('\0');
				scriptIndex = -1;
				goto operation_loop;
			case char_operator:
				g_memory.add<charT>(script[scriptIndex]);
				goto operation_loop;
			case T('\0'):
			default:
				--scriptIndex;
				{Instruction& instruction = g_stack_instruction.at_r(0);
				instruction.modifier = g_memory.max_index - instruction.shift;}
				goto evaluate;
			}
		}

		name: {
			g_stack_instruction.add(Instruction::val(ValueType::name, g_memory.max_index));
		name_loop:
			switch (script[++scriptIndex])
			{
			case T('\1'):
				stream->read(script, string_buffer_init_size);
				if (stream->eof())
					script[stream->gcount()] = T('\0');
				scriptIndex = -1;
				goto name_loop;
			case char_space_character: // if special character then name finished.
			case char_operator:
			case char_special_character:
			case T('`'):
			case T('\''):
			case T('"'):
			case T('\0'):
				--scriptIndex; 
				{Instruction& instruction = g_stack_instruction.at_r(0);
				instruction.modifier = g_memory.max_index - instruction.shift;}
				goto evaluate;

			default:
				g_memory.add<charT>(script[scriptIndex]);
				goto name_loop;
			}
		}

		number: {
			g_stack_instruction.add(Instruction::val(ValueType::int64, g_memory.max_index));
			int64 number = 0;
		number_loop:
			switch (script[++scriptIndex])
			{
			case T('\1'):
				stream->read(script, string_buffer_init_size);
				if (stream->eof())
					script[stream->gcount()] = T('\0');
				scriptIndex = -1;
				goto number_loop;
			case char_number:
				number *= 10;
				number += script[scriptIndex] - T('0');
				goto number_loop;

			default:
				--scriptIndex;
				g_memory.add<int64>(number);
				goto evaluate;
			}
		}

		string: {
			g_stack_instruction.add(Instruction::val(ValueType::string, g_memory.max_index));
		string_loop:
			switch (script[++scriptIndex])
			{
			case T('"'):
				{Instruction& instruction = g_stack_instruction.at_r(0);
				instruction.modifier = g_memory.max_index - instruction.shift;}
				goto evaluate;

			string_loop_escape:
			case T('\\'):
				switch (script[++scriptIndex]) {
				case T('\0'):
					goto error_string_missing_closing_quote;
				case T('\1'):
					stream->read(script, string_buffer_init_size);
					if (stream->eof())
						script[stream->gcount()] = T('\0');
					scriptIndex = -1;
					goto string_loop_escape;
				case T('\"'):
					g_memory.add<charT>('\"');
				case T('0'):
					g_memory.add<charT>('\0');
				case T('n'):
					g_memory.add<charT>('\n');
				case T('t'):
					g_memory.add<charT>('\t');
				case T('r'):
					g_memory.add<charT>('\r');
				}
				goto string_loop;

			case T('\0'):
				goto error_string_missing_closing_quote;

			case T('\1'):
				stream->read(script, string_buffer_init_size);
				if (stream->eof())
					script[stream->gcount()] = T('\0');
				scriptIndex = -1;
				goto string_loop;

			default:
				g_memory.add<charT>(script[scriptIndex]);
				goto string_loop;
			}
		}

		link: {
			g_stack_instruction.add(Instruction::val(ValueType::string, g_memory.max_index));
		link_loop:
			switch (script[++scriptIndex])
			{
			link_loop_escape:
			case T('\\'):
				switch (script[++scriptIndex]) {
				case T('\0'):
					goto error_string_missing_closing_quote;
				case T('\1'):
					stream->read(script, string_buffer_init_size);
					scriptIndex = -1;
					goto link_loop_escape;
				case T('\''):
					g_memory.add<charT>('\'');
				case T('0'):
					g_memory.add<charT>('\0');
				case T('n'):
					g_memory.add<charT>('\n');
				case T('t'):
					g_memory.add<charT>('\t');
				case T('r'):
					g_memory.add<charT>('\r');
				}
				goto link_loop;

			case T('\''):
				{Instruction& instruction = g_stack_instruction.at_r(0);
				instruction.modifier = g_memory.max_index - instruction.shift;}
				goto evaluate;

			case T('\0'):
				goto error_string_missing_closing_quote;

			case T('\1'):
				stream->read(script, string_buffer_init_size);
				scriptIndex = -1;
				goto string_loop;

			default:
				g_memory.add<charT>(script[scriptIndex]);
				goto link_loop;
			}
		}

		expression: {
			g_stack_instruction.add(Instruction::val(ValueType::string, g_memory.max_index));
		expression_loop:
			switch (script[++scriptIndex])
			{
			expression_loop_escape:
			case T('\\'):
				switch (script[++scriptIndex]) {
				case T('\0'):
					goto error_string_missing_closing_quote;
				case T('\1'):
					stream->read(script, string_buffer_init_size);
					scriptIndex = -1;
					goto expression_loop_escape;
				case T('`'):
					g_memory.add<charT>('`');
				case T('0'):
					g_memory.add<charT>('\0');
				case T('n'):
					g_memory.add<charT>('\n');
				case T('t'):
					g_memory.add<charT>('\t');
				case T('r'):
					g_memory.add<charT>('\r');
				}
				goto expression_loop;

			case T('\`'):
				{Instruction& instruction = g_stack_instruction.at_r(0);
				instruction.modifier = g_memory.max_index - instruction.shift;}
				goto evaluate;

			case T('\0'):
				goto error_string_missing_closing_quote;

			case T('\1'):
				stream->read(script, string_buffer_init_size);
				scriptIndex = -1;
				goto string_loop;

			default:
				g_memory.add<charT>(script[scriptIndex]);
				goto expression_loop;
			}
		}
	}

	skip: {
		switch (script[++scriptIndex]) {
		case T(';'): goto evaluate;	//Maybe :parse?
		case T('('): goto skip_group;
		case T('['): goto skip_array;
		case T('{'): goto skip_context;
		case T(')'):
		case T(']'):
		case T('}'): 
			--scriptIndex;
			goto evaluate; //Need for proper removal of skip instruction.
		default: goto skip;
		}

		skip_group: {
			switch(script[++scriptIndex]) {
			case T(')'): goto skip;
			case T(']'): goto error_syntax;
			case T('}'): goto error_syntax;
			default: goto skip_group;
			}
		}

		skip_array: {
			switch(script[++scriptIndex]) {
			case T(']'): goto skip;
			case T(')'): goto error_syntax;
			case T('}'): goto error_syntax;
			default: goto skip_array;
			}
		}

		skip_context: {
			switch(script[++scriptIndex]) {
			case T('}'): goto skip;
			case T(')'): goto error_syntax;
			case T(']'): goto error_syntax;
			default: goto skip_context;
			}
		}
	}

    evaluate: {
		instruction_r0 = *(g_stack_instruction.content + g_stack_instruction.max_index);
		instruction_r1 = *(g_stack_instruction.content + g_stack_instruction.max_index - 1);
		instruction_r2 = *(g_stack_instruction.content + g_stack_instruction.max_index - 2);

                                                                              //   Decision tree 
        switch (instruction_r0.instr) {                                       //    ___________
        case InstructionType::start: goto parse;                              //   |   |   |s t| :parse
		case InstructionType::call: goto parse;                               //   |   |   |fun| :parse
		case InstructionType::error: goto parse;                              //   |   |   |err| :error
        case InstructionType::op: goto parse;                                 //   |   |   |o p| :parse //TODO: op val op :eval_prefix
		case InstructionType::skip_next: goto eval_skip_next;                 //   |   |   |s>-| :eval_skip_next
		case InstructionType::skip_after_next: goto parse;                    //   |   |   |s->| :parse
		case InstructionType::ignore_separator: goto parse;                   //   |   |   |/;/| :parse
		case InstructionType::ignore_array_start: goto parse;                 //   |   |   |/[/| :parse
		//case InstructionType::call: goto eval_call;
		case InstructionType::separator:
			switch (instruction_r1.instr) {                                   //   |   | x | ; |
			case InstructionType::ignore_separator: goto eval_erase_r0r1;     //   |   |/;/| ; | :eval_erase_r0r1
			case InstructionType::spacing: goto eval_erase_r1;                //   |   | _ | ; | :eval_erase_r1
			case InstructionType::ignore_array_start: goto parse;             //   |   |/[/| ; | :parse
			case InstructionType::separator: goto eval_tuple_add_empty;       //   |   | ; | ; | :eval_tuple_add_empty //Possibly unreachable
			case InstructionType::start_array: goto eval_tuple_add_empty;     //   |   | [ | ; | :eval_tuple_add_empty
			case InstructionType::op: goto eval_postfix;                      //   |   |o p| ; | :eval_postfix		//TODO: Better to do another level to check for val
			case InstructionType::value:                                      
				switch (instruction_r2.instr) {                               //   | x |val| ; |
				case InstructionType::start: goto eval_delete_r0r1;           //   |s t|val| ; | :eval_delete_r0r1
				case InstructionType::start_context: goto eval_delete_r0r1;   //   | { |val| ; | :eval_delete_r0r1
				case InstructionType::call: goto eval_delete_r0r1;            //   |fun|val| ; | :eval_delete_r0r1
				case InstructionType::op: goto eval_prefix;                   //   |o p|val| ; | :eval_prefix		//TODO: shift to match signature.
				case InstructionType::start_array: goto eval_tuple_add;       //   | [ |val| ; | :eval_tuple_add
				case InstructionType::spacing: goto eval_binary_long;         //   | _ |val| ; | :eval_binary_long
				case InstructionType::skip_after_next: goto eval_skip_after_next;//|s->|val| ; | :eval_skip_after_next
				case InstructionType::ignore_separator: goto eval_leave_r1;   //   |/;/|val| ; | :eval_leave_r1
				case InstructionType::ignore_array_start: goto parse;         //   |/[/|val| ; | :parse
				default: goto error_syntax; }
			default: goto error_syntax; }
        case InstructionType::start_group: goto parse;                        //   |   |   | ( | :parse
		case InstructionType::end_group:                                      //   |   | x | ) |
			switch (instruction_r1.instr) {
			case InstructionType::spacing: goto eval_erase_r1;                //   |   | _ | ) | :eval_erase_r1
			case InstructionType::op:  
				switch (instruction_r2.instr) {                               //   | x |o p| ) |
				case InstructionType::value: goto eval_postfix;               //   |val|o p| ) | :eval_postfix
				default: goto error_syntax;}
			case InstructionType::value:                                      //   | x |val| ) |
				switch (instruction_r2.instr) {
				case InstructionType::start_group: goto eval_group;           //   | ( |val| ) | :eval_group
				case InstructionType::spacing: goto eval_binary_long;         //   | _ |val| ) | :eval_binary_long
				default: goto error_syntax; }
			default: goto error_syntax; }
		case InstructionType::start_array:                                    //   |   | x | [ | 
			switch (instruction_r1.instr) {
			case InstructionType::ignore_array_start: goto eval_erase_r0r1;   //   |   |/[/| [ | :eval_erase_r0r1
			case InstructionType::call: goto parse;                           //   |   |fun| [ | :parse
			default: goto eval_tuple_start;}                                  //   |   |all| [ | :eval_tuple_start
		case InstructionType::end_array:                                      
			switch (instruction_r1.instr) {                                   //   |   | x | ] |
			case InstructionType::spacing: goto eval_erase_r1;                //   |   | _ | ] | :eval_erase_r1
			case InstructionType::start_array: goto eval_tuple_empty;         //   |   | [ | ] | :eval_tuple_empty
			case InstructionType::value:                                      //   | x |val| ] |
				switch (instruction_r2.instr) {
				case InstructionType::start_array: goto eval_tuple_end;       //   | [ |val| ] | :eval_tuple_end
				case InstructionType::spacing: goto eval_binary_long;         //   | _ |val| ] | :eval_binary_long
				default: goto error_syntax;}
			default: goto error_syntax;}
		case InstructionType::start_context:                                  
            switch (instruction_r1.instr) {                                   //   |   | x | { |
			case InstructionType::call: goto eval_expr;                       //   |   |fun| { | :eval_expr
            case InstructionType::spacing:                                    
				switch (instruction_r2.instr) {                               //   | x | _ | { |
				case InstructionType::start: goto eval_expr;                  //   |s t| _ | { | :eval_erase_r1
				case InstructionType::value:  goto eval_erase_r1;             //   |val| _ | { | :eval_erase_r1
				case InstructionType::op: goto eval_expr;                     //   |o p| _ | { | :eval_expr
				case InstructionType::start_group: goto eval_expr;            //   | ( | _ | { | :eval_expr
				case InstructionType::start_array: goto eval_expr;            //   | [ | _ | { | :eval_expr
				case InstructionType::start_context: goto eval_expr;          //   | { | _ | { | :eval_expr
				default: goto error_syntax;}
            case InstructionType::value: goto eval_context_start;             //   |   |val| { | :eval_context_start
			case InstructionType::start: goto eval_expr;                      //   |   |s t| { | :eval_expr
			case InstructionType::start_group: goto eval_expr;                //   |   | ( | { | :eval_expr
			case InstructionType::op: goto eval_expr;                         //   |   |o p| { | :eval_expr
			case InstructionType::context: goto parse;                        //   |   |ctx| { | :parse
            default: goto error_syntax; }
        case InstructionType::end_context:                                    //   |   | x | } |
            switch (instruction_r1.instr) {
			case InstructionType::start_context: goto eval_context_finish_empty;// |   | { | } | :eval_context_finish_empty
			case InstructionType::spacing: goto eval_erase_r1;                //   |   | _ | } | :eval_erase_r1
            case InstructionType::value:                                      //   | x |val| } |
                switch (instruction_r2.instr) {
                case InstructionType::op: goto eval_prefix;                   //   |o p|val| } | :eval_prefix
				case InstructionType::spacing: goto eval_erase_r2;            //   | _ |val| } | :eval_erase_r2
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
			case InstructionType::separator: goto eval_cleanup_separator;     //   |   | ; |end| :eval_cleanup_separator
			case InstructionType::spacing: goto eval_erase_r1;                //   |   | _ |end| :eval_erase_r1
			case InstructionType::op:                                         //   | x |o p|end|
				switch (instruction_r2.instr) {
				case InstructionType::spacing: goto parse;                    //   | _ |o p|end| :parse
				case InstructionType::value: goto eval_postfix;               //   |val|o p|end| :eval_postfix
				default: goto error_syntax;	}
			case InstructionType::value:                                      //   | x |val|end|
				switch (instruction_r2.instr) {
				case InstructionType::start: goto ending;                     //   |s t|val|end| :ending
				case InstructionType::call: goto eval_finish_call;            //   |fun|val|end| :eval_finish_call
				case InstructionType::spacing: goto eval_binary_long;         //   | _ |val|end| :eval_binary_long
				case InstructionType::op: goto eval_prefix;	}                 //   |o p|val|end| :eval_prefix	//Possibly unreachable
			default: goto error_syntax;
			}
        case InstructionType::spacing:                                        //   |   | x | _ |
            switch (instruction_r1.instr) {
            case InstructionType::start: goto eval_erase_r0;                  //   |   |s t| _ | :eval_erase_r0
			case InstructionType::skip_after_next: goto eval_erase_r0;        //   |   |s->| _ | :eval_erase_r0
			case InstructionType::ignore_separator: goto eval_erase_r0;       //   |   |/;/| _ | :eval_erase_r0
			case InstructionType::separator: goto eval_erase_r0;              //   |   | ; | _ | :eval_erase_r0
			case InstructionType::start_group: goto eval_erase_skip_r0;       //   |   | ( | _ | :eval_erase_skip_r0
			case InstructionType::start_array: goto eval_erase_skip_r0;       //   |   | [ | _ | :eval_erase_skip_r0
			case InstructionType::start_context: goto eval_erase_skip_r0;     //   |   | { | _ | :eval_erase_skip_r0
			case InstructionType::spacing: goto eval_erase_r0;                //   |   | _ | _ | :eval_erase_r0
			case InstructionType::call: goto eval_erase_r0;                   //   |   |fun| _ | :eval_erase_r0
            case InstructionType::op:                                         //   | x |o p| _ |
                switch (instruction_r2.instr) {
				case InstructionType::start: goto eval_erase_r0;              //   |s t|o p| _ | :eval_erase_r0
				case InstructionType::skip_after_next: goto eval_erase_r0;    //   |s->|o p| _ | :eval_erase_r0
				case InstructionType::skip_next: goto eval_erase_r0;          //   |s>-|o p| _ | :eval_erase_r0
				case InstructionType::ignore_separator: goto eval_erase_r0;   //   |/;/|o p| _ | :eval_erase_r0
				case InstructionType::ignore_array_start: goto eval_erase_r0; //   |/[/|o p| _ | :eval_erase_r0
				case InstructionType::start_group: goto eval_erase_r0;        //   | ( |o p| _ | :eval_erase_r0
				case InstructionType::start_array: goto eval_erase_r0;        //   | [ |o p| _ | :eval_erase_r0
				case InstructionType::start_context: goto eval_erase_r0;      //   | { |o p| _ | :eval_erase_r0
				case InstructionType::call: goto parse;                       //   |fun|o p| _ | :eval_erase_r0
				case InstructionType::op: goto eval_erase_r0;                 //   |o p|o p| _ | :parse
                case InstructionType::spacing: goto parse;                    //   | _ |o p| _ | :parse
                case InstructionType::value: goto eval_postfix;               //   |val|o p| _ | :eval_postfix
                default: goto error_syntax; }
            case InstructionType::value:                                      //   | x |val| _ | 
                switch (instruction_r2.instr) {
                case InstructionType::start: goto parse;                      //   |s t|val| _ | :parse
				case InstructionType::start_context: goto parse;              //   | { |val| _ | :parse
				case InstructionType::start_array: goto parse;                //   | [ |val| _ | :parse
				case InstructionType::start_group: goto parse;                //   | ( |val| _ | :parse
				case InstructionType::skip_after_next: goto parse;            //   |s->|val| _ | :parse
                case InstructionType::spacing: goto eval_binary_long;         //   | _ |val| _ | :eval_binary_long
                case InstructionType::op: goto eval_prefix;                   //   |o p|val| _ | :eval_prefix	//Possibly unreachable
				case InstructionType::call: goto parse;                       //   |fun|val| _ | :parse
				case InstructionType::value: goto eval_erase_r0;}             //   |val|val| _ | :eval_erase_r0
            default: goto error_syntax; 
			}
        case InstructionType::value:                                          //   |   | x |val|
            switch (instruction_r1.instr) {
			case InstructionType::skip_after_next: goto parse;                //   |   |s->|val| :parse
			case InstructionType::ignore_separator: goto parse;               //   |   |/;/|val| :parse
			case InstructionType::ignore_array_start: goto parse;             //   |   |/[/|val| :parse
			case InstructionType::start_context: goto parse;                  //   |   | { |val| :parse
			case InstructionType::start_array: goto parse;                    //   |   | [ |val| :parse
			case InstructionType::start_group: goto parse;                    //   |   | ( |val| :parse
			case InstructionType::start: goto parse;                          //   |   |s t|val| :parse
            case InstructionType::value: goto eval_coalescing;                //   |   |val|val| :eval_coalescing
			case InstructionType::spacing: goto parse;                        //   |   | _ |val| :parse
			case InstructionType::call: goto parse;                           //   |   |fun|val| :parse
            //    switch (instruction_r2.instr) {
            //    case InstructionType::value: goto eval_binary_coalescing;     //   |val| _ |val| :eval_binary_coalescing //TODO: Priority at the right operand
            //    case InstructionType::op: goto parse;                         //   |o p| _ |val| :parse //Possibly unreachable
            //    default: goto error_syntax; }
            case InstructionType::op:                                         //   | x |o p|val|
                switch (instruction_r2.instr) {
                case InstructionType::value: goto eval_binary;                //   |val|o p|val| :eval_binary
				case InstructionType::op: goto eval_prefix;                   //   |o p|o p|val| :eval_prefix
                case InstructionType::start: goto eval_prefix;                //   |s t|o p|val| :eval_prefix
				case InstructionType::ignore_separator: goto eval_prefix;     //   |/;/|o p|val| :eval_prefix
				case InstructionType::ignore_array_start: goto eval_prefix;   //   |/[/|o p|val| :eval_prefix
				case InstructionType::skip_next: goto eval_prefix;            //   |s>-|o p|val| :eval_prefix
				case InstructionType::skip_after_next: goto eval_prefix;      //   |s->|o p|val| :eval_prefix
                case InstructionType::start_group: goto eval_prefix;          //   | ( |o p|val| :eval_prefix
                case InstructionType::start_array: goto eval_prefix;          //   | [ |o p|val| :eval_prefix
                case InstructionType::start_context: goto eval_prefix;        //   | { |o p|val| :eval_prefix
                case InstructionType::spacing: goto eval_prefix;              //   | { |o p|val| :eval_prefix
				case InstructionType::call: goto eval_prefix;                 //   |fun|o p|val| :eval_prefix
                case InstructionType::separator: goto eval_prefix;            //   | ; |o p|val| :eval_prefix   //Possibly unreachable
                default: goto error_syntax; }
            default: goto error_syntax; }
        default: goto error_syntax; }

		eval_erase_r0: {
			--g_stack_instruction.max_index;
			goto evaluate; //goto parse;
		}

		eval_erase_r1: {
			g_stack_instruction.at_r(1) = g_stack_instruction.get_r(0);
			--g_stack_instruction.max_index;
			goto evaluate; //goto parse;
		}

		eval_erase_r2: {
			g_stack_instruction.at_r(2) = g_stack_instruction.get_r(1);
			g_stack_instruction.at_r(1) = g_stack_instruction.get_r(0);
			g_stack_instruction.max_index -= 2;
			goto evaluate; //goto parse;
		}

		eval_erase_skip_r0: {
			--g_stack_instruction.max_index;
			goto parse;
		}

		eval_erase_skip_r1: {
			g_stack_instruction.at_r(1) = g_stack_instruction.get_r(0);
			--g_stack_instruction.max_index;
			goto parse;
		}

		eval_erase_skip_r2: {
			g_stack_instruction.at_r(2) = g_stack_instruction.get_r(1);
			g_stack_instruction.at_r(1) = g_stack_instruction.get_r(0);
			g_stack_instruction.max_index -= 2;
			goto parse;
		}

		eval_erase_r0r1: {
			g_stack_instruction.max_index -= 2;
			goto evaluate; //goto parse;
		}

		eval_delete_r0r1: {
			g_memory_delete_span_r(2);
			goto parse;
		}

		eval_leave_r1: {
			g_stack_instruction.at_r(2) = instruction_r1;
			g_stack_instruction.max_index -= 2;
			goto parse;
		}

		eval_skip_next: {
			if (((uint32)instruction_r0.modifier) == 0) {
				g_stack_instruction.at_r(0).instr = InstructionType::ignore_separator;
				//--stacks.instructions.max_index;
				goto skip;
			} else {
				--g_stack_instruction.at_r(0).modifier;
				goto skip;
			}
		}

		eval_skip_after_next: {
			if (((uint32)instruction_r2.modifier) == 0) {
				//stacks.instructions.at_r(1) = instruction_r1;
				//stacks.instructions.at_r(2).instr = InstructionType::ignore_separator;
				//stacks.instructions.max_index -= 1;

				//stacks.instructions.at_r(2) = instruction_r1;
				//stacks.instructions.at_r(1).instr = InstructionType::ignore_separator;
				//stacks.instructions.max_index -= 1;

				g_stack_instruction.at_r(2) = instruction_r1;
				g_stack_instruction.max_index -= 2;

				goto skip;
			} else {
				// I don't know what to do here yet.
				--g_stack_instruction.at_r(0).modifier; 
				goto skip;
			}
		}

		eval_context_start: {
			g_stack_context.add(instruction_r1);

			Procedure lc_value;

			if (!g_specification->context.onEnter.count(instruction_r1.value))
				if (!g_specification->context.onEnter.count(ValueType::all))
					goto error_no_context_onEnter;
				else
					lc_value = g_specification->context.onEnter[ValueType::all];
			else
				lc_value = g_specification->context.onEnter[instruction_r1.value];

			lc_value();

			goto parse;
		}

		eval_context_finish: {
			if (g_specification->context.onExit.count(instruction_r1.value))
				g_specification->context.onExit[instruction_r1.value]();
			else if(g_specification->context.onExit.count(ValueType::all))
				g_specification->context.onExit[ValueType::all]();
			else
				goto error_invalid_op;
			
			g_stack_context.max_index -= 1;
			g_memory_delete_r(3);

			goto eval_leave_r1;
		}

		eval_context_finish_empty: {
			Procedure lc_value;

			if (!g_specification->context.onExit.count(instruction_r2.value))
				if (!g_specification->context.onExit.count(ValueType::all))
					goto error_no_context_onExit;
				else
					lc_value = g_specification->context.onExit[ValueType::all];
			else
				lc_value = g_specification->context.onExit[instruction_r2.value];

			lc_value();

			--g_stack_context.max_index;

			g_stack_instruction.max_index -= 2;

			goto parse;
		}

		eval_tuple_start: {
			g_stack_instruction.at_r(0) = Instruction::pos(InstructionType::start_array, g_memory.max_index);
			g_memory.add<Array<Instruction>*>(new Array<Instruction>());
			goto parse;
		}

		eval_tuple_add: {
			instruction_r1.shift -= instruction_r2.shift + 8;
			g_memory.get<Array<Instruction>*>(instruction_r2.shift)->add(instruction_r1);
			g_stack_instruction.max_index -= 2;
			goto parse;
		}

		eval_tuple_add_empty: { //Possibly have no reason to add this instead of using [ ...; (); ... ] //TODO: remove
			//g_stack_array.get(instruction_r2.shift)->add(Instruction::val(ValueType::tuple, 0));
			g_stack_instruction.max_index -= 1;
			goto parse;
		}

		eval_tuple_empty: { //TODO: redo
			Array<Instruction>* tuple = new Array<Instruction>();
			g_memory.add<Array<Instruction>*>(tuple);
			--g_stack_instruction.max_index;
			g_stack_instruction.add(Instruction::val(ValueType::tuple, g_memory.max_index));
			goto parse;
		}

		eval_tuple_end: {
			instruction_r1.shift -= instruction_r2.shift + 8;
 			g_memory.get<Array<Instruction>*>(instruction_r2.shift)->add(instruction_r1);	
			instruction_r2.instr = InstructionType::value;
			instruction_r2.value = ValueType::tuple;
			g_stack_instruction.at_r(2) = instruction_r2; 

			g_stack_instruction.max_index -= 2;

			goto evaluate; //goto parse;
		}

		eval_group: {
			g_stack_instruction.at_r(2) = instruction_r1;
			g_stack_instruction.max_index -= 2;

			goto evaluate;
		}

		eval_prefix: {
			String opString = String((charT*)(g_memory.content + instruction_r1.shift), instruction_r1.modifier);
			Table<ValueType, Procedure>* table;

			if (!g_specification->op.prefixForward.count(opString))
				goto eval_prefix_general_case;

			table = &g_specification->op.prefixForward[opString];

			if (table->count(instruction_r0.value))	//By string concrete value
			{
				table->at(instruction_r0.value)();
				goto evaluate;
			}
			else if (table->count(ValueType::all))	//By string general value
			{
				table->at(ValueType::all)();
				goto evaluate;
			}

			eval_prefix_general_case:

			table = &g_specification->op.prefixGeneral;

			if (table->count(instruction_r0.value))	//By type concrete value
			{
				table->at(instruction_r0.value)();
				goto evaluate;
			}
			else if (table->count(ValueType::all))	//By type general value
			{
				table->at(ValueType::all)();
				goto evaluate;
			}
			else
				goto error_invalid_op;

			goto evaluate;
		}

		eval_postfix: {
			String opString = String((charT*)(g_memory.content + instruction_r1.shift), instruction_r1.modifier);
			Table<ValueType, Procedure>* table;

			if (!g_specification->op.postfixForward.count(opString))
				goto eval_postfix_general_case;

			table = &g_specification->op.postfixForward[opString];

			if (table->count(instruction_r2.value))	//By string concrete value
			{
				table->at(instruction_r2.value)();
				goto evaluate;
			}
			else if (table->count(ValueType::all))	//By string general value
			{
				table->at(ValueType::all)();
				goto evaluate;
			}

			eval_postfix_general_case:

			table = &g_specification->op.postfixGeneral;

			if (table->count(instruction_r2.value))	//By type concrete value
			{
				table->at(instruction_r2.value)();
				goto evaluate;
			}
			else if (table->count(ValueType::all))	//By type general value
			{
				table->at(ValueType::all)();
				goto evaluate;
			}
			else 
				goto error_invalid_op;
			
			goto evaluate;
		}

		eval_binary: {
			String opString = String((charT*)(g_memory.content + instruction_r1.shift), instruction_r1.modifier);
			Table<ValueTypeBinary, Procedure>* table;

			if (!g_specification->op.binaryForward.count(opString))
				goto eval_binary_general_case;

			table = &g_specification->op.binaryForward[opString];

			if (table->count(ValueTypeBinary(instruction_r2.value, instruction_r0.value)))	//By string concrete value
			{
				table->at(ValueTypeBinary(instruction_r2.value, instruction_r0.value))();
				goto evaluate;
			}
			else if (table->count(ValueTypeBinary(instruction_r2.value, ValueType::all)))	//By string Lconcrete Rgeneral
			{
				table->at(ValueTypeBinary(instruction_r2.value, ValueType::all))();
				goto evaluate;
			}
			else if (table->count(ValueTypeBinary(ValueType::all, instruction_r0.value)))	//By string Lgeneral Rconcrete
			{
				table->at(ValueTypeBinary(instruction_r2.value, ValueType::all))();
				goto evaluate;
			}
			else if (table->count(ValueTypeBinary(ValueType::all, ValueType::all)))			//By string general
			{
				table->at(ValueTypeBinary(ValueType::all, ValueType::all))();
				goto evaluate;
			}

			eval_binary_general_case:

			table = &g_specification->op.binaryGeneral;

			if (table->count(ValueTypeBinary(instruction_r2.value, instruction_r0.value)))	//By type concrete value
			{
				table->at(ValueTypeBinary(instruction_r2.value, instruction_r0.value))();
				goto evaluate;
			}
			else if (table->count(ValueTypeBinary(instruction_r2.value, ValueType::all)))	//By type Lconcrete Rgeneral
			{
				table->at(ValueTypeBinary(instruction_r2.value, ValueType::all))();
				goto evaluate;
			}
			else if (table->count(ValueTypeBinary(ValueType::all, instruction_r0.value)))	//By type Lgeneral Rconcrete
			{
				table->at(ValueTypeBinary(instruction_r2.value, ValueType::all))();
				goto evaluate;
			}
			else if (table->count(ValueTypeBinary(ValueType::all, ValueType::all)))			//By type general
			{
				table->at(ValueTypeBinary(ValueType::all, ValueType::all))();
				goto evaluate;
			}
			else
				goto error_invalid_op;
			

			goto evaluate;
		}

		eval_binary_long: {
			
			instruction_r3 = g_stack_instruction.get_r(3);

			--scriptIndex;

			switch (instruction_r3.instr) {
			//case InstructionType::skip_after_next:
			//	goto eval_skip_after_next;

			case InstructionType::value:

				// val sp val x
				g_stack_instruction.at_r(2) = instruction_r1;
				g_stack_instruction.max_index -= 2;

				instruction_r0 = instruction_r1;
				instruction_r1 = instruction_r3;

				goto eval_coalescing;

			case InstructionType::op:

				if (g_stack_instruction.get_r(4).instr == InstructionType::spacing)
				{
					// [val sp] op sp val x
					g_stack_instruction.at_r(4) = instruction_r3;
					g_stack_instruction.at_r(3) = instruction_r1;
					g_stack_instruction.max_index -= 3;

					instruction_r0 = instruction_r1;
					instruction_r1 = instruction_r3;
					instruction_r2 = g_stack_instruction.get_r(2);

					goto eval_binary;
				}

				// [st] op sp val x
				g_stack_instruction.at_r(2) = instruction_r1;
				g_stack_instruction.at_r(1) = instruction_r0;
				g_stack_instruction.max_index -= 2;

				instruction_r0 = instruction_r1;
				instruction_r1 = instruction_r2;
				//instruction_r2 = g_stack_instruction.at_r(2);

				goto eval_binary;

			case InstructionType::call:
				++scriptIndex;
				goto parse;
				/*g_stack_instruction.at_r(2) = g_stack_instruction.get_r(1);
				g_stack_instruction.max_index -= 2;
				goto evaluate;*/
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
			
			Procedure lc_value;// = specification->binary[(*(String*)tmpPtr)][ValueTypeBinary(instruction_r2.value, instruction_r0.value)];

			if (!g_specification->op.coalescing.count(ValueTypeBinary(instruction_r1.value, instruction_r0.value)))
				if (!g_specification->op.coalescing.count(ValueTypeBinary(instruction_r1.value, ValueType::all)))
					if (!g_specification->op.coalescing.count(ValueTypeBinary(ValueType::all, instruction_r0.value)))
						if (!g_specification->op.coalescing.count(ValueTypeBinary(ValueType::all, ValueType::all)))
							goto error_invalid_op;
						else
							lc_value = g_specification->op.coalescing[ValueTypeBinary(ValueType::all, ValueType::all)];
					else
						lc_value = g_specification->op.coalescing[ValueTypeBinary(ValueType::all, instruction_r0.value)];
				else
					lc_value = g_specification->op.coalescing[ValueTypeBinary(instruction_r1.value, ValueType::all)];
			else
				lc_value = g_specification->op.coalescing[ValueTypeBinary(instruction_r1.value, instruction_r0.value)];

			lc_value();

			goto evaluate;
		}

		eval_coalescing_long: {
			g_stack_instruction.at_r(1) = instruction_r0;
			--g_stack_instruction.max_index;
			
			goto eval_coalescing;
		}

		eval_cleanup_separator: {
			g_stack_instruction.at_r(1) = instruction_r0;
			--g_stack_instruction.max_index;

			goto evaluate;
		}

		eval_expr: {
			charT* function = (charT*)malloc(256*sizeof(charT));
			uint32 max_index = 255;
			uint32 index = -1;

			uint32 depth = 0;

			charT ch;
			while (true) {
				++index;
				switch (ch = script[++scriptIndex])
				{
				case T('\0'):
					goto error_syntax;
				case T('\1'):
					stream->read(script, string_buffer_init_size);
					if (stream->eof())
						script[stream->gcount()] = T('\0');
					scriptIndex = -1;
					break;
				case T('{'):
					++depth;
					break;
				case T('}'):
					if (depth == 0) {
						function[index] = 0;
						--g_stack_instruction.max_index;
						g_stack_instruction.add(Instruction::val(ValueType::expression, g_memory.max_index));
						g_memory.add(function);
						goto evaluate;
					}
					else
						--depth;
					break;
				default:
					if (index > max_index)
					{
						max_index = ((max_index + 1) << 1) - 1;
						realloc(function, max_index);	//TODO: check for memory
					}
					function[index] = ch;
					break;
				}
			}

			/*
			__m128i context_finish = _mm_set1_epi8('{');
			__m128i chunk = _mm_loadu_si128((__m128i*)(script + scriptIndex));
			uint16 cmp_result;
			if (cmp_result = _mm_cmpeq_epi8_mask(chunk, context_finish))
			{
				if ((uint8)cmp_result) {

				}
			}
			*/
		}

		eval_finish_call: {
			script = *(charT**)(g_memory.content + instruction_r2.shift);
			scriptIndex = *(int*)(g_memory.content + instruction_r2.shift + sizeof(void*));
			g_stack_instruction.at_r(2) = g_stack_instruction.get_r(3);
			g_stack_instruction.at_r(3) = g_stack_instruction.get_r(1);
			g_stack_instruction.max_index -= 2;
			goto evaluate;
		}
	}

	//Errors
	{
	error_memory_allocation: //TODO: Before throwing error should try freeing up memory
		g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_memory_allocation });
		goto evaluate;

	error_string_missing_closing_quote:
		g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_string_missing_closing_quote });
		goto evaluate;

	error_syntax:
		g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_syntax });
		goto evaluate;

	error_invalid_op:
		g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_invalid_op });
		goto evaluate;

	error_no_context_onEnter:
		g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_no_context_onEnter });
		goto evaluate;

	error_no_context_onExit:
		g_stack_instruction.add(Instruction{ InstructionType::error, ValueType::none, (int16)Status::error_no_context_onExit });
		goto evaluate;
	}

ending:
	return Status::success;
}

void g_memory_delete_top() {
	Instruction instr = g_stack_instruction.get_r(0);
	switch (instr.instr) {
	case InstructionType::value:
		if (g_specification->type.destructor.count(instr.value)) {
			((Destructor)g_specification->type.destructor[instr.value])(g_memory.content + instr.shift);
		}
		--g_stack_instruction.max_index;
		g_memory.max_index = instr.shift;
		break;
	default:
		--g_stack_instruction.max_index;
		break;
	}
}

void g_memory_delete_r(size_t index) {
	Instruction instr = g_stack_instruction.get_r(index);
	size_t iter = index;

	switch (instr.instr) {
	case InstructionType::value:
		if (g_specification->type.destructor.count(instr.value)) {
			((Destructor)g_specification->type.destructor[instr.value])(g_memory.content + instr.shift);
		}
	case InstructionType::op:
		size_t size;
		switch (instr.value) {
		case ValueType::string:
		case ValueType::name:
			size = instr.modifier;
			g_memory.erase(instr.shift, size);
			break;
		default:
			size = g_specification->type.size[(size_t)instr.value];
			g_memory.erase(instr.shift,size);
			break;
		}

		while (iter) {
			instr = g_stack_instruction.get_r(--iter);
			switch (instr.instr) {
			case InstructionType::value:
			case InstructionType::op:
				instr.shift -= size;
			default:
				g_stack_instruction.at_r(iter+1) = instr;
				break;
			}
		}
		g_stack_instruction.at_r(1) = g_stack_instruction.get_r(0);
	default:
		--g_stack_instruction.max_index;
		break;
	}
}

void g_memory_delete_span_r(size_t index) {
	++index;
	Instruction instr;
	while(--index){	// We get the top element of stack and then pop it, so no need for direct index of the instruction.
		instr = g_stack_instruction.get_r(0);
		switch (instr.instr) {
		case InstructionType::value:
			if (g_specification->type.destructor.count(instr.value)) {
				((Destructor)g_specification->type.destructor[instr.value])(g_memory.content + instr.shift);
			}
			g_memory.max_index = instr.shift;
			--g_stack_instruction.max_index;
			break;
		case InstructionType::op:
			g_memory.max_index = instr.modifier;
			--g_stack_instruction.max_index;
			break;
		default:
			--g_stack_instruction.max_index;
			break;
		}
	}
}

void g_memory_delete_span_r(size_t indexBegin, size_t indexEnd) {

}