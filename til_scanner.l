%option c++ prefix="til_scanner_" outfile="til_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "til_parser.tab.h"

// don't change this
#define yyerror LexerError


bool parseInt(const std::string &str, int base, int &out);

bool parseDouble(const std::string &str, double &out);

%}
%x X_STRING X_COMMENT X_STRING_IGNORE X_HEX_INT
%%
  yydebug=1; set_debug(1);

";".*$                  ; /* ignore comments */

"/*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"*/"        yy_pop_state();
<X_COMMENT>"/*"        yy_push_state(X_COMMENT); /* nested comment */
<X_COMMENT>.|\n        ; /* ignore comments */

">="                   return tGE; 
"<="                   return tLE; 
"=="                   return tEQ; 
"!="                   return tNE; 
"&&"                   return tAND;
"||"                   return tOR;

  /* Types */
"int"                  return tTYPE_INT;
"double"               return tTYPE_DOUBLE;
"string"               return tTYPE_STRING;
"void"                 return tTYPE_VOID;

  /* Declarations */
"external"             return tEXTERNAL;
"forward"              return tFORWARD;
"public"               return tPUBLIC;
"var"                  return tVAR;

  /* Instructions */
"loop"                 return tLOOP; 
"stop"                 return tSTOP;
"next"                 return tNEXT;
"return"               return tRETURN;

"block"                return tBLOCK;
"if"                   return tIF; 

"print"                return tPRINT; 
"println"              return tPRINTLN; 

 /* Expressions */
"read"                 return tREAD; 
"null"                 return tNULL;
"set"                  return tSET;
"index"                return tINDEX;
"objects"              return tOBJECTS;
"sizeof"               return tSIZEOF;
"function"             return tFUNCTION;

"program"              return tPROGRAM;


[A-Za-z][A-Za-z0-9]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

\"                     yy_push_state(X_STRING); yylval.s = new std::string("");
<X_STRING>\"           yy_pop_state(); return tSTRING;
<X_STRING>\\\"         *yylval.s += yytext + 1;
<X_STRING>\\\\         *yylval.s += yytext + 1;
<X_STRING>\\t          *yylval.s += '\t';
<X_STRING>\\n          *yylval.s += '\n';
<X_STRING>\\r          *yylval.s += '\r';
<X_STRING>\\0          yy_push_state(X_STRING_IGNORE);
<X_STRING>\\[0-7]{1,3} {
                        /* Handling 1 to 3 digits of 8 bits (0 to 7) */
                        int i = std::stoi(yytext + 1, nullptr, 8);
                        if (i > 255) 
                          yyerror("out of range (maximum value allowed is 255)");
                        *yylval.s += (char) i;
                       }

<X_STRING>\\.          *yylval.s += yytext + 1;
<X_STRING>\n           yyerror("newline in string");
<X_STRING>\0           yyerror("null byte in string");
<X_STRING>.            *yylval.s += yytext;

<X_STRING_IGNORE>\"    yy_pop_state(); yy_pop_state(); return tSTRING;
<X_STRING_IGNORE>\\\"  ;      
<X_STRING_IGNORE>\\\\  ;
<X_STRING_IGNORE>\n    yyerror("newline in string");
<X_STRING_IGNORE>\0    yyerror("null byte in string");
<X_STRING_IGNORE>.     ;

 /* Decimal integers */
0|[1-9][0-9]*          if (parseInt(yytext, 10, yylval.i)) return tINTEGER; yyerror("integer overflow");

 /* Handle integers of base 16 */
"0x"                        yy_push_state(X_HEX_INT);
<X_HEX_INT>[[:xdigit:]]+    {
                              if (parseInt(yytext, 16, yylval.i)) {
                                if (yylval.i == 0) {
                                  yyerror("zero isn't valid as hexadecimal");
                                } else {
                                  yy_pop_state();
                                  return tINTEGER;
                                }
                              } else {
                                yyerror("integer overflow");
                              }
                            }
<X_HEX_INT>.|\n             std::cerr << "WARNING: line " << lineno() << ": bad hexadecimal data!" << std::endl; exit(1);

 /* Handle doubles */
[0-9]*\.[0-9]+([Ee][-+]?[0-9]+)?        if (parseDouble(yytext, yylval.d)) return tDOUBLE; yyerror("double overflow");
[0-9]+\.[0-9]*([Ee][-+]?[0-9]+)?        if (parseDouble(yytext, yylval.d)) return tDOUBLE; yyerror("double overflow");
[0-9]+([Ee][-+]?[0-9]+)                 if (parseDouble(yytext, yylval.d)) return tDOUBLE; yyerror("double overflow");


[-()<>=+*/%{}.~@?!]   return *yytext;

[ \t\n\r]+            ; /* ignore whitespace */

.                     yyerror("Unknown character");

%%

bool parseInt(const std::string& str, int base, int& out) {
  try {
    out = std::stoi(str, nullptr, base);
    return true;
  } catch (std::out_of_range&) {
    return false;
  }
}

bool parseDouble(const std::string& str, double& out) {
  try {
    out = std::stod(str, nullptr);
    return true;
  } catch (std::out_of_range&) {
    return false;
  }
}