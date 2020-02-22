
digit   [0-9]
letter_  [a-zA-Z_]

int     0|([1-9]{digit}*)|(0[0-7]+)|(0[xX][0-9A-Fa-f]+)
float   ({digit}+\.{digit}+)|((({digit}+\.{digit}*)|({digit}*\.{digit}+))[eE][+-]?{digit}+)

relop   >|<|>=|<=|==|!=
type    int|float

comment (\/\/.*\n)|(\/\*(.|\n)*\*\/)
id      {letter_}[{letter_}|digit]+

%%

{int}       { printf("INT\n"); }
{float}     { printf("FLOAT\n"); }
{id}        { printf("ID\n"); }
{comment}   { printf("Comment\n"); }
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


%%





