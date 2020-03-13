
%{
    #include "syntax.tab.h"
    int yycolumn = 1;
    int error_num = 0;
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}
%option yylineno
digit   [0-9]
letter_  [a-zA-Z_]

int     0|([1-9]{digit}*)|(0[0-7]+)|(0[xX][0-9A-Fa-f]+)
float   ({digit}+\.{digit}+)|((({digit}+\.{digit}*)|({digit}*\.{digit}+))[eE][+-]?{digit}+)

oct_error 0[0-9a-zA-Z]
/* hex_error 0[xX][0-9A-Fa-f]*[g-zG-Z]+[0-9a-zA-Z]* */
float_error ({digit}+\.)|(\.{digit}+)|((({digit}+\.{digit}*)|({digit}*\.{digit}+))([eE][+-]*|[eE][+-]?{digit}+[a-zA-Z]+{digit}*))

relop   >|<|>=|<=|==|!=
type    int|float

comment \/\/.*
multicomment \/\*
id      {letter_}({letter_}|{digit})*

delimit " "|\t|\r

%%

{int}       { yylval.type_int = strtol(yytext, NULL, 0); return INT; }
{float}     { yylval.type_float = atof(yytext); return FLOAT; }
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
                else {
                    error_num++;
                    //printf("Error type A at Line %d: Unterminated comment. \n", yylineno);
                    fprintf(stderr, "Error type A at Line %d: Unterminated comment. \n", yylineno);
                }
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
{type}      { strcpy(yylval.type_char, yytext); return TYPE;}
"("         { 
                //printf("LP\n");
                return LP;
            }
")"         { return RP;}
"["         { //printf("LB\n"); 
                return LB;
            }
"]"         { //printf("RB\n"); 
                return RB;
            }
"{"         { return LC;}
"}"         { return RC;}
"struct"    { return STRUCT;}
"return"    { return RETURN;}
"if"        { return IF;}
"else"      { return ELSE;}
"while"     { return WHILE;}
{oct_error} {   
                error_num++;
                //printf("Error type A at Line %d: Invalid octal or hexadecimal number \'%s\'.\n", yylineno, yytext); 
                fprintf(stderr, "Error type A at Line %d: Invalid octal or hexadecimal number \'%s\'.\n", yylineno, yytext); }
{float_error} { 
                error_num++;
                //printf("Error type A at Line %d: Invalid float number \'%s\'.\n", yylineno, yytext); 
                fprintf(stderr, "Error type A at Line %d: Invalid float number \'%s\'.\n", yylineno, yytext); }
{id}        {   
                strcpy(yylval.type_char, yytext); 
                return ID; 
            }
{delimit}   {  }
\n          { yycolumn = 1; }
.           { 
                error_num++;
                //printf("Error type A at Line %d: Mysterious characters \'%s\'.\n", yylineno, yytext);
                fprintf(stderr, "Error type A at Line %d: Mysterious characters \'%s\'.\n", yylineno, yytext);
            }

%%



