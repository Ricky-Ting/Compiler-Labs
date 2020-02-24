
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

delimit " "|\n|\t|\r
%%

{int}       { printf("INT\n"); }
{float}     { printf("FLOAT\n"); }
{comment}   { printf("Comment\n"); }
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
                    printf("Comment\n"); 
                else
                    printf("Error type A at Line %d: Unterminated comment \n", yylineno);
                } 
";"         { printf("SEMI\n"); }
","         { printf("COMMA\n"); }
"="         { printf("ASSIGNOP\n"); }
{relop}     { printf("RELOP\n"); }
"+"         { printf("PLUS\n");}
"-"         { printf("MINUS\n");}
"*"         { printf("STAR\n");}
"/"         { printf("DIV\n");}
"&&"        { printf("AND\n");}
"||"        { printf("OR\n");}
"."         { printf("DOT\n");}
"!"         { printf("NOT\n"); }
{type}      { printf("TYPE\n");}
"("         { printf("LP\n");}
")"         { printf("RP\n");}
"["         { printf("LB\n");}
"]"         { printf("RB\n");}
"{"         { printf("LC\n");}
"}"         { printf("RC\n");}
"struct"    { printf("STRUCT\n");}
"return"    { printf("RETURN\n");}
"if"        { printf("IF\n");}
"else"      { printf("ELSE\n");}
"while"     { printf("WHILE\n");}
{id}        { printf("ID\n"); }
{delimit}   {  }
.           { 
                printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
            }

%%





