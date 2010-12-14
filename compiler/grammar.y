%{
#include <apparser.h>
#include <apexpr.h>
#include <aptype.h>
#include <apstmt.h>
#include <apvar.h>
#include <apfunc.h>
#include <apunit.h>
#include <apenv.h>
#include <apimport.h>
#include <apsymtab.h>
#include <apdef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define YYSTYPE apnode_t
#define YYLTYPE aploc_t
#define YY_EXTRA_TYPE apparser_t *
#define YY_NO_INPUT
#define YYERROR_VERBOSE
int yylex(apnode_t *node, aploc_t *loc, void *scanner);
void yyerror(aploc_t *loc, apparser_t *parser, void *scanner, const char *message);

%}

%union { apexpr_t *expr; }
%union { apstmt_t *stmt; }
%union { aptype_t *type; }
%union { apunit_t *unit; }
%union { apvar_t *var; }
%union { apfunc_t *func; }
%union { apimport_t *import; }
%union { apdef_t *def; }
%union { char *string; }
%union { int null; }
%union { int flag; }

%pure_parser
%locations
%parse-param { apparser_t *parser }
%parse-param { void *scanner }
%lex-param { yyscan_t *scanner }

%destructor { apexpr_free($$); $$ = 0; } <expr>
%destructor { apstmt_free($$); $$ = 0; } <stmt>
%destructor { aptype_free($$); $$ = 0; } <type>
%destructor { apunit_free($$); $$ = 0; } <unit>
%destructor { apvar_free($$); $$ = 0; } <var>
%destructor { apfunc_free($$); $$ = 0; } <func>
%destructor { free($$); $$ = 0; } <string>


%left '.' '['
%left TOK_INC TOK_DEC
%left '!' '~'
%left '*' '/' '%'
%left '+' '-'
%left TOK_LSHIFT TOK_RSHIFT
%left '<' '>' TOK_LE TOK_GE
%left TOK_EQUAL TOK_NOTEQUAL
%left '&'
%left '|' '^'
%left TOK_AND
%left TOK_OR
%left '=' TOK_MUL_ASSIGN TOK_DIV_ASSIGN TOK_MOD_ASSIGN 
%left TOK_ADD_ASSIGN TOK_SUB_ASSIGN
%left TOK_BITAND_ASSIGN TOK_BITOR_ASSIGN
%left '?'


/* BISON declarations */
%token <string> TOK_IDENT
%token <expr> TOK_STRING
%token <expr> TOK_NUMBER 
%token TOK_CLASS
%token TOK_INTERFACE
%token TOK_STRUCT
%token TOK_MODULE
%token TOK_IMPORT
%token TOK_DEF
%token TOK_VAR
%token TOK_WHEN
%token TOK_CASE
%token TOK_SEP
%token TOK_INIT
%token TOK_DESTROY
%token <flag> TOK_PUBLIC
%token <flag> TOK_PRIVATE
%token <flag> TOK_STATIC
%token <flag> TOK_NATIVE
%token TOK_WHILE
%token TOK_ELSE
%token TOK_UNTIL
%token TOK_IF
%token TOK_DO
%token TOK_FOR
%token TOK_RETURN
%token TOK_ARROW
%token TOK_LEFT_ARROW
%token TOK_EQUAL
%token TOK_NOTEQUAL
%token TOK_OR
%token TOK_AND
%token TOK_GE
%token TOK_LE
%token TOK_LSHIFT
%token TOK_RSHIFT
%token TOK_MUL_ASSIGN
%token TOK_DIV_ASSIGN
%token TOK_SUB_ASSIGN
%token TOK_ADD_ASSIGN
%token TOK_MOD_ASSIGN
%token TOK_BITOR_ASSIGN
%token TOK_BITAND_ASSIGN
%token TOK_INC
%token TOK_DEC

%type <unit> class
%type <unit> interface
%type <unit> module
%type <unit> struct
%type <var> attribute 
%type <import> import
%type <def> def
%type <func> constructor
%type <func> destructor
%type <func> function
%type <func> prototype
%type <func> native
%type <flag> modifiers
%type <var> parameter_signature
%type <var> parameter_list
%type <type> type
%type <stmt> block
%type <stmt> when_list
%type <stmt> when
%type <stmt> statement_list
%type <stmt> statement
%type <expr> expression
%type <expr> expression_list
%type <expr> storage
%type <expr> assignment


/* The Standard Apollo Grammar, version 2010 */
%%
translation_unit
    : TOK_CLASS TOK_IDENT '{' class '}' {
		if (strcmp($2, $4->name)) {
			yyerror(&@$, parser, scanner, "Class name does not match filename");
		}
		free($2);
    }
    | TOK_INTERFACE TOK_IDENT '{' interface '{' {
		if (strcmp($2, $4->name)) {
			yyerror(&@$, parser, scanner, "Class name does not match filename");
		}
		free($2);
    }
    | TOK_STRUCT TOK_IDENT '{' struct '}' {
		if (strcmp($2, $4->name)) {
			yyerror(&@$, parser, scanner, "Class name does not match filename");
		}
		free($2);
    }
    | TOK_MODULE TOK_IDENT '{' module '}' {
		if (strcmp($2, $4->name)) {
			yyerror(&@$, parser, scanner, "Class name does not match filename");
		}
		free($2);
    }
	| /* empty is an error */ { 
		yyerror(&@$, parser, scanner, "Input file is empty"); 
		YYERROR;
	}
	| error { }
    ;

class
    : import class { $$ = $2; apunit_import($$, $1); }
    | def class { $$ = $2; apunit_def($$, $1); }
    | attribute class { $$ = $2; apunit_var($$, $1); }
    | constructor class { $$ = $2; apunit_ctor($$, $1); }
    | destructor class { $$ = $2; apunit_dtor($$, $1); }
    | function class { $$ = $2; apunit_func($$, $1); }
	| native class { $$ = $2; apunit_func($$, $1); }
	| error class { $$ = $2; }
    | /* empty */ { 
		$$ = parser->unit;
		$$->type = APUNIT_TYPE_CLASS;
	}
    ;

interface
    : import interface { $$ = $2; apunit_import($$, $1); }
    | def interface { $$ = $2; apunit_def($$, $1); }
	| prototype interface { $$ = $2; apunit_func($$, $1); }
	| attribute interface { 
		$$ = $2;
		apvar_free($1);
		yyerror(&@$, parser, scanner, "Instance variable in interface");
		YYERROR;
	}
    | constructor interface {
		$$ = $2;
		apfunc_free($1);
		yyerror(&@$, parser, scanner, "Interfaces cannot have constructors");
		YYERROR;
	}
    | destructor interface { 
		$$ = $2;
		apfunc_free($1);
		yyerror(&@$, parser, scanner, "Interfaces cannot have destructors");
		YYERROR;
	}
	| error interface { $$ = $2; }
    | /* empty */ { 
		$$ = parser->unit;
		$$->type = APUNIT_TYPE_INTERFACE;
	}
    ;

struct
    : import struct { $$ = $2; apunit_import($$, $1); }
    | def struct { $$ = $2; apunit_def($$, $1); }
    | attribute struct { $$ = $2; apunit_var($$, $1); }
    | constructor struct { $$ = $2; apunit_ctor($$, $1); }
    | function struct { $$ = $2; apunit_func($$, $1); }
	| prototype struct { $$ = $2; apunit_func($$, $1); }
	| destructor struct {
		$$ = $2;
		apfunc_free($1);
		yyerror(&@$, parser, scanner, "Structs cannot have destructors");
		YYERROR;
	}
	| error struct { $$ = $2; }
    | /* empty */ { 
		$$ = parser->unit;
		$$->type = APUNIT_TYPE_STRUCT;
	}
    ;

module
    : import module { $$ = $2; apunit_import($$, $1); }
    | def module { $$ = $2; apunit_def($$, $1); }
	| attribute module { $$ = $2; apunit_var($$, $1); }
	| constructor module {
		$$ = $2;
		apfunc_free($1);
		yyerror(&@$, parser, scanner, "Modules cannot have constructors");
		YYERROR;
	}
	| destructor module {
		$$ = $2;
		apfunc_free($1);
		yyerror(&@$, parser, scanner, "Modules cannot have destructors");
		YYERROR;
	}
    | function module { $$ = $2; apunit_func($$, $1); }
	| error module { $$ = $2; }
    | /* empty */ { 
		$$ = parser->unit;
		$$->type = APUNIT_TYPE_MODULE;
	}
    ;
    
import
    : TOK_IMPORT TOK_IDENT TOK_SEP { 
		$$ = apimport_alloc(&@$, $2);
	}
    ;

def
    : TOK_DEF type TOK_IDENT TOK_SEP { 
		$$ = apdef_alloc($2, aptype_object($3)); 
	}
    ;

attribute
    : TOK_VAR TOK_IDENT ':' type '=' expression TOK_SEP {
		// TODO: Set symbol table for class-level
		$$ = apvar_alloc(&@$, $2, $4, $6);
    }
    | TOK_VAR TOK_IDENT ':' type TOK_SEP {
		$$ = apvar_alloc(&@$, $2, $4, 0);
    }
	;

constructor
    : TOK_DEF TOK_INIT parameter_signature block { 
		$$ = apfunc_alloc(&@$, apenv_ident(parser->env, "@init"), $3, 0);
		$$->flags &= APUNIT_FLAG_STATIC;
		$$->block = $4;
	}
    ;

destructor
    : TOK_DEF TOK_DESTROY '(' ')' block { 
		$$ = apfunc_alloc(&@$, apenv_ident(parser->env, "@destroy"), 0, 0); 
		$$->block = $5;
	}
	| TOK_DEF TOK_DESTROY '(' parameter_list ')' block {
		apvar_free($4);
		apstmt_free($6);
		$$ = 0;
		yyerror(&@$, parser, scanner, "Destructors cannot take arguments"); 
		YYERROR;
	}
    ;

function
    : TOK_DEF TOK_IDENT parameter_signature ':' type modifiers block {
		$$ = apfunc_alloc(&@$, $2, $3, $5);
		$$->flags = $6;
		$$->block = $7;
    }
	| TOK_DEF TOK_IDENT parameter_signature modifiers block {
		$$ = apfunc_alloc(&@$, $2, $3, 0);
		$$->flags = $4;
		$$->block = $5;
	}
	;

prototype
	: TOK_DEF TOK_IDENT parameter_signature ':' type modifiers TOK_SEP {
		$$ = apfunc_alloc(&@$, $2, $3, $5);
		$$->flags = $6;
	}
	| TOK_DEF TOK_IDENT parameter_signature modifiers TOK_SEP {
		$$ = apfunc_alloc(&@$, $2, $3, 0);
		$$->flags = $4;
	}
    ;

native
	: TOK_DEF TOK_IDENT parameter_signature ':' type modifiers TOK_NATIVE TOK_SEP {
		$$ = apfunc_alloc(&@$, $2, $3, $5);
		$$->flags = $6|APUNIT_FLAG_NATIVE;
	}
	| TOK_DEF TOK_IDENT parameter_signature modifiers TOK_NATIVE TOK_SEP {
		$$ = apfunc_alloc(&@$, $2, $3, 0);
		$$->flags = $4|APUNIT_FLAG_NATIVE;
	}
    ;
	
parameter_signature
	: '(' parameter_list ')' { $$ = $2; }
	| '(' ')' { $$ = 0; }
	;

parameter_list
	: TOK_IDENT ':' type ',' parameter_list { 
		$$ = apvar_formal(&@1, $1, $3, $5);
	}
	| TOK_IDENT ':' type { 
		$$ = apvar_formal(&@$, $1, $3, 0);
	}
    ;

modifiers
	: TOK_PUBLIC { $$ = APUNIT_FLAG_PUBLIC; }
	| TOK_PRIVATE { $$ = APUNIT_FLAG_PRIVATE; }
	| TOK_STATIC { $$ = APUNIT_FLAG_STATIC; }
	| TOK_PUBLIC TOK_STATIC { $$ = APUNIT_FLAG_PUBLIC|APUNIT_FLAG_STATIC; }
	| TOK_PRIVATE TOK_STATIC { $$ = APUNIT_FLAG_PRIVATE|APUNIT_FLAG_STATIC; }
	| /* empty */ { $$ = 0; }
	;

type
    : TOK_IDENT { $$ = aptype_object($1); }
    ;

block
    : '{' statement_list '}' { 
		$$ = $2; 
	}
	| TOK_ARROW TOK_RETURN expression TOK_SEP { 
        $$ = apstmt_return(&@$, $3); 
    }
	| TOK_ARROW TOK_RETURN TOK_SEP { 
        $$ = apstmt_return(&@$, 0); 
    }
	| TOK_ARROW expression TOK_SEP { 
		$$ = apstmt_expr(&@$, $2); 
    }
    ;

statement_list
    : statement_list statement { 
		$$ = apstmt_append($1, $2); 
	}
	| statement_list error { $$ = $1; }
    | /* empty */ { 
		$$ = apstmt_block(&@$);
	}
    ;


statement
	: TOK_FOR TOK_IDENT ':' type TOK_LEFT_ARROW expression block {
		$$ = apstmt_for(&@$, apvar_alloc(&@1, $2, $4, $6), $7);
	}
	| TOK_UNTIL expression block {
		$$ = apstmt_until(&@$, $2, $3);
	}
	| TOK_WHILE expression block {
		$$ = apstmt_while(&@$, $2, $3);
	}
    | TOK_CASE expression '{' when_list '}' {
        $$ = apstmt_case(&@$, $2, $4);
    }
    | TOK_VAR TOK_IDENT ':' type { 
		$$ = apstmt_decl(&@$, apvar_alloc(&@$, $2, $4, 0)); 
	}
	| TOK_RETURN expression TOK_SEP { 
        $$ = apstmt_return(&@$, $2); 
    }
	| TOK_RETURN TOK_SEP { 
        $$ = apstmt_return(&@$, 0); 
    }
    | TOK_VAR TOK_IDENT ':' type '=' expression TOK_SEP { 
		$$ = apstmt_decl(&@$, apvar_alloc(&@$, $2, $4, $6)); 
	}  
    | expression TOK_SEP { 
		$$ = apstmt_expr(&@$, $1); 
	}
    | assignment TOK_SEP { 
		$$ = apstmt_expr(&@$, $1); 
	}
	| TOK_IF expression block {
		$$ = apstmt_cond(&@$, $2, $3, 0);
	}
    | TOK_IF expression block TOK_ELSE block {
        $$ = apstmt_cond(&@$, $2, $3, $5);
    }
	;

storage
    : expression '.' TOK_IDENT { 
		/* Member variable access */
		$$ = apexpr_member(&@$, $1, $3); 
	}
    | expression '[' expression ']' { 
		$$ = apexpr_index(&@$, $1, $3); 
	}

assignment
    : storage '=' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "="), $1, $3); 
	}
    | storage TOK_MUL_ASSIGN expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "*="), $1, $3); 
	}
    | storage TOK_DIV_ASSIGN expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "/="), $1, $3); 
	}
    | storage TOK_MOD_ASSIGN expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "%="), $1, $3); 
	}
    | storage TOK_SUB_ASSIGN expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "-="), $1, $3); 
	}
    | storage TOK_ADD_ASSIGN expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "+="), $1, $3); 
	}
    | storage TOK_BITAND_ASSIGN expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "&="), $1, $3); 
	}
    | storage TOK_BITOR_ASSIGN expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "|="), $1, $3); 
	}
    ;


expression
	: expression '?' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "?"), $1, $3);
	}
    | expression TOK_OR expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "||"), $1, $3); 
	}
    | expression TOK_AND expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "&&"), $1, $3); 
	}
    | expression '|' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "|"), $1, $3); 
	}
    | expression '^' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "^"), $1, $3); 
	}
    | expression '&' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "&"), $1, $3); 
	}
    | expression TOK_EQUAL expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "=="), $1, $3); 
	}
    | expression TOK_NOTEQUAL expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "!="), $1, $3); 
	}
    | expression '>' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, ">"), $1, $3); 
	}
    | expression '<' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "<"), $1, $3); 
	}
    | expression TOK_GE expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, ">="), $1, $3); 
	}
    | expression TOK_LE expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "<="), $1, $3); 
	}
    | expression TOK_LSHIFT expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "<<"), $1, $3); 
	}
    | expression TOK_RSHIFT expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, ">>"), $1, $3); 
	}
    | expression '+' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "+"), $1, $3); 
	}
    | expression '-' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "-"), $1, $3); 
	}
    | expression '*' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "*"), $1, $3); 
	}
    | expression '/' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "/"), $1, $3); 
	}
    | expression '%' expression { 
		$$ = apexpr_binary(&@$, apenv_ident(parser->env, "%"), $1, $3); 
	}
    | '!' expression { 
        $$ = apexpr_unary(&@$, apenv_ident(parser->env, "!"), $2); 
    }
    | '~' expression { 
        $$ = apexpr_unary(&@$, apenv_ident(parser->env, "~"), $2); 
    }
    | expression TOK_INC { 
		$$ = apexpr_unary(&@$, apenv_ident(parser->env, "++"), $1); 
	}
    | expression TOK_DEC { 
		$$ = apexpr_unary(&@$, apenv_ident(parser->env, "--"), $1); 
	}
    | TOK_IDENT '(' expression_list ')' {
		$$ = apexpr_call(&@$, $1, $3);
    }
	| TOK_IDENT '(' ')' {
		/* Call on an object expression */
		$$ = apexpr_call(&@$, $1, 0);
	}
    | expression '.' TOK_IDENT '(' expression_list ')' {
        $1->next = $5;
        $$ = apexpr_call(&@$, $3, $1);
        
    }
    | expression '.' TOK_IDENT '(' ')' {
        $$ = apexpr_call(&@$, $3, $1);
    }
    | storage { $$ = $1; }
	| '(' expression ')' { $$ = $2; } 
    | TOK_STRING { $$ = $1; }
    | TOK_NUMBER { $$ = $1; }
	| TOK_IDENT { $$ = apexpr_ident(&@$, $1); }
    ;

expression_list
	: expression ',' expression_list { 
		$$ = $1;
		$$->next = $3;
	}
	| expression { $$ = $1; } 
	;


when_list
    : when_list when {
        $$ = apstmt_append($1, $2); 
    }
    | when {
        $$ = apstmt_append(apstmt_block(&@$), $1);
    }
    ;

when
    : TOK_WHEN TOK_IDENT ':' type block {
        $$ = apstmt_when(&@$, apvar_alloc(&@$, $2, $4, 0), $5); 
    }
    ;


