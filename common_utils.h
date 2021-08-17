#pragma once

//Cases
//These macros are supposed to be used within switch statements (colon required at the end of macro)

//Case for numbers (range 0-9)
#define char_number T('0'): case T('1'): case T('2'): case T('3'): case T('4'): case T('5'): case T('6'): case T('7'): case T('8'): case T('9')

//Case for operator characters
#define char_operator 										\
	     T('<'): case T('>'): case T('='):					\
	case T('@'): case T('?'): case T(','):					\
	case T('-'): case T('+'): case T('*'): case T(':'):		\
	case T('/'): case T('^'): case T('\\'): case T('%'):	\
	case T('&'): case T('|'):								\
	case T('.'): case T('~')								

//Case for whitespace characters
#define char_space_character								\
	     T('\n'): case T('\r'): case T('\t'): case T(' ')	

//Case for special characters
#define char_special_character								\
		 T(';'):											\
	case T('['): case T('{'): case T('('):					\
	case T(']'): case T('}'): case T(')')					


