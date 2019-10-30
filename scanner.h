/*
 *	Predmet : Formální jazyky a překladače									 
 *	Projekt : Implementace překladače imperativního jazyka IFJ19			 
 *	Súbor	: scanner.h														 
 *				-hlavičkový súbor pre náš scanner (lexikálny analyzátor)	 
 *	Tím		: číslo 10 / varianta II										 
 *	Autori	: Dominik Boboš (xbobos00)										 
 *			  Peter Hudeček (xhudec34)										 
 *			  Timotej Kováčik (xkovac49)									 
 *			  Dávid Oravec (xorave05)										 
 *																			 
 */

#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <stdio.h>
#include "str.h"
#include "error.h"

typedef enum {
	STATE_START,					// startovaci stav {S}
	STATE_ID_OR_KEYWORD,			// konecny stav {F20}
	STATE_INT,						// konecny stav intu {F1}
	STATE_FLOAT,					// konecny stav {F3}
	STATE_FLOAT_POINT,				// medzistav {Q1}
	STATE_FLOAT_POINT_NUMBER,		// konecny stav {F2}
	STATE_FLOAT_EXP,				// medzisstav {Q2}
	STATE_FLOAT_EXP_OP,				// medzistav {Q3}
	STATE_STRING_START,				// medzistav {Q6}
	STATE_BACKSLASH,				// medzistav {Q7}
	STATE_BACKSLASH_X,				// medzistav {Q8}
	STATE_BACKSLASH_X_NUM,			// medzistav {Q9}		
	STATE_DIVIDE,					// konecny stav {F7}
	STATE_GREATER_THAN,				// konecny stav {F11}
	STATE_LESS_THAN,				// konecny stav {F9}
	STATE_EQUALS,					// konecny stav {F17}
	STATE_NOT_EQUAL_START,			// medzistav {Q4}
	STATE_COMMENT,					// medzistav {Q5}
	STATE_BLOCK_COMMENT_START,		// medzistav {Q10}
	STATE_BLOCK_COMMENT_CONTINUE,	// medzistav {Q11}
	STATE_BLOCK_COMMENT,			// medzistav {Q12}
	STATE_BLOCK_COMMENT_LEAVE_TRY,	// medzistav {Q13}
	STATE_BLOCK_COMMENT_LEAVE,		// medzistav {Q14}
	STATE_EOL,
} state;

typedef enum
{									//V ZADANI 3.1 !!PY je CASE SENSITIVE!!
	//TODO vstavane funkcie su tiez  klucove slova
	KEYWORD_DEF,					//def
	KEYWORD_ELSE,					//else
	KEYWORD_IF,						//if
	KEYWORD_NONE,					//None
	KEYWORD_PASS,					//pass - prikaz ktory nic nerobi
	KEYWORD_RETURN,					//return
	KEYWORD_WHILE,					//while
	KEYWORD_INPUTS,
	KEYWORD_INPUTI,
	KEYWORD_INPUTF,
	KEYWORD_PRINT,
	KEYWORD_LEN,
	KEYWORD_SUBSTR,
	KEYWORD_CHR,
	KEYWORD_ORD,
	//KEYWORD_AND,					//su v py3 ale neviem ci aj v IFJ19
	//KEYWORD_AS,
	//KEYWORD_ASSERT,
	//KEYWORD_CLASS,
	//KEYWORD_DEL,
	//KEYWORD_EXCEPT,
	//KEYWORD_FINALLY,
	//KEYWORD_FROM,
	//KEYWORD_GLOBAL,
	//KEYWORD_IMPORT,
	//KEYWORD_IN,					//v zadani sa o nich nehovori, aspon som 
	//KEYWORD_IS,					//ich tam ja nikde nevidel spomenute
	//KEYWORD_LAMBDA,				//ale pre python su viacere z nich dost
	//KEYWORD_NONLOCAL,				//dolezite tak neviem
	//KEYWORD_NOT,
	//KEYWORD_OR,
	//KEYWORD_RAISE,
	//KEYWORD_TRY,
	//KEYWORD_WITH,
	//KEYWORD_YIELD,				

	//KEYWORD_FOR,					//v rozsireniach (12.7.1)
	//KEYWORD_BREAK,
	//KEYWORD_CONINUE,
	//KEYWORD_ELIF,
	//KEYWORD_TRUE,
	//KEYWORD_FALSE,
} Keyword;

typedef enum 		///TODO need help with these check-if there are all we need
{
	/*
		Datove typy
	*/
	TYPE_INT,
	TYPE_FLOAT,						///float or double?
	TYPE_STRING,
	/*
		ID / Keyword / OtherStuff
	*/
	TYPE_IDENTIFIER, 				/// Identifier
	TYPE_KEYWORD, 					/// Keyword
	TYPE_EOL,
	TYPE_EOF,
	TYPE_EMPTY,	///ak ani jeden z tychto, prip. placeholder dokym nenajdeme dalsie
	/*
		matematicke operacie
	*/
	//TYPE_BACKSLASH,				///backslash je vlastne na delenie ci?		
	TYPE_PLUS,						//	'+'
	TYPE_MINUS,						//	'-'
	TYPE_MULTIPLY,					//	'*'
	TYPE_DIVIDE,					//	'/'
	TYPE_DIVIDE_INT,				//	'//'
	TYPE_ASSIGN_VALUE,//mame toto vobec popisane v stavoch? ked priradzujeme hodnotu?
	/*
		logicke operacie
	*/
	TYPE_GREATER_THAN,				//	>
	TYPE_LESS_THAN,					// 	<
	TYPE_GREATER_EQUAL,				//	>=
	TYPE_LESS_EQUAL,				//  <=
	TYPE_EQUALS,			//tieto harmim nema :D ale podla mna su useful
	TYPE_EQUAL_TO,
	TYPE_NOT_EQUAL,			// 			--||--...alebo je to ekvivalent equals?
	/*
		Znaky + Space
	*/		
	TYPE_LEFT_PAR,					//	'('
	TYPE_RIGHT_PAR,					//	')'
	TYPE_COMMA,						// 	','
	TYPE_SEMICOLON,					//	';'
	TYPE_SPACE,						///kvoli indentom je to fajn sledovat nie?				
} token_type;

typedef struct {
    string *string;
    int int_value;
    double decimal_value;
    Keyword keyword;
} token_attribute;

typedef struct {
    token_type type;
    token_attribute attribute;
} token;


int getNextToken(token *token);

#endif	// !_SCANNER_H_