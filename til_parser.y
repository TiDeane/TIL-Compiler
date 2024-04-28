%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                   i;          /* integer value */
  std::string          *s;          /* symbol name or string literal */
  cdk::basic_node      *node;       /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
};

%token <i> tINTEGER
%token <s> tIDENTIFIER tSTRING
%token tLOOP tIF tPRINT tPRINTLN tREAD tPROGRAM

%nonassoc tIFX
%nonassoc tELSE

%right '='
%left tGE tLE tEQ tNE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY

%type <node> stmt program
%type <sequence> list exprs
%type <expression> expr
%type <lvalue> lval

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

program : '(' tPROGRAM list ')' { compiler->ast(new til::function_node(LINE,
                                                new til::block_node(LINE,
                                                new cdk::sequence_node(LINE), $3))); }
        ;

list : stmt      { $$ = new cdk::sequence_node(LINE, $1); }
     | list stmt { $$ = new cdk::sequence_node(LINE, $2, $1); }
     ;

stmt : expr ';'                         { $$ = new til::evaluation_node(LINE, $1); }
     | tPRINT exprs                     { $$ = new til::print_node(LINE, $2, false); }
     | tPRINTLN exprs                   { $$ = new til::print_node(LINE, $2, true); }
     | tREAD                            { $$ = new til::read_node(LINE); }
     | tLOOP '(' expr stmt ')'          { $$ = new til::loop_node(LINE, $3, $4); }
     | tIF '(' expr ')' stmt %prec tIFX { $$ = new til::if_node(LINE, $3, $5); }
     | tIF '(' expr ')' stmt tELSE stmt { $$ = new til::if_else_node(LINE, $3, $5, $7); }
     | '{' list '}'                     { $$ = $2; }
     ;

expr : tINTEGER              { $$ = new cdk::integer_node(LINE, $1); }
     | tSTRING               { $$ = new cdk::string_node(LINE, $1); }
     | '-' expr %prec tUNARY { $$ = new cdk::unary_minus_node(LINE, $2); }
     | '+' expr %prec tUNARY { $$ = new cdk::unary_plus_node(LINE, $2); }
     | expr '+' expr         { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr         { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr         { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr         { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr         { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr         { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr         { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr         { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr         { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr         { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr         { $$ = new cdk::eq_node(LINE, $1, $3); }
     | '(' expr ')'          { $$ = $2; }
     | lval                  { $$ = new cdk::rvalue_node(LINE, $1); }
     | "set" lval expr       { $$ = new cdk::assignment_node(LINE, $2, $3); }
     ;

exprs   : expr                 { $$ = new cdk::sequence_node(LINE, $1);     }
        | exprs ',' expr       { $$ = new cdk::sequence_node(LINE, $3, $1); }
        ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, $1); }
     ;

%%
