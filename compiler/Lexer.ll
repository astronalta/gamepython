%{
#include "Statement.hpp"
#include "Formal.hpp"
#include "Expression.hpp"
#include "Feature.hpp"
#include "Parser.hpp"
#include "Grammar.hpp"
#include <iostream>

#define YYSTYPE ParseNode
#define YYLTYPE Location
#define YY_EXTRA_TYPE Parser*
#define YY_NO_INPUT
#define YYERROR_VERBOSE
#define YY_INPUT(buf, res, len) { \
	Parser *parser = yyget_extra(yyscanner);\
	parser->input().read(buf, len);\
	res = (parser->input().bad()) ? 0 : parser->input().gcount();\
}
#define YY_USER_ACTION {\
	Parser *parser = yyget_extra(yyscanner);\
	yylloc->first_line = yylineno;\
	yylloc->first_column = parser->column();\
	parser->column(parser->column() + yyleng);\
	yylloc->last_column = parser->column() - 1;\
	yylloc->last_line = yylineno;\
}

using namespace std;

%}

%option reentrant 
%option bison-bridge
%option never-interactive
%option noyywrap
%option nounput
%option yylineno
%option bison-locations
%s END DOC

/* Lexer rules */
%%
import BEGIN(INITIAL); return IMPORT;
case BEGIN(INITIAL); return CASE;
when BEGIN(INITIAL); return WHEN;
public BEGIN(INITIAL); return PUBLIC;
private BEGIN(INITIAL); return PRIVATE;
static BEGIN(INITIAL); return STATIC;
native BEGIN(INITIAL); return NATIVE;
while BEGIN(INITIAL); return WHILE;
else BEGIN(INITIAL); return ELSE;
until BEGIN(INITIAL); return UNTIL;
if BEGIN(INITIAL); return IF;
function BEGIN(INITIAL); return FUNCTION;
for BEGIN(INITIAL); return FOR;
let BEGIN(INITIAL); return LET;
return BEGIN(END); return RETURN;
xor BEGIN(INITIAL); return XOR;
and BEGIN(INITIAL); return AND;
or BEGIN(INITIAL); return OR;
in BEGIN(INITIAL); return IN;
not BEGIN(INITIAL); return NOT;
fork BEGIN(INITIAL); return FORK;
yield BEGIN(INITIAL); return YIELD;
-> BEGIN(INITIAL); return RIGHT_ARROW;
\<\- BEGIN(INITIAL); return LEFT_ARROW;
== BEGIN(INITIAL); return EQUAL; 
!= BEGIN(INITIAL); return NOT_EQUAL; 
\>\= BEGIN(INITIAL); return GREATER_OR_EQUAL;
\<\> BEGIN(INITIAL); return COMPARE;
\<\= BEGIN(INITIAL); return LESS_OR_EQUAL;
\<\< BEGIN(INITIAL); return LEFT_SHIFT;
\>\> BEGIN(INITIAL); return RIGHT_SHIFT;
\^\= BEGIN(INITIAL); return POWER_ASSIGN;
\*\= BEGIN(INITIAL); return MULTIPLY_ASSIGN;
\/\= BEGIN(INITIAL); return DIVIDE_ASSIGN;
\-\= BEGIN(INITIAL); return SUBTRACT_ASSIGN; 
\+\= BEGIN(INITIAL); return ADD_ASSIGN;
\%\= BEGIN(INITIAL); return MODULUS_ASSIGN;
\|\= BEGIN(INITIAL); return BIT_OR_ASSIGN;
xor\= BEGIN(INITIAL); return BIT_XOR_ASSIGN;
\&\= BEGIN(INITIAL); return BIT_AND_ASSIGN;
\+\+ BEGIN(INITIAL); return INCREMENT;
\-\- BEGIN(INITIAL); return DECREMENT;
\:\: BEGIN(INITIAL); return SCOPE;
; BEGIN(INITIAL); return SEMICOLON;
[0-9]+ {
	String* value = yyextra->environment()->name(yytext);
	yylval->expression = new IntegerLiteral(*yylloc, value); 
    BEGIN(END);
	return NUMBER;
}
(true|false) {
    String* value = yyextra->environment()->name(yytext);
    yylval->expression = new BooleanLiteral(*yylloc, value);
    BEGIN(END);
    return BOOLEAN;
}
\"[^"]*\" {
	yytext[strlen(yytext)-1] = 0;
	String* value = yyextra->environment()->name(yytext);
	yylval->expression = new StringLiteral(*yylloc, value);
    BEGIN(END);
	return STRING;
}
\'[^']*\' {
	String* value = yyextra->environment()->name(yytext);
	yylval->expression = new StringLiteral(*yylloc, value); 
    BEGIN(END);
	return STRING;
}
@[a-z][a-z]* {
    yylval->name = yyextra->environment()->name(yytext);
    BEGIN(END); 
    return OPERATOR;
}
[a-z][A-Za-z0-9_]*[?!]? {
	yylval->name = yyextra->environment()->name(yytext);
    BEGIN(END);
	return IDENTIFIER;
}
[A-Z][A-Za-z0-9]* {
	yylval->name = yyextra->environment()->name(yytext);
    BEGIN(END);
	return TYPE;
}

[{([,] BEGIN(DOC); return yytext[0];
[)\]] BEGIN(END); return yytext[0];
[}] BEGIN(INITIAL); return yytext[0];
<END>[\n\r] {
    BEGIN(INITIAL); 
    yyget_extra(yyscanner)->column(1); 
    return SEPARATOR;
}
<INITIAL>[\n\r] {
    yyget_extra(yyscanner)->column(1);
}
<DOC>[\n\r] {
    yyget_extra(yyscanner)->column(1);
}
<DOC>#.* {
    const char* comment = yytext + 1;    
    while (isspace(*comment)) {
        comment++;
    }
    return COMMENT;
}

#.*
[\t ]
. return yytext[0];
%%

void Parser::force_separator() {
    struct yyguts_t *yyg = (struct yyguts_t *)scanner_;
    BEGIN(END);
}

