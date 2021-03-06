/*
 *	Predmet : Formální jazyky a překladače
 *	Projekt : Implementace překladače imperativního jazyka IFJ19
 *	Súbor	: parser.c
 *				-parser - modul spracováva syntaktickú analýzu 
 *						a sémantickú analýzu
 *	Tím		: číslo 10 / varianta II
 *	Autori	: Dominik Boboš (xbobos00)
 *			  Peter Hudeček (xhudec34)
 *			  Timotej Kováčik (xkovac49)
 *			  Dávid Oravec (xorave05)
 *
 */

#include "parser.h"
#include "scanner.h"
#include "expr.h"
#include <ctype.h>
#include "generator.h"

#define TRUE 1
#define FALSE 0

static int params(ParserData *data);

static int statement(ParserData *data);

static int statement_next(ParserData *data);


/*
 *  Funkcia na porovnavanie očakávaného token.typu
 *  s tym co nam realne prijde so suboru vo forme dalsieho
 *  tokenu
 */
int checkTokenType(token *Token, token_type type)
{
    int returncode = 0;
    if((returncode = getNextToken(Token)) == TOKEN_OK)
    {
        if (type == Token->type)
            return SYNTAX_OK;
        else
            return ERROR_PARSER;
    }
    return returncode;
}


/*	
*	Funkcia na zabezpecenie spravnej syntaxe
*	v pripade ze nastane nepriazniva situacia
*	koniec viac-riadkoveho komentara TYPE_COMMENT
*	a za nim nenasleduje TYPE_EOL
*
*	Vyuziva Token data zo struct ParserData
*/
int isComment(ParserData *data)
{
	int result = 0;
	if(data->Token.type == TYPE_COMMENT)
	{
		///ak nasledujuci token nie je eol, vrat chybu
		if ((result = checkTokenType(&data->Token, TYPE_EOL)) == 0)
			return SYNTAX_OK;
		else return result;
	}
	else if (data->Token.type == TYPE_EOL) return SYNTAX_OK;
	else return ERROR_PARSER;
}


/*
*   Pomocna funkcia, aby som usetril riadky a zvysil prehladnost
*   Funkcia prida identifikator do globalnej (bool false) alebo 
*   do lokalnej hash tabulky (bool true)
*   int position - 0-LEFT 1-CURRENT 2-RIGHT
*/
int addToHash(ParserData *data, bool isLocal, int position) {
    bool errIntern;
    ///pridavam do lokalnej tabulky
    if (isLocal == true) {

        switch (position) {
            case 0:
                data->leftID = htabAddSymbol(&data->localT, data->Token.attribute.string->str, &errIntern);
                if (errIntern == true) return ERROR_INTERN;
                break;
            case 1:
                data->currentID = htabAddSymbol(&data->localT, data->Token.attribute.string->str, &errIntern);
                if (errIntern == true) return ERROR_INTERN;
                break;
            case 2:
                data->rightID = htabAddSymbol(&data->localT, data->Token.attribute.string->str, &errIntern);
                if (errIntern == true) return ERROR_INTERN;
                break;
            default:
                return ERROR_INTERN;
        }
    } 
    else {
        ///case kedy pridavam do global
        switch (position) {
            case 0:
                data->leftID = htabAddSymbol(&data->globalT, data->Token.attribute.string->str, &errIntern);
                if (errIntern == true) return ERROR_INTERN;
                data->leftID->isGlobal = true;
                break;
            case 1:
                data->currentID = htabAddSymbol(&data->globalT, data->Token.attribute.string->str, &errIntern);
                if (errIntern == true) return ERROR_INTERN;
                data->currentID->isGlobal = true;
                break;
            case 2:
                data->rightID = htabAddSymbol(&data->globalT, data->Token.attribute.string->str, &errIntern);
                if (errIntern == true) return ERROR_INTERN;
                data->rightID->isGlobal = true;
                break;
            default:
                return ERROR_INTERN;
        }
    }
    return SYNTAX_OK;
}


/*
 * PRAVIDLO <prog>
 */
static int prog(ParserData* data)
{
	static int result;

/*  *   *   *   *   *   *   *   vzdy si na zaciatku pytam token     *   *   *   *   *   *   */
    if (data->Token.attribute.keyword != KEYWORD_DEF ) {
        if ((result = getNextToken(&data->Token)) != 0) return result;
    }

/*  *   *   *   *   *   *   *   vzdy si na zaciatku pytam token     *   *   *   *   *   *   */

/**************************************************** D E F **************************************************************
*1.<prog> -> KEYWORD_DEF TYPE_IDENTIFIER(<params>)TYPE_COLON TYPE_EOL TYPE_INDENT <statement> TYPE_EOL TYPE_DEDENT <prog>*
**************************************************************************************************************************/
    if (data->Token.attribute.keyword == KEYWORD_DEF && data->Token.type == TYPE_KEYWORD) {
        data->in_declaration = 1;           //je true

        // "Identifikátor funkcie"
        if ((result = checkTokenType(&data->Token, TYPE_IDENTIFIER)) == 0) {
            //pridam identifier do tabulky symbolov// checknem ci nenastava redefinicia
        if ((data->currentID = htabSearch(&data->globalT, data->Token.attribute.string->str)) != NULL &&
            data->currentID->isDefined == true) return ERROR_PROGRAM_SEMANTIC;
            ///pokial sa najde zhoda, je tu pokus o redefiniciu

        if ((data->currentID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL ) {

            bool errIntern;
            data->currentID = htabAddSymbol(&data->globalT, data->Token.attribute.string->str, &errIntern);
            if (errIntern == true) return ERROR_INTERN;

        }
            GENERATE(genFunctionHead,data->Token.attribute.string->str);
            data->leftID = NULL;
       	    //data->not_declared_function = 0;    //lebo ju deklarujeme
            data->currentID->isDefined = true;
        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) == 0) {
            data->paramIndex = 0;
        	//preskocim na params (poslem im "data" ktore uchovavaju info
        	//ci sme v deklaracii alebo mimo nej atd..
        if ((result = params(data)) == 0) {

        if (data->Token.type == TYPE_RIGHT_PAR) {  //toto je hlavne test, ptom sa toto ifko moze odstranit
        if ((result = checkTokenType(&data->Token, TYPE_COLON)) == 0) {
    	if (((result = checkTokenType(&data->Token, TYPE_EOL)) == 0)) {
        while (data->Token.type != TYPE_INDENT) {
            if (data->Token.type == TYPE_EOL) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
            } else return ERROR_PARSER;
        }
        if(data->Token.type == TYPE_INDENT) {

            data->in_function = 1;


/*  *   *   *   *   pytam si dalsi token lebo statement si sam nepyta   *   *   *   *   *   *   */
        if ((result = getNextToken(&data->Token)) != 0) return result;
        if ((result = statement(data)) != 0) {
            if (data->was_return == TRUE && result == 2) {          ///bol return
                if ((result = getNextToken(&data->Token)) != 0) return result;      ///pytam si dalsi token lebo return nepyta
                }
            else return result;      //nieco sa posralo v statement
        }
        if (data->was_return == TRUE && result == 0) {
            if ((result = getNextToken(&data->Token)) != 0) return result;      ///pytam si dalsi token lebo return nepyta
            result = 0;
        } else {
            GENERATE(genFunctionReturn,DTYPE_UNDEFINED);
        }
        data->was_return = FALSE;

        if (data->Token.attribute.keyword == KEYWORD_DEF) {     ///bez tohto sa potom v progu nepyta dalsi token
            data->Token.attribute.keyword = KEYWORD_PASS;       ///hocijaky iny keyword tu musim dat okrem DEF
        }

        while (data->Token.type != TYPE_DEDENT) {
            if (data->Token.type == TYPE_EOL) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
            } else return ERROR_PARSER;
        }
        data->in_declaration = 0;
        data->in_function = 0;

        GENERATE(genFunctionEnd, data->currentID->identifier);

        if(data->Token.type == TYPE_EOF) return SYNTAX_OK;
        //rekurzia aby som sa vratil spat do <prog>
        else {
            //po kazdej definicii funkcie treba uvolnovat tabulku lokalnych premennych
            htabFree(&data->localT);
            return result = prog(data);
        }

        }
        else return result;	//neprisiel indent
   		}
   		else return result; //nebol EOL 
        }
        else return ERROR_PARSER;  //neprisla ':'
        }
        else return ERROR_PARSER;  ///ked sa nieco pokazilo a neprisla mi z params(&data) ')'
        }
        else return result;
        }
        else return result; //neprisla lava zatvorka
        }
        else return result;	// bolo def, ale nebol spravny identifikator
    }
/*  *   *   *   *   *   *   4.  <prog> -> TYPE_EOF <end>    *   *   *   *   *   *   *   */
/*  *   *   *   *   *   *   *   * 24. <end> -> ε    *   *   *   *   *   *   *   *   *   */
    else if (data->Token.type == TYPE_EOF) {
        return result = SYNTAX_OK;
    }
    else if ((result = isComment(data)) == 0) {    //ak dokumentacny retazec alebo basic EOL
        return result = prog(data);
    }
/*  *   *   *   *   * 3.  <prog> -> <statement> TYPE_EOL <prog> *   *   *   *   *   *   */
    else {
        if (result == 1) return result;
	    else return result = statement(data);
	}
}



/*
 * FUNKCIA NA PRIRADENIE PARAMETROV DO FUNKCIE
 * PRÍPADNE KONTROLA ČI SÚ PRI VOLANÍ FUNKCIE
 * SPRÁVNE VOLANÉ VŠETKY PARAMETRE
 */
static int params(ParserData *data)
{
    static int result;


    //som v deklaracii funkcie
    if ((data->leftID == NULL && data->in_declaration == 1) ) {
        if ((result = checkTokenType(&data->Token, TYPE_IDENTIFIER)) == 0) {
        
            if ((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL) {

                switch (data->Token.type) {
                    case TYPE_INT:
                        stringAddChar(data->currentID->param, 'i');
                        break;
                    case TYPE_FLOAT:
                        stringAddChar(data->currentID->param, 'f');
                        break;
                    case TYPE_STRING:
                        stringAddChar(data->currentID->param, 's');
                        break;
                    default:
                        stringAddChar(data->currentID->param, 'u');
                        break;
                }
            }
            else {
                switch (data->rightID->type) {
                    case DTYPE_INT:
                        stringAddChar(data->currentID->param, 'i');
                        break;
                    case DTYPE_DOUBLE:
                        stringAddChar(data->currentID->param, 'f');
                        break;
                    case DTYPE_STRING:
                        stringAddChar(data->currentID->param, 's');
                        break;
                    default:
                        stringAddChar(data->currentID->param, 'u');
                        break;
                }
            }
            data->paramIndex += 1;
            bool errIntern;

            if (!(data->rightID = htabAddSymbol(&data->localT, data->Token.attribute.string->str, &errIntern))){
                if (errIntern == true) return ERROR_INTERN;
                else return ERROR_PROGRAM_SEMANTIC; ///redefinicia
            }

            GENERATE(declareFunctionParam, data->Token.attribute.string->str, data->paramIndex);
            //nacitavam dalsi token ak je ciarka ocakavam dalsi param.
            //21. 	<param_next> -> TYPE_COMMA TYPE_IDENTIFIER <param_next>
            if ((result = checkTokenType(&data->Token, TYPE_COMMA)) == 0) {
                result = params(data);    //<param_next>
            } else if (data->Token.type == TYPE_RIGHT_PAR) {
                if (data->currentID->previouslyCalled == TRUE) {        ///ak bola predtym volana ale nie definovana checknem ci sedi pocet parametrov
                    if (data->currentID->paramCount != data->paramIndex) return ERROR_WRONG_NUMBER_OF_PARAMS;

                    else {
                        data->currentID->previouslyCalled = FALSE;
                        return SYNTAX_OK;
                    }
                }
                data->currentID->paramCount = data->paramIndex;
                //ulozim ze dana funkcia ma zatial N paramaterov podla data->paramIndex
                return SYNTAX_OK;
            } else if (result == 1) return ERROR_SCANNER;
            else return ERROR_PARSER;   //neprisla ciarka ani prava zatvorka
        }
        //nacitany token je prava zatvorka -> <params> -> ε
        else if ((data->Token.type == TYPE_RIGHT_PAR) && (data->paramIndex == 0)) {
            //ulozim ze dana funcia nepotrebuje parametre

            if (data->currentID->previouslyCalled == TRUE) {        ///ak bola predtym volana ale nie definovana checknem ci sedi pocet parametrov
                if (data->currentID->paramCount != data->paramIndex) return ERROR_WRONG_NUMBER_OF_PARAMS;
                else {
                    data->currentID->previouslyCalled = FALSE;
                    return SYNTAX_OK;
                }
            }
            data->currentID->paramCount = data->paramIndex;
            return SYNTAX_OK;
        }
        else
            return ERROR_PARSER; ///param index bol vacsi ako nula ale dosla mi hned zatvorka
    }                            ///pripadne za ciarkou neprisiel dalsi identif

    //ten divny pripad kedy volam v definicii funkcie nedefinovanu funkciu
    else if(data->leftID->isDefined == false && data->in_declaration == 1) {
        if ((result = checkTokenType(&data->Token, TYPE_IDENTIFIER)) == 0 || data->Token.type == TYPE_INT ||
                data->Token.type == TYPE_FLOAT || data->Token.type == TYPE_STRING || data->Token.attribute.keyword == KEYWORD_NONE) {
            switch (data->Token.type) {
                case TYPE_INT:
                    stringAddChar(data->leftID->param, 'i');
                    break;
                case TYPE_FLOAT:
                    stringAddChar(data->leftID->param, 'f');
                    break;
                case TYPE_STRING:
                    stringAddChar(data->leftID->param, 's');
                    break;
                case KEYWORD_NONE:
                    stringAddChar(data->leftID->param, 'u');
                    break;
                default:
                    stringAddChar(data->leftID->param, 'u');
                    break;
            }
            data->paramIndex += 1;

            //nacitavam dalsi token ak je ciarka ocakavam dalsi param.
            //21. 	<param_next> -> TYPE_COMMA TYPE_IDENTIFIER <param_next>
            if ((result = checkTokenType(&data->Token, TYPE_COMMA)) == 0)
                params(data);	//<param_next>
            else if (data->Token.type == TYPE_RIGHT_PAR) {
                data->leftID->paramCount = data->paramIndex;
                data->leftID->previouslyCalled = TRUE;
                //ulozim ze dana funkcia ma zatial N paramaterov podla data->paramIndex
                return SYNTAX_OK;
            }
            else return ERROR_PARSER;   //neprisla ciarka ani prava zatvorka
        }
        //nacitany token je prava zatvorka -> <params> -> ε
        else if ((data->Token.type == TYPE_RIGHT_PAR) && (data->paramIndex == 0)) {
            //ulozim ze dana funcia nepotrebuje parametre
            data->leftID->paramCount = data->paramIndex;
            data->leftID->previouslyCalled = TRUE;
            return SYNTAX_OK;
        }
        else
            return ERROR_PARSER; ///param index bol vacsi ako nula ale dosla mi hned zatvorka
    }                            ///pripadne za ciarkou neprisiel dalsi identif
    return result;
}


/*
 * PRAVIDLO STATEMENT
 */
static int statement(ParserData *data)
{ 
    static int result;


/******************************************** I F *************************************************      
*            	5. 	<statement> -> KEYWORD_IF <expression> TYPE_COLON                             *
*            	TYPE_EOL TYPE_INDENT <statement> TYPE_DEDENT KEYWORD_ELSE TYPE_COLON              *
*            	TYPE_EOL TYPE_INDENT <statement> TYPE_DEDENT <statement_next>                     *
***************************************************************************************************/
    if ((data->Token.type == TYPE_KEYWORD) && (data->Token.attribute.keyword == KEYWORD_IF)) {
    	data->in_if = 1;
    	data->uniqLabel +=1;
    	int tempLabel = data->uniqLabel;

        data->leftID = htabSearch(&data->globalT,"%return");
        if (data->leftID) {
            data->leftID->type = DTYPE_BOOL;
        } else if (data->currentID) data->currentID->type = data->currentID->type;
        else return ERROR_PROGRAM_SEMANTIC;

/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */
        if ((result = getNextToken(&data->Token)) != 0) return result;
        if ((result = expression(data)) != 0 ) return result;
/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */

        //COLON a.k.a dvojbodka	, pytal som si token v expression
        if(data->Token.type == TYPE_COLON) {      
        if((result = checkTokenType(&data->Token, TYPE_EOL)) == 0) {  
		GENERATE(generateIfStart,tempLabel);
        while (data->Token.type != TYPE_INDENT) {
            if (data->Token.type == TYPE_EOL) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
            } else return ERROR_PARSER;
        }
        if(data->Token.type == TYPE_INDENT) {
            //rekurzia pre vnutro IF-u
            if ((result = getNextToken(&data->Token)) != 0) return result;
			if((result = statement(data)) != SYNTAX_OK) return result;
		//DEDENT zo statement_next sa mi vypytal dalsi token, nemusim pytat novy
		if(data->Token.type == TYPE_DEDENT) {
		if (((result = checkTokenType(&data->Token, TYPE_KEYWORD)) == 0) &&
			data->Token.attribute.keyword == KEYWORD_ELSE) {
			GENERATE(generateIfPre,tempLabel);
		if((result = checkTokenType(&data->Token, TYPE_COLON)) == 0) {
		if((result = checkTokenType(&data->Token, TYPE_EOL)) == 0) {
        while (data->Token.type != TYPE_INDENT) {
            if (data->Token.type == TYPE_EOL) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
            } else return ERROR_PARSER;
        }
            if(data->Token.type == TYPE_INDENT) {
			//rekurzia pre vnutro ELSE
			
			GENERATE(generateIf,tempLabel);
			
            if ((result = getNextToken(&data->Token)) != 0) return result;
            if((result = statement(data)) != SYNTAX_OK) return result;
            if (data->was_return == TRUE) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
                data->was_return=FALSE;
            }
		//DEDENT zo statement_next sa mi vypytal dalsi token, nemusim pytat novy
        while (data->Token.type != TYPE_DEDENT) {
            if (data->Token.type == TYPE_EOL) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
            } else return ERROR_PARSER;
        }
		if(data->Token.type == TYPE_DEDENT) {
			data->in_if = 0;
			
			GENERATE(generateIfEnd,tempLabel);
			
/*  *   *   *   *   *   *   pokracovanie statementov    *   *   *   *   *   *   *   */
            if ((result = getNextToken(&data->Token)) != 0) return result;
			if ((result = statement_next(data)) != SYNTAX_OK) return result;
/*  *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   */
		}       //neprisiel DEDENT
		else return ERROR_PARSER;
		}       //neprisiel INDENT
		else return result;
		}       //neprisiel EOL
		else return result;
		}       //neprisiel COLON
		else return result;
		}       //neprisiel ELSE
		else return result;
		}       //neprisiel DEDENT
		else return ERROR_PARSER;
		}       //neprisiel INDENT
		else return result;
		}       //neprisiel EOL
		else return result;
		} 		//neprisiel COLON
		else return ERROR_PARSER;
    }


/******************************************** W H I L E********************************************      
*               6. <statement> -> KEYWORD_WHILE <expression> TYPE_COLON                           *
*                   TYPE_EOL TYPE_INDENT <statement> TYPE_EOL                                     *
*                   TYPE_DEDENT <statement_next>                                                  *
***************************************************************************************************/
/*6. <statement> -> KEYWORD_WHILE <expression> TYPE_COLON TYPE_EOL TYPE_INDENT <statement> TYPE_EOL TYPE_DEDENT <statement_next>*/
    //WHILE
    else if ((data->Token.type == TYPE_KEYWORD) && (data->Token.attribute.keyword == KEYWORD_WHILE)) {
		data->in_while = 1;
    	data->uniqLabel +=1;

        data->leftID = htabSearch(&data->globalT,"%return");
        if (data->leftID) {
            data->leftID->type = DTYPE_BOOL;
        } else if (data->currentID) data->currentID->type = data->currentID->type ;
    	//vytvaram si to nato lebo vnutro WHILE by mohlo byt IF/WHILE co by inkrementovalo uniqLabel a dolny jump by som jumpoval zle
    	int docasnyLabel = data->uniqLabel;
    	generateWHILElabel(docasnyLabel);
        
/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */
        if ((result = getNextToken(&data->Token)) != 0) return result;
        if ((result = expression(data)) != 0 ) return result;
        generateWHILEcondition(docasnyLabel);
/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */

		if (data->Token.type == TYPE_COLON) {
	    if ((result = checkTokenType(&data->Token, TYPE_EOL)) == 0) {
        while (data->Token.type != TYPE_INDENT) {
            if (data->Token.type == TYPE_EOL) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
            } else return ERROR_PARSER;
        }
        if(data->Token.type == TYPE_INDENT) {

		// vnutro while-u rekurzia statementu
        if ((result = getNextToken(&data->Token)) != 0) return result;
	    if ((result = statement(data)) != SYNTAX_OK) return result;
        while (data->Token.type != TYPE_DEDENT) {
            if (data->Token.type == TYPE_EOL) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
            } else return ERROR_PARSER;
        }
	    if (data->Token.type == TYPE_DEDENT) {
			data->in_while = 0;

/*  *   *   *   *   *   *   pokracovanie statementov    *   *   *   *   *   *   *   */
            generateWHILEjumptostart(docasnyLabel);
            if ((result = getNextToken(&data->Token)) != 0) return result;
            generateWHILEend(docasnyLabel);
            if((result = statement_next(data)) != SYNTAX_OK) return result;
/*  *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   */
            if (data->was_return ==TRUE) {
                if ((result = getNextToken(&data->Token)) != 0) return result;
                data->was_return=FALSE;
            }
	    }
	    else return result; //neprisiel dedent
	    }
	    else return result; //neprisiel indent
	    }
	    else return result; //neprisiel EOL
	    }
	    else return ERROR_PARSER; //neprisiel COLON
    }


/******************************************** R E T U R N *******************************************   
*   *   *   *   *   *   *   *7. <statement> -> KEYWORD_RETURN <expression> *   *   *   *   *   *   *
****************************************************************************************************/
     else if ((data->Token.type == TYPE_KEYWORD) && (data->Token.attribute.keyword == KEYWORD_RETURN)) {
        if (data->in_function < 1) return ERROR_PARSER;//ERROR_SEMANTIC_OTHERS;    ///pokial sa vola mimo funkcie
        data->was_return=TRUE;
        data->leftID = htabSearch(&data->globalT, "%return");
/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */
        if ((result = getNextToken(&data->Token)) != 0) return result;
        if ((result = expression(data)) != 0 ) return result;

/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */

        else if (data->Token.type == TYPE_EOL) {
            data->leftID = htabSearch(&data->globalT, "%return");
            if (data->leftID != NULL) {
                GENERATE(genFunctionReturn, data->leftID->type);
            } else {
                addCode("DEFVAR LF@retval");
                addCode("\n");
                addCode("MOVE GF@$result GF@%return");
                addCode("\n");
            }
            GENERATE(generateReturn, data->currentID->identifier);
            return result = SYNTAX_OK;
        }
        else return ERROR_PARSER;
    }


/*******   DEKLARACIA PREMENNYCH || VOLANIE FUNKCIE || PRIRADOVANIE HODNOTY DO PREMENNEJ   **********
*  *    tu moze nastat jednak definicia a jednak len priradenie hodnoty dolezite hlavne pri WHILE   *
*  *  * 8.  <statement> -> TYPE_IDENTIFIER TYPE_ASSIGN_VALUE <expression> TYPE_EOL <statement_next  *
*  *   *   *    9.	<statement> -> TYPE_IDENTIFIER(<params>) <statement_next>   *   *   *   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_IDENTIFIER) {
        static int result;

        if ((data->leftID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL) {
            //kvoli pripadu ci nahodou nenastava volanie funkcie ktora neexistuje
            if ((result = checkTokenType(&data->Token, TYPE_ASSIGN_VALUE)) == 0) {

                //ak nie je globalne pridam ju
                if (data->in_function == 1 ) {

                    if ((data->leftID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL) {
                        ///pridam premennu lokalne vo funkci
                        if ((result = addToHash(data, true, LEFT)) != 0) return ERROR_INTERN;
                        char *frame= "LF";
                        GENERATE(declareVar,frame,data->Token.attribute.string->str);
                    }
                    //else -> nerobim nic - nepridavam do tabulky
                }
                else {
                    //pridavam premennu globalne
                    if ((result = addToHash(data, false, LEFT)) != 0) return ERROR_INTERN;
                    char *frame= "GF";
                    GENERATE(declareVar,frame,data->Token.attribute.string->str);
                }

                if (data->Token.type == TYPE_ASSIGN_VALUE) {
/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */
                    if ((result = getNextToken(&data->Token)) != 0) return result;
                    if (data->Token.type == TYPE_KEYWORD && (   data->Token.attribute.keyword == KEYWORD_INPUTI ||
                                                                data->Token.attribute.keyword == KEYWORD_INPUTS ||
                                                                data->Token.attribute.keyword == KEYWORD_INPUTF ||
                                                                data->Token.attribute.keyword == KEYWORD_SUBSTR ||
                                                                data->Token.attribute.keyword == KEYWORD_LEN    ||
                                                                data->Token.attribute.keyword == KEYWORD_CHR    ||
                                                                data->Token.attribute.keyword == KEYWORD_ORD))
                    {
                        return result = statement(data);
                    }
                    else if ((result = expression(data)) != 0 ) return result;
/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */

                    if (data->Token.type == TYPE_EOL) {
                        return result = statement_next(data);
                    } else if (data->Token.type == TYPE_EOF) return SYNTAX_OK;
                    else return ERROR_PARSER;
                }
                else return ERROR_PARSER;
                ///volam nedefinovanu funkciu
                } else if (data->Token.type == TYPE_LEFT_PAR) {
                if (data->in_declaration == 1) {
                    if ((result = addToHash(data, false, LEFT)) != 0) return ERROR_INTERN;
                    else {
                        data->leftID->isDefined = false;
                        data->leftID->isGlobal = false; ///explicitne to musim prestavit
                        data->paramIndex = 0;
                        if ((result = params(data)) != 0) return result;
                        if ((result = checkTokenType(&data->Token, TYPE_EOL)) == 0) {
                            data->leftID->previouslyCalled = TRUE;
                            generateCALL(data->leftID->identifier);
                            return result = statement_next(data);
                        }
                        else return result;
                    }
                }
                return result = ERROR_PROGRAM_SEMANTIC; //volanie funkcie ktora neexistuje
            } else return ERROR_PARSER;
        }
        else if (data->leftID->isGlobal == true) {

            if ((result = checkTokenType(&data->Token, TYPE_ASSIGN_VALUE)) == 0) {

                if (data->in_function == 1 ) {
                    if ((data->leftID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL) {
                        ///pridam premennu lokalne vo funkci
                        if ((result = addToHash(data, true, LEFT)) != 0) return ERROR_INTERN;

                            char *frame= "LF";
                            GENERATE(declareVar,frame,data->Token.attribute.string->str);
                    }
                    //else -> nerobim nic - nepridavam do tabulky
                }

/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */
                if ((result = getNextToken(&data->Token)) != 0) return result;
                if (data->Token.type == TYPE_KEYWORD && (data->Token.attribute.keyword == KEYWORD_INPUTI ||
                    data->Token.attribute.keyword == KEYWORD_INPUTS || data->Token.attribute.keyword == KEYWORD_INPUTF ||
                    data->Token.attribute.keyword == KEYWORD_SUBSTR || data->Token.attribute.keyword == KEYWORD_LEN ||
                    data->Token.attribute.keyword == KEYWORD_CHR || data->Token.attribute.keyword == KEYWORD_ORD )) {
                    return result = statement(data);
                }
                else if ((result = expression(data)) != 0 ) return result;
/*  *   *   *   *   *   *   *   *   posielam expression do Expr.c   *   *   *   *   *   *   *   *   */
                if (data->Token.type == TYPE_EOL) {
                    return result = statement_next(data);

                } else if (data->Token.type == TYPE_EOF) return SYNTAX_OK;
                else return ERROR_PARSER;
            } else return result = ERROR_PARSER;
        }
        /*  *   *   *   *   *   *   VOLANIE FUNKCIE    *   *   *   *   *   *   */
        else if (data->leftID->isDefined == true) {

                if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) == 0) {

                    if (data->leftID->paramCount !=0) {
                        data->paramIndex = 1;
                        while (data->paramIndex <= data->leftID->paramCount) {
                            ++(data->paramIndex);
                            //ak mi pride hned prava zatvorka tak je zly pocet parametrov
                            if ((result = checkTokenType(&data->Token, TYPE_IDENTIFIER)) != 0 &&
                            (data->Token.type != TYPE_INT && data->Token.type != TYPE_STRING && data->Token.type != TYPE_FLOAT)) {
                                if(data->Token.type == TYPE_RIGHT_PAR) return ERROR_WRONG_NUMBER_OF_PARAMS;
                                else return result;
                            }
                            if (data->Token.type == TYPE_IDENTIFIER) {
                                if (((data->rightID = htabSearch(&data->localT, data->Token.attribute.string->str)) ==
                                     NULL))
                                    if (((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL))
                                        return ERROR_PROGRAM_SEMANTIC;
                            }
                            if ((result = checkTokenType(&data->Token, TYPE_COMMA)) != 0) {
                                if (data->Token.type == TYPE_RIGHT_PAR)
                                    result = 0;
                                else return result;
                            }
                        }
                        if (data->Token.type == TYPE_RIGHT_PAR) {
                            if ((result = checkTokenType(&data->Token, TYPE_EOL)) == 0) {

                                generateCALL(data->leftID->identifier);
                                return result = statement_next(data);

                            } else if (result == 2 && data->Token.type == TYPE_EOF ) {
                                generateCALL(data->leftID->identifier);
                                return SYNTAX_OK;
                            }
                            else return result;
                        } else if ((result = checkTokenType(&data->Token, TYPE_IDENTIFIER)) == 0 || data->Token.type == TYPE_INT ||
                                   data->Token.type == TYPE_STRING || data->Token.type == TYPE_FLOAT)
                            return result = ERROR_WRONG_NUMBER_OF_PARAMS;
                        else return result = ERROR_PARSER;
                    }
                    else {
                        if ((result = getNextToken(&data->Token)) != 0) return result;
                        else if (data->Token.type == TYPE_RIGHT_PAR) {
                            if ((result = checkTokenType(&data->Token, TYPE_EOL)) == 0) {

                                generateCALL(data->leftID->identifier);
                                return result = statement_next(data);

                            } else if (result == 2 && data->Token.type == TYPE_EOF) {
                                generateCALL(data->leftID->identifier);
                                return SYNTAX_OK;
                            }
                            else return result;
                        } else if (data->Token.type == TYPE_IDENTIFIER || data->Token.type == TYPE_INT ||
                        data->Token.type == TYPE_STRING || data->Token.type == TYPE_FLOAT) return result = ERROR_WRONG_NUMBER_OF_PARAMS;
                        else return result = ERROR_PARSER;
                    }
                }
                //ked s funkciou narabam ako s premennou e.g. foo = 2;
                else if (result == 2 && data->Token.type == TYPE_ASSIGN_VALUE) return ERROR_PROGRAM_SEMANTIC;
                else return result; ///neprisla mi zatvorka
        }
        else return ERROR_PROGRAM_SEMANTIC; //bol to identifier ale nic z tohto tu
    }


/******************************************** P R I N T *******************************************
*   *   *   *   *   *11. <statement> -> KEYWORD_PRINT(<expression>) <statement_next>  *   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_PRINT) {
        static int result;
        data->rightID = htabSearch(&data->globalT, "print");
        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) != 0) return result;

        while (data->Token.type != TYPE_RIGHT_PAR) {
            GENERATE(createFrameForParams);
            if ((result = getNextToken(&data->Token)) != 0) return result;
            if (data->Token.type == TYPE_IDENTIFIER) {
                if (data->in_function == 1) {
                    if ((data->currentID = htabSearch(&data->localT, data->Token.attribute.string->str)) ==NULL)
                        if ((data->currentID = htabSearch(&data->globalT, data->Token.attribute.string->str)) ==NULL)
                            return result = ERROR_PROGRAM_SEMANTIC;
                }
                else {
                    if ((data->currentID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL)
                        return result = ERROR_PROGRAM_SEMANTIC;
                }
            }
            if (data->Token.type == TYPE_RIGHT_PAR) break;

            GENERATE(passParamsToFunction,data->Token, 1, data);
                    generateCALL("print");
            if ((result = getNextToken(&data->Token)) != 0) return result;
            if (data->Token.type == TYPE_RIGHT_PAR || data->Token.type == TYPE_COMMA) {
                if (data->Token.type == TYPE_COMMA) {
                    GENERATE(addCode, "WRITE ");
                    GENERATE(addCode, "string@\\032");
                    GENERATE(addCode, "\n");
                }
            }

            else return ERROR_PARSER;
        }
        GENERATE(addCode,"WRITE ");
        GENERATE(addCode,"string@");
        GENERATE(addCode,"\\010 ");
        GENERATE(addCode, "\n");

        if (data->Token.type == TYPE_RIGHT_PAR) {
            if ((result = getNextToken(&data->Token)) != 0) return result;
            return result = statement_next(data);
        }
        else return ERROR_PARSER;
    }


/******************************************** P A S S ***********************************************
*   *   *   *   *   *12. 	<statement> -> KEYWORD_PASS <statement_next>*   *   *   *   *   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_PASS) {
        static int result;
        if ((result = getNextToken(&data->Token)) != 0) return result;
        return result = statement_next(data);
    }


/******************************************** I N P U T S  *******************************************
*   *   *   *   *   *13. 	<statement> -> KEYWORD_INPUTS() <statement_next>    *   *   *   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_INPUTS) {
        static int result;
        data->rightID = htabSearch(&data->globalT, "inputs");
        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) != 0) return result;
        if ((result = checkTokenType(&data->Token, TYPE_RIGHT_PAR)) != 0) return result;
        if (data->leftID != NULL) {
            data->leftID->type = DTYPE_STRING;
         //   data->leftID = NULL;
        }
        GENERATE(createFrameForParams);
        generateCALL("inputs");
        if(data->leftID !=NULL) {               ///toto potrebuje check
            char *frame;
            if (data->in_function == 1) {
                frame = "LF";
                GENERATE(genFunctionRetValue, data->Token.attribute.string->str, frame);
            } else {
                frame = "GF";
                GENERATE(genFunctionRetValue, data->Token.attribute.string->str, frame);
            }
        }
        if ((result = getNextToken(&data->Token)) != 0) return result;
        return result = statement_next(data);
    }


/******************************************** I N P U T I  *******************************************
*   *   *   *   *   *14. 	<statement> -> KEYWORD_INPUTI() <statement_next>    *   *   *   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_INPUTI) {
        static int result;
        data->rightID = htabSearch(&data->globalT, "inputi");
        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) != 0) return result;
        if ((result = checkTokenType(&data->Token, TYPE_RIGHT_PAR)) != 0) return result;
        if (data->leftID != NULL) {
            data->leftID->type = DTYPE_INT;
            //data->leftID = NULL;
        }
        GENERATE(createFrameForParams);
        generateCALL("inputi");
        if(data->leftID !=NULL) {               ///toto potrebuje check
            char *frame;
            if (data->in_function == 1) {
                frame = "LF";
                GENERATE(genFunctionRetValue, data->Token.attribute.string->str, frame);
            } else {
                frame = "GF";
                GENERATE(genFunctionRetValue, data->Token.attribute.string->str, frame);
            }
        }
        if ((result = getNextToken(&data->Token)) != 0) return result;
        return result = statement_next(data);
    }


/******************************************** I N P U T F  *******************************************
*   *   *   *   *   *15. 	<statement> -> KEYWORD_INPUTF() <statement_next>    *   *   *   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_INPUTF) {
        static int result;
        data->rightID = htabSearch(&data->globalT, "inputf");
        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) != 0) return result;
        if ((result = checkTokenType(&data->Token, TYPE_RIGHT_PAR)) != 0) return result;
        if (data->leftID != NULL) {
            data->leftID->type = DTYPE_DOUBLE;
           // data->leftID = NULL;
        }
        GENERATE(createFrameForParams);
        generateCALL("inputf");
        if(data->leftID !=NULL) {               ///toto potrebuje check
            char *frame;
            if (data->in_function == 1) {
                frame = "LF";
                GENERATE(genFunctionRetValue, data->Token.attribute.string->str, frame);
            } else {
                frame = "GF";
                GENERATE(genFunctionRetValue, data->Token.attribute.string->str, frame);
            }
        }
        if ((result = getNextToken(&data->Token)) != 0) return result;
        return result = statement_next(data);
    }


/*********************************************** L E N  *********************************************
*   *   *   *   *   *16. 	<statement> -> KEYWORD_LEN(TYPE_STRING) <statement_next>    *   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_LEN) {
        static int result;
        data->rightID = htabSearch(&data->globalT, "len");
        GENERATE(createFrameForParams);

        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) != 0) return result;
            if ((result = getNextToken(&data->Token)) != 0) return result;


        if (data->Token.type == TYPE_IDENTIFIER) {
            if (((data->rightID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL))
            if (((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL))
                    return ERROR_PROGRAM_SEMANTIC;
            GENERATE(passParamsToFunction,data->Token, 1, data);
        }
        else if (data->Token.type == TYPE_STRING) {GENERATE(passParamsToFunction,data->Token, 1,data);}
        else if (data->Token.attribute.keyword == KEYWORD_NONE && data->Token.type == TYPE_KEYWORD){
            GENERATE(passParamsToFunction,data->Token, 1,data);
        }
        else return ERROR_PARSER;

        if (data->leftID != NULL) {
            data->leftID->type = DTYPE_INT;
        }
        generateCALL("len");
        if (data->leftID != NULL) {
            char *frame;
            if (data->in_function == 1) {
                frame = "LF";
                GENERATE(genFunctionRetValue, data->leftID->identifier, frame);
            } else {
                frame = "GF";
                GENERATE(genFunctionRetValue, data->leftID->identifier, frame);
            }
        }
        if ((result = checkTokenType(&data->Token, TYPE_RIGHT_PAR)) != 0) return result;
            if ((result = getNextToken(&data->Token)) != 0) return result;
            if (( result = statement_next(data)) != 0) return result;
    }


/********************************************* S U B S T R ******************************************
17. <statement> -> KEYWORD_SUBSTR(TYPE_STRING TYPE_COMMA TYPE_INT TYPE_COMMA TYPE_INT) <statement_next>
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_SUBSTR) {
        static int result;
        data->rightID = htabSearch(&data->globalT, "substr");
        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) != 0) return result;
        GENERATE(createFrameForParams);
            int i = 0;
            while (i < 3) {
                switch (i) {
                    case 0:
                    {
                        if ((result = getNextToken(&data->Token)) != 0)
                            return result;
                        if (data->Token.type == TYPE_STRING) {
                            i++;
                            GENERATE(passParamsToFunction,data->Token, 1,data);
                            if ((result = checkTokenType(&data->Token, TYPE_COMMA)) != 0) return result;
                            break;
                        } else if (data->Token.type == TYPE_IDENTIFIER) {
                            if (((data->rightID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL))
                                if (((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) ==
                                 NULL))
                                    return ERROR_PROGRAM_SEMANTIC;
                            i++;
                            GENERATE(passParamsToFunction,data->Token, 1,data);
                            if ((result = checkTokenType(&data->Token, TYPE_COMMA)) != 0) return result;
                            break;
                        } else if (data->Token.type != TYPE_STRING) return ERROR_ARTIHMETIC;
                        else return ERROR_PARSER;
                    }
                    case 1:
                    {
                        if ((result = getNextToken(&data->Token)) != 0)
                            return result;
                        if (data->Token.type == TYPE_INT) {
                            i++;
                            GENERATE(passParamsToFunction,data->Token, 2,data);
                            if ((result = checkTokenType(&data->Token, TYPE_COMMA)) != 0) return result;
                            break;
                        } else if (data->Token.type == TYPE_IDENTIFIER) {
                            if (((data->rightID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL))
                                if (((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) ==
                                 NULL))

                                    return ERROR_PROGRAM_SEMANTIC;
                            if (data->rightID->type != DTYPE_INT) return ERROR_ARTIHMETIC;
                            i++;
                            GENERATE(passParamsToFunction,data->Token, 2,data);
                            if ((result = checkTokenType(&data->Token, TYPE_COMMA)) != 0) return result;
                            break;
                        }  else if (data->Token.type != TYPE_INT) return ERROR_ARTIHMETIC;
                        else return ERROR_PARSER;
                    }
                    case 2:
                    {
                        if ((result = getNextToken(&data->Token)) != 0)
                            return result;
                        if (data->Token.type == TYPE_INT) {
                            i++;
                            GENERATE(passParamsToFunction,data->Token, 3,data);
                            break;
                        } else if (data->Token.type == TYPE_IDENTIFIER) {
                            if (((data->rightID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL))
                                if (((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL))
                                    return ERROR_PROGRAM_SEMANTIC;
                            if (data->rightID->type != DTYPE_INT) return ERROR_ARTIHMETIC;
                            i++;
                            GENERATE(passParamsToFunction,data->Token, 3,data);
                            break;
                        }  else if (data->Token.type != TYPE_INT) return ERROR_ARTIHMETIC;
                        else return ERROR_PARSER;
                    }
                    default:
                        return ERROR_PARSER;
                }
            }
        if (data->leftID != NULL) {
            data->leftID->type = DTYPE_STRING;
        }
        generateCALL("substr");
        if (data->leftID != NULL) {
            char *frame;
            if (data->in_function == 1) {
                frame = "LF";
                GENERATE(genFunctionRetValue, data->leftID->identifier, frame);
            } else {
                frame = "GF";
                GENERATE(genFunctionRetValue, data->leftID->identifier, frame);
            }
        }
        if ((result = getNextToken(&data->Token)) != 0) return result;
        if (data->Token.type == TYPE_RIGHT_PAR) {
            if ((result = getNextToken(&data->Token)) != 0) return result;
            if ((result = statement_next(data)) != 0) return result;
        }
        else {return ERROR_PARSER;}
    }


/*********************************************** C H R  *********************************************
*   *   18. <statement> -> KEYWORD_CHR(TYPE_STRING TYPE_COMMA TYPE_INT) <statement_next>   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_CHR) {
        static int result;
        data->rightID = htabSearch(&data->globalT, "chr");
        GENERATE(createFrameForParams);

        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) != 0) return result;
        //zavolam expr
        if ((result = getNextToken(&data->Token)) != 0) return result;
        if (data->Token.type == TYPE_IDENTIFIER) {
            if (((data->rightID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL))
                if (((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL))
                    return ERROR_PROGRAM_SEMANTIC;

            GENERATE(passParamsToFunction,data->Token, 1,data);

            if (data->rightID->type != DTYPE_INT) return ERROR_ARTIHMETIC;
        }
        else if (data->Token.type == TYPE_INT) {
            GENERATE(passParamsToFunction,data->Token, 1,data);
        } else if (data->Token.type != TYPE_INT) return ERROR_ARTIHMETIC;
        else return ERROR_PARSER;

        if (data->leftID != NULL) {
            data->leftID->type = DTYPE_STRING;
        }
        generateCALL("chr");
        if (data->leftID != NULL) {
            char *frame;
            if (data->in_function == 1) {
                frame = "LF";
                GENERATE(genFunctionRetValue, data->leftID->identifier, frame);
            } else {
                frame = "GF";
                GENERATE(genFunctionRetValue, data->leftID->identifier, frame);
            }
        }
        if ((result = checkTokenType(&data->Token, TYPE_RIGHT_PAR)) != 0) return result;
        if ((result = getNextToken(&data->Token)) != 0) return result;
        if (( result = statement_next(data)) != 0) return result;
    }


/*********************************************** O R D  *********************************************
*   *   *   *   *   *19. 	<statement> -> KEYWORD_ORD(TYPE_INT) <statement_next>   *   *   *   *   *
****************************************************************************************************/
    else if (data->Token.type == TYPE_KEYWORD && data->Token.attribute.keyword == KEYWORD_ORD) {
        static int result;
        data->rightID = htabSearch(&data->globalT, "ord");
        GENERATE(createFrameForParams);

        if ((result = checkTokenType(&data->Token, TYPE_LEFT_PAR)) != 0) return result;

        int i = 0;
        while (i < 2) {
            switch (i) {
                case 0: {
                    if ((result = getNextToken(&data->Token)) != 0)
                        return result;
                    if (data->Token.type == TYPE_STRING) {
                        i++;

                        GENERATE(passParamsToFunction,data->Token, 1,data);

                        if ((result = checkTokenType(&data->Token, TYPE_COMMA)) != 0) return result;
                        break;
                    } else if (data->Token.type == TYPE_IDENTIFIER) {
                        if (((data->rightID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL))
                            if (((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL))
                                return ERROR_PROGRAM_SEMANTIC;

                        GENERATE(passParamsToFunction,data->Token, 1,data);

                        if (data->rightID->type != DTYPE_STRING) return ERROR_ARTIHMETIC;
                        i++;
                        if ((result = checkTokenType(&data->Token, TYPE_COMMA)) != 0) return result;
                        break;
                    } else if (data->Token.type != TYPE_STRING) return ERROR_ARTIHMETIC;
                    else return ERROR_PARSER;
                }
                case 1: {
                    if ((result = getNextToken(&data->Token)) != 0)
                        return result;
                    if (data->Token.type == TYPE_INT) {
                        i++;

                        GENERATE(passParamsToFunction,data->Token, 2,data);

                        break;
                    } else if (data->Token.type == TYPE_IDENTIFIER) {
                        if (((data->rightID = htabSearch(&data->localT, data->Token.attribute.string->str)) == NULL))
                            if (((data->rightID = htabSearch(&data->globalT, data->Token.attribute.string->str)) == NULL))
                                return ERROR_PROGRAM_SEMANTIC;

                        GENERATE(passParamsToFunction,data->Token, 2,data);

                        if (data->rightID->type != DTYPE_INT) return ERROR_ARTIHMETIC;
                        i++;
                        break;
                    } else if (data->Token.type != TYPE_INT) return ERROR_ARTIHMETIC;
                    else return ERROR_PARSER;
                }
                default:
                    return ERROR_INTERN;
            }
        }
        if (data->leftID != NULL) {
            data->leftID->type = DTYPE_STRING;
        }
        generateCALL("ord");
        if (data->leftID != NULL) {
            char *frame;
            if (data->in_function == 1) {
                frame = "LF";
                GENERATE(genFunctionRetValue, data->leftID->identifier, frame);
            } else {
                frame = "GF";
                GENERATE(genFunctionRetValue, data->leftID->identifier, frame);
            }
        }
        if ((result = checkTokenType(&data->Token, TYPE_RIGHT_PAR)) != 0) return result;
        if ((result = getNextToken(&data->Token)) != 0) return result;
        if (( result = statement_next(data)) != 0) return result;
    }




/*	*   overujem ci nahodou nenastala situacia s komentom alebo je tam len prosté EOL   *   */
    else if ((result = isComment(data)) == 0) return statement_next(data);
/*  *   *  koniec zacyklenia <statement> a <statement_next> -> vrati sa do <prog>   *   *   */
    else if(data->Token.type == TYPE_EOF || data->Token.type == TYPE_DEDENT)
        return result = SYNTAX_OK;
    else if (data->Token.attribute.keyword == KEYWORD_DEF)
        return result = prog(data);
    else return result;

    return result;
}



/******************************** <s t a t e m e n t _ n e x t >    *****************************
*                                                                                               *
*                       10.  <statement_next> -> TYPE_EOL <statement>                           *
*                                                                                               *
*************************************************************************************************/
static int statement_next(ParserData *data) 
{
	static int result;
    if ((result = isComment(data)) == 0 ) {
        //ak bol eol ocakavam dalsi statement (tam sa mi to ked tak vrati spat do progu)
        if ((result = getNextToken(&data->Token)) == 0)
            return result = statement(data);
        else return result;
    }
/**************************** <s t a t e m e n t _ n e x t >  ->  ε  ****************************
*   *   *   *   *   *   *   *    25.  <statement_next>  ->  ε   *   *   *   *   *   *   *   *   *
*************************************************************************************************/
/*  *   *  koniec zacyklenia <statement> a <statement_next> -> vrati sa do <prog>   *   *   */
    else if(data->Token.type == TYPE_EOF || data->Token.type == TYPE_DEDENT)
        return result = SYNTAX_OK;
    else if (data->Token.attribute.keyword == KEYWORD_DEF)
    	return result = prog(data);
    else if (data->Token.type == TYPE_KEYWORD || data->Token.type == TYPE_IDENTIFIER) return result = statement(data);
    else if ((result = checkTokenType(&data->Token, TYPE_EOL)) == 0) return result = statement(data);
    else return result;
}

/*
*	inicializuje vsetky potrebne data
*	v strukture ParserData
*/
int variablesInit(ParserData *data)
{
	htabInit(&data->globalT);
	htabInit(&data->localT);
    data->Token.attribute.keyword = KEYWORD_PASS;
	data->currentID = NULL;
	data->leftID = NULL;
	data->rightID = NULL;

	data->paramIndex = 0;
	data->uniqLabel = 0;

    data->was_return = 0;
	data->in_function = 0;
	data->in_declaration = 0;
	data->in_if = 0;
	data->in_while = 0;

	/*
	*	inicializacia 
	*	vstavanych 
	*	funkcii
	*/
	bool errIntern;
	Data* id;

	// Len(s)
	id = htabAddSymbol(&data->globalT, "len", &errIntern);
	if (errIntern) 
		return ERROR_INTERN;

	id->isDefined = true;
	id->type = DTYPE_INT;
	if (!htabAddParam(id, DTYPE_STRING)) 
		return ERROR_INTERN;

	// SubStr(s =string, i =Integer, n =Integer)
	id = htabAddSymbol(&data->globalT, "substr", &errIntern);
	if (errIntern) 
		return ERROR_INTERN;

	id->isDefined = true;
	id->type = DTYPE_STRING;
	if (!htabAddParam(id, DTYPE_STRING)) 
		return ERROR_INTERN;
	if (!htabAddParam(id, DTYPE_INT)) 
		return ERROR_INTERN;
	if (!htabAddParam(id, DTYPE_INT)) 
		return ERROR_INTERN;

	// Ord(s =String, i =Integer)
	id = htabAddSymbol(&data->globalT, "ord", &errIntern);
	if (errIntern) 
		return ERROR_INTERN;

	id->isDefined = true;
	id->type = DTYPE_INT;
	if (!htabAddParam(id, DTYPE_STRING)) 
		return ERROR_INTERN;
	if (!htabAddParam(id, DTYPE_INT)) 
		return ERROR_INTERN;

	// Print(i) returns nil
	id = htabAddSymbol(&data->globalT, "print", &errIntern);
	if (errIntern) 
		return ERROR_INTERN;
    id->isDefined = true;
    id->type = DTYPE_UNDEFINED;
    ///ma neobmedzeny pocet parametrov

    id = htabAddSymbol(&data->globalT, "inputi", &errIntern);
    if (errIntern)
        return ERROR_INTERN;
    id->isDefined = true;
    id->type = DTYPE_INT;

    id = htabAddSymbol(&data->globalT, "inputf", &errIntern);
    if (errIntern)
        return ERROR_INTERN;
    id->isDefined = true;
    id->type = DTYPE_DOUBLE;

    id = htabAddSymbol(&data->globalT, "inputs", &errIntern);
    if (errIntern)
        return ERROR_INTERN;
    id->isDefined = true;
    id->type = DTYPE_STRING;

    // Chr(i =integer) returns String
    id = htabAddSymbol(&data->globalT, "chr", &errIntern);
    if (errIntern) return ERROR_INTERN;
    id->isDefined = true;
    id->type = DTYPE_STRING;
    if (!htabAddParam(id, DTYPE_INT)) return ERROR_INTERN;

	// Global variable %return for storing result of expression.
	id = htabAddSymbol(&data->globalT, "%return", &errIntern);
	if (errIntern) return ERROR_INTERN;
	id->isDefined = true;
	id->type = DTYPE_UNDEFINED;
	id->isGlobal = true;

	return SYNTAX_OK;
}


/*
*	uvolni pouzivane premenne zo symtable
*/
void variablesFree(ParserData *data)
{
	htabFree(&data->globalT);
	htabFree(&data->localT);
}

/*
 *
 *  MAIN PARSERU
 *
 */
int parse()
{
	int result = 0;

	string parserStr;
	if (stringInit(&parserStr))
		return ERROR_INTERN;
	setString(&parserStr);

	ParserData data;
	if (variablesInit(&data) == ERROR_INTERN)
	{
		stringStrFree(&parserStr);
		return ERROR_INTERN;
	}
    if (!generateCode())
    {
        stringStrFree(&parserStr);
        variablesFree(&data);
        return ERROR_INTERN;
    }
    generateMain();
    data.Token.attribute.keyword = KEYWORD_PASS;
    result = prog(&data);
    generateMainEnd();

	stringStrFree(&parserStr);
	variablesFree(&data);

	return result;
}