// $Id$

/*
    CompSL scripting language 
    Copyright (C) 2007  Thomas Jones & John Peberdy

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef BYTECODE_H_
#define BYTECODE_H_

#include "compsl.h"


// all instructions working with variables take address in a1 unless otherwise
// stated

COMPSL_INTERN typedef enum  { 
	BC_NOOP = 0, // do nothing
	BC_PUSH, 	//push local var
	BC_PSHT, 	//push temp register
	BC_APUSH, 	//push element from local array (index on stack)
	BC_CPUSH, 	//push constant
	BC_GPSH,	//same as PUSH but for global
	BC_GAPS, 	//samp as APUSH but for global
	
	BC_POP, 	//pop to local var
	BC_POPT, 	//pop to local temp register
	BC_APOP,	//pop into local array (index on top of stack, value is next)
	BC_DPOP, 	//pop and discard value
	BC_GPOP,	//same as POP but for global
	BC_GAPP,	//same as APOP but for global
	
	BC_DUP, 	//duplicate top of stack
	
	BC_CALL, 	//call native (native ID in a1, args on stack, ltr w/rightmost @ top)
	
	BC_ADD, 	//add top 2 stack elements (integer)
	BC_SUB, 	//subtract top 2 stack elements b = pop; a = pop; push a - b;
	BC_MUL, 	//multiply top 2 stack elements 
	BC_DIV ,	//divide top 2 stack elements b = pop; a = pop; push a / b;
	BC_LE,		// <=
	BC_LS,		// <
	BC_EQ,		// ==
	BC_NE,		// !=
	BC_GR,		// >
	BC_GE,		// >=
	BC_MOD,		//modulus of top 2 stack b = pop; a = pop; push a % b;
	BC_FMOD,	//same as MOD only for float
	BC_AND,		//boolean and of top two stack elements
	BC_OR,		//boolean or of top two stack elements
	BC_NOT, 	//boolean not of top stack element
	BC_BAND,	//bitwise and of top two stack elements
	BC_BOR,		//bitwise or of top two stack elements
	BC_BXOR,	//bitwise xor of top two stack elements
	BC_BNOT,	//bitwise not of top stack element
	BC_SFTL,	//shift left, top of stack is shift, next is value
	BC_SFTR,	//shift right, top of stack is shift, next is value
	BC_FADD,	//same as ADD but for float
	BC_FSUB, 	//same as SUB but for float
	BC_FMUL, 	//same as MUL but for float
	BC_FDIV,	//same as DIV but for float
	BC_FLE,		// <=
	BC_FL,		// <
	BC_FEQ,		// ==
	BC_FNE,		// !=
	BC_FGR,		// >
	BC_FGE,		// >=
	
	BC_JMP, 	//jump to offeset in sa from current positon
	BC_JMZ,		//if top of stack is zero, jump, always pop and discard top of stack
	BC_JMN,		//if top of stack is non-zero, jump
	BC_FLIN,	//cast float to int (top of stack)
	BC_INFL, 	//cast int to float (top of stack)
	BC_SAVE,	//save top of stack to temp register without moving stack pointer
	BC_STO,		//save top of stack to local variable without moving stack pointer
	BC_GSTO,	//save top of stack to global variable without moving stack pointer
	BC_INC,		//increment top of stack
	BC_DEC,		//decrement top of stack
	BC_FINC,	//increment top of stack
	BC_FDEC,	//decrement top of stack
	
	//builtins
	BC_ABS,		BC_ABSF,	BC_SIN,		BC_COS,		//28-2B
	BC_TAN,		BC_ASIN,	BC_ACOS,	BC_ATAN,	//2C-2F
	BC_SQRT,	BC_LN,		BC_FLOOR,	BC_CEIL,	//30-33
	BC_RAND,	BC_ATAN2,	BC_POW,		BC_MIN,		//34-37
	BC_MAX,		BC_MINF,	BC_MAXF,	BC_HYPOT,	//38-3B
	BC_FEQUAL,
	
	//misc
		BC_PYES,	BC_NONO,	BC_END, 	BC_HLT, 	BC_DBG
	} BC_DEF;

typedef struct
{
  BC_DEF code;
  union 
  {
    struct
    {
      uint8_t a1 /*__deprecated__*/; // uncomment depreciated after first relase
      uint8_t a2; // clean up all refernces to it
    };
    uint16_t a; // look up wether this works as expected
    int16_t sa;
  };
} bytecode;

#endif /*BYTECODE_H_*/
