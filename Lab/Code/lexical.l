
%{
    #include "syntax.tab.h"
%}
%option yylineno
digit   [0-9]
letter_  [a-zA-Z_]

int     0|([1-9]{digit}*)|(0[0-7]+)|(0[xX][0-9A-Fa-f]+)
float   ({digit}+\.{digit}+)|((({digit}+\.{digit}*)|({digit}*\.{digit}+))[eE][+-]?{digit}+)

relop   >|<|>=|<=|==|!=
type    int|float

comment \/\/.*
multicomment \/\*
id      {letter_}({letter_}|{digit})*

delimit " "|\t|\r

%%

{int}       { return INT; }
{float}     { return FLOAT; }
{comment}   {  }
{multicomment}   { 
                int state = 0;
                while(1) {
                    char c = input();
                    if(c == '\0')
                        break;
                    if(c == '*') {
                        state = 1;
                    } else if (c == '/') {
                        if(state == 1) {
                            state = 2;
                            break;
                        } else {
                            state = 0;
                        }
                    } else {
                        state = 0;
                    }
                }
                if(state == 2)
                    ; 
                else
                    printf("Error type A at Line %d: Unterminated comment \n", yylineno);
                } 
";"         { return SEMI; }
","         { return COMMA; }
"="         { return ASSIGNOP; }
{relop}     { return RELOP; }
"+"         { return PLUS;}
"-"         { return MINUS;}
"*"         { return STAR;}
"/"         { return DIV;}
"&&"        { return AND;}
"||"        { return OR;}
"."         { return DOT;}
"!"         { return NOT; }
{type}      { return TYPE;}
"("         { return LP;}
")"         { return RP;}
"["         { return LB;}
"]"         { return RB;}
"{"         { return LC;}
"}"         { return RC;}
"struct"    { return STRUCT;}
"return"    { return RETURN;}
"if"        { return IF;}
"else"      { return ELSE;}
"while"     { return WHILE;}
{id}        { return ID; }
{delimit}   {  }
\n          { yycolumn = 1; }
.           { 
                printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
            }

%%



