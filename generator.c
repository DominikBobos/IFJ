/*
 *	Predmet : Formální jazyky a překladače
 *	Projekt : Implementace překladače imperativního jazyka IFJ19
 *	Súbor	: generator.c
 *				-modul v ktorom sa generuju casti IFJcode19
 *	Tím		: číslo 10 / varianta II
 *	Autori	: Dominik Boboš (xbobos00)
 *			  Peter Hudeček (xhudec34)
 *			  Timotej Kováčik (xkovac49)
 *			  Dávid Oravec (xorave05)
 *
 */

#include "generator.h"
#include "scanner.h"
#include "str.h"
#include <ctype.h>

#define MAX 50 ///maximum pre convertovanie stringu

#define ADDCODEINT(_code)													\
	do {																	\
		char str[50];												        \
		sprintf(str, "%d", _code);											\
		addCode(str);														\
	} while (0)

string genCode;
bool addInstr(const char *_instr,...) {
    if (stringAddConst(&genCode, (_instr))){
        return false;
    }
    if (stringAddConst(&genCode, " \n")){
        return false;
    }
    return true;
}

bool addCode(const char *_code,...){
    return (stringAddConst(&genCode, (_code))) ? false : true;
}

bool generateHead(){
    addInstr(".IFJcode19");

    addInstr("DEFVAR GF@$temp1");
    addInstr("DEFVAR GF@$temp2");
    addInstr("DEFVAR GF@$temp3");
    addInstr("DEFVAR GF@$result");
    addInstr("MOVE GF@$result nil@nil");
    addInstr("DEFVAR GF@%return");

    addInstr("JUMP $$main");

    return true;
}



bool generateBuiltIn(){
    addInstr(PRINT);
    addInstr(INPUTI);
    addInstr(INPUTS);
    addInstr(INPUTF);
    addInstr(LEN);
    addInstr(SUBSTR);
    addInstr(ORD);
    addInstr(CHR);

    return true;
}

bool generateCode(){

    stringInit(&genCode);
    generateHead();
    generateBuiltIn();

    return true;
}

bool writeCodeToStdout(){
    fprintf(stdout,"%s",genCode.str);
    stringFree(&genCode);

    return true;
}

bool generateMain(){
    addInstr("LABEL $$main");
    addInstr("CREATEFRAME");
    addInstr("PUSHFRAME");

    return true;
}

bool generateMainEnd(){
    addInstr("POPFRAME");
    addInstr("CLEARS");

    return true;
}

bool generateValue(token *token){
    char tempStr[30];
    unsigned char c;

    string help;
    if (stringInit(&help)) return false;

    if (token->type == TYPE_INT){
        sprintf(tempStr,"%d",token->attribute.int_value);
        addCode("int@");
        addCode(tempStr);
        addCode("\n");
        return true;
    } else if (token->type == TYPE_FLOAT){
        sprintf(tempStr,"%a",token->attribute.decimal_value);
        addCode("float@");
        addCode(tempStr);
        addCode("\n");
        return true;
    } else if (token->type == TYPE_STRING){
        for (int i = 0; (c = (unsigned char) (token->attribute.string->str[i])) != '\0'; i++)
        {
            if (c == '#' || c == '\\' || c <= 32 || !isprint(c))
            {
                stringAddChar(&help, '\\');
                sprintf(tempStr, "%03d", c);
                stringAddConst(&help, tempStr);
            }
            else
            {
                stringAddChar(&help,(char) c);
            }
        }
        addCode("string@"); addCode(help.str);
        addCode("\n");
        return true;
    } else if (token->type == TYPE_IDENTIFIER){
        addCode("LF@");
        addCode(token->attribute.string->str);
        return true;
    }
    return false;
}

bool pushVar(token *token,ParserData *data){
    //nechce s ami prepisovat funkcia tak tu hadzem tento nezmysel
    int bs = token->attribute.int_value;
    token->attribute.int_value = bs;
    addCode("PUSHS ");
    if (data->Token.attribute.keyword == KEYWORD_NONE){
        addCode("nil@nil");
    } else if (!generateTerm(data->Token,data)) return false;
    addCode("\n");
    return true;
}

bool pushValue(token *token){
    addCode("PUSHS ");
    generateValue(token);

    return true;
}

bool declareVar(char *frame, char *ID)
{
    addCode("DEFVAR ");
    addCode(frame);
    addCode("@");
    addCode(ID);
    addCode("\n");

    return true;
}

bool genFunctionHead(char *ID)
{

    addCode("LABEL $");
    addCode(ID);
    addCode("\n");
    addInstr("CREATEFRAME");
    addInstr("PUSHFRAME");
    addCode("\n");

    return true;
}

bool declareFunctionParam(char *ID, int index)
{
    addCode("DEFVAR LF@");
    addCode(ID);
    addCode("\n");
    addCode("MOVE LF@");
    addCode(ID);
    addCode(" LF@%");
    ADDCODEINT(index);
    addCode("\n");

    return true;
}

bool genFunctionEnd(char *ID)
{

    addCode("LABEL $"); 
    addCode(ID); 
    addCode("%return\n");
    addInstr("POPFRAME");
    addInstr("RETURN");

    return true;
}

bool op1ToDouble(){
    addInstr("INT2FLOATS");
    return true;
}

bool op3ToDouble(){
    addInstr("POPS GF@$temp1");
    addInstr("FLOAT2INTS");
    addInstr("PUSHS GF@$temp2");

    return true;
}

bool generateConcatenation(){
    addInstr("POPS GF@$temp2");
    addInstr("POPS GF@$temp1");
    addInstr("CONCAT GF@$result GF@$temp1 GF@$temp2");
    addInstr("PUSHS GF@$result");

    return true;
}

bool generateAdds(){
    addInstr("ADDS");
    return true;
}

bool generateSubs(){
    addInstr("SUBS");
    return true;
}

bool generateMuls(){
    addInstr("MULS");
    return true;
}

bool generateDivs(){
    addInstr("DIVS");
    return true;
}

bool generateIDivs(){
    addInstr("IDIVS");
    return true;
}

bool generateLess(){
    addInstr("LTS");
    return true;
}

bool generateMore(){
    addInstr("GTS");

    return true;
}

bool generateEquals(){
    addInstr("EQS");

    return true;
}

bool generateNotEqual(){
    addInstr("EQS");
    addInstr("NOTS");

    return true;
}

bool generateMoreEqual(){
    addInstr("POPS GF@$temp1");
    addInstr("POPS GF@$temp2");
    addInstr("PUSHS GF@$temp2");
    addInstr("PUSHS GF@$temp1");
    addInstr("GTS");
    addInstr("PUSHS GF@$temp2");
    addInstr("PUSHS GF@$temp1");
    addInstr("EQS");
    addInstr("ORS");

    return true;
}

bool generateLessEqual(){
    addInstr("POPS GF@$temp1");
    addInstr("POPS GF@$temp2");
    addInstr("PUSHS GF@$temp2");
    addInstr("PUSHS GF@$temp1");
    addInstr("LTS");
    addInstr("PUSHS GF@$temp2");
    addInstr("PUSHS GF@$temp1");
    addInstr("EQS");
    addInstr("ORS");

    return true;
}

bool generateSaveExprResult(char *id, char *frame){


    addCode("POPS ");
    addCode(frame);
    addCode("@");
    addCode(id);
    addCode("\n");
    return true;
}

bool generateReturn(char *id)
{
    addInstr("MOVE LF@retval GF@$result");
    addCode("JUMP $");
    addCode(id);
    addCode("%return\n");
    return true;
}

static bool genVarValue(DataType type)
{
    switch (type)
    {
        case DTYPE_INT:
            addCode("int@0");
            break;

        case DTYPE_DOUBLE:
            addCode("float@0.0");
            break;

        case DTYPE_STRING:
            addCode("string@");
            break;

        case DTYPE_BOOL:
            addCode("bool@false");
            break;

        case DTYPE_UNDEFINED:
            addCode("nil@nil");
            break;

        default:
            return false;
    }
    return true;
}

bool genFunctionReturn(DataType type)
{
    addInstr("DEFVAR LF@%retval");

    addCode("MOVE LF@%retval ");
    if (!genVarValue(type)) return false;
    addCode("\n");

    return true;
}

bool createFrameForParams()
{
    addInstr("CREATEFRAME");

    return true;
}

bool passParamsToFunction(token Token, int i, ParserData *data)
{
    addCode("DEFVAR TF@%"); ADDCODEINT(i); addCode("\n");

    addCode("MOVE TF@%"); ADDCODEINT(i); addCode(" ");
    if (data->Token.attribute.keyword == KEYWORD_NONE){
        addCode("nil@nil");
    } else if (!generateTerm(Token, data)) return false;
    addCode("\n");

    return true;
}

bool convertPassedParams(DataType wrong, DataType converted, int index)
{
    if (wrong == DTYPE_DOUBLE && converted == DTYPE_INT)
    {
        addCode("FLOAT2R2EINT TF@%");
        ADDCODEINT(index);
        addCode(" TF@%");
        ADDCODEINT(index);
        addCode("\n");
    }
    else if (wrong == DTYPE_INT && converted == DTYPE_DOUBLE)
    {
        addCode("INT2FLOAT TF@%");
        ADDCODEINT(index);
        addCode(" TF@%");
        ADDCODEINT(index);
        addCode("\n");
    }

    return true;
}


bool generateTerm(token Token, ParserData *data) {
    char temp[MAX];
    unsigned char c;

    string help;
    if (stringInit(&help)) return false;

    switch (Token.type) {
        case TYPE_INT:
            sprintf(temp, "%d", Token.attribute.int_value);
            addCode("int@");
            addCode(temp);
            break;

        case TYPE_FLOAT:
            sprintf(temp, "%a", Token.attribute.decimal_value);
            addCode("float@");
            addCode(temp);
            break;

        case TYPE_STRING:
            for (int i = 0; (c = (unsigned char) (Token.attribute.string->str)[i]) != '\0'; i++) {
                if (c == '#' || c == '\\' || c <= 32 || !isprint(c)) {
                    stringAddChar(&help, '\\');
                    sprintf(temp, "%03d", c);
                    stringAddConst(&help, temp);
                } else {
                    stringAddChar(&help, c);
                }
            }
            addCode("string@");
            addCode(help.str);
            break;

        case TYPE_IDENTIFIER:
            if (data->leftID != NULL) {
                if (data->leftID->isGlobal) {
                    addCode("GF@");
                    addCode(Token.attribute.string->str);
                } else {
                    addCode("LF@");
                    addCode(Token.attribute.string->str);
                }
            } else if (data->currentID != NULL) {
                if (data->currentID->isGlobal) {
                    addCode("GF@");
                    addCode(Token.attribute.string->str);
                } else {
                    addCode("LF@");
                    addCode(Token.attribute.string->str);
                }
            } else if (data->rightID != NULL) {
                if (data->rightID->isGlobal) {
                    addCode("GF@");
                    addCode(Token.attribute.string->str);
                } else {
                    addCode("LF@");
                    addCode(Token.attribute.string->str);
                }
            } else return false;
            break;
        default:
            stringStrFree(&help);
            return false;
    }

    stringStrFree(&help);

    return true;
}



bool genFunctionRetValue(char *leftID, char *frame)//DataType leftT, DataType retT)
{
    /*if (leftT == DTYPE_INT && retT == DTYPE_DOUBLE)
    {
        addInstr("FLOAT2R2EINT TF@%return TF@%return");
    }
    else if (leftT == DTYPE_DOUBLE && retT == DTYPE_INT)
    {
        addInstr("INT2FLOAT TF@%return TF@%return");
    }*/

    addCode("MOVE ");
    addCode(frame);
    addCode("@");
    addCode(leftID);
    addCode(" TF@%return\n");

    return true;
}



bool generateIfStart(int index)
{

    addInstr("# If Else");
    addCode("JUMPIFEQ $");
    addCode("Else");
    addCode("%");
    ADDCODEINT(index);
    addCode(" GF@%return bool@false\n");

    return true;
}
bool generateIfPre(int index)
{

        addCode("JUMP $EndIf%");
        ADDCODEINT(index);
        addCode("\n");
        return true;
}
bool generateIf(int index)
{
    addInstr("# Else");
    addCode("LABEL $Else%");
    ADDCODEINT(index);
    addCode("\n");


    return true;
}

bool generateIfEnd(int index)
{

    addInstr("# End If");
    addCode("LABEL $EndIf%");
    ADDCODEINT(index);
    addCode("\n");

    return true;
}

bool generateCALL(char *id) {
    addCode("CALL $");
    addCode(id);
    addCode("\n");
    return true;
}
bool generateWHILElabel(int label){
    addCode("LABEL $while");
    ADDCODEINT(label);
    addCode("\n");
    return true;
}
bool generateWHILEjumptostart(int label){
    addCode("JUMP $while");
    ADDCODEINT(label);
    addCode("\n");
    return true;
}
bool generateWHILEcondition(int label){
    addCode("JUMPIFEQ $end");
    ADDCODEINT(label);
    addCode(" GF@%return bool@false\n");
    return true;
}
bool generateWHILEend(int label){
    addCode("LABEL $end");
    ADDCODEINT(label);
    addCode("\n");
    return true;
}
