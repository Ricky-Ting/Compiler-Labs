/* declared tokens */
%locations
%{
    #include "Tree.h"
    #include "lex.yy.c" 
    
    TreeNode_t* root = NULL;
    void yyerror(char *msg) {
        error_num++;
        fprintf(stderr, "Error type B at Line %d: %s.\n", yylloc.first_line, msg);
    }
%}

%union {
    int type_int;
    float type_float;
    char type_char[55];
    TreeNode_t* type_tree;
}

%token <type_int>INT 
%token <type_float> FLOAT 
%token <type_char> TYPE
%token <type_char> ID
%token SEMI COMMA ASSIGNOP
%token RELOP
%token PLUS MINUS STAR DIV 
%token AND OR DOT NOT
%token LP RP LB RB LC RC
%token STRUCT RETURN IF ELSE WHILE

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left OR
%left AND 
%left RELOP
%left PLUS MINUS LOWER_THAN_NEG
%left STAR DIV 
%right NOT NEG
%left LP RP LB RB DOT

%type <type_tree> Program ExtDefList ExtDef ExtDecList 
%type <type_tree> Specifier StructSpecifier OptTag Tag
%type <type_tree> VarDec FunDec VarList ParamDec
%type <type_tree> CompSt StmtList Stmt DefList Def DecList Dec
%type <type_tree> Exp Args

%%

/* High-level Definitions */

Program : ExtDefList     { 
                            if ($1 != NULL)
                                $$ = newTreeNode("Program", NULL, @$.first_line);
                            else 
                                $$ = newTreeNode("Program", NULL, yylloc.last_line);
                            insertTreeNode($$, $1);
                            root = $$;
                        }
    ;

ExtDefList : ExtDef ExtDefList  {
                                    $$ = newTreeNode("ExtDefList", NULL, @$.first_line);
                                    insertTreeNode($$, $1);
                                    insertTreeNode($$, $2);
                                }
    | { $$ = NULL; }
    ;

ExtDef : Specifier ExtDecList SEMI  {
                                        $$ = newTreeNode("ExtDef", NULL, @$.first_line);
                                        insertTreeNode($$, $1);
                                        insertTreeNode($$, $2);
                                        insertTreeNode($$, newTreeNode("SEMI", NULL, 0));
                                    }
    | Specifier SEMI                {
                                        $$ = newTreeNode("ExtDef", NULL, @$.first_line);
                                        insertTreeNode($$, $1);
                                        insertTreeNode($$, newTreeNode("SEMI", NULL, 0));
                                    }
    | Specifier FunDec CompSt       {
                                        $$ = newTreeNode("ExtDef", NULL, @$.first_line);
                                        insertTreeNode($$, $1);
                                        insertTreeNode($$, $2);
                                        insertTreeNode($$, $3);
                                    }
    | error SEMI {$$ = NULL;}
    | Specifier error {$$ = NULL;}
    ;

ExtDecList : VarDec                 {
                                        $$ = newTreeNode("ExtDecList", NULL, @$.first_line);
                                        insertTreeNode($$, $1);
                                    }
    | VarDec COMMA ExtDecList       {
                                        $$ = newTreeNode("ExtDecList", NULL, @$.first_line);
                                        insertTreeNode($$, $1);
                                        insertTreeNode($$, newTreeNode("COMMA", NULL, 0));
                                        insertTreeNode($$, $3);
                                    }
    ;


/* Specifiers */

Specifier : TYPE                    {
                                        $$ = newTreeNode("Specifier", NULL, @$.first_line);
                                        insertTreeNode($$, newTreeNode("TYPE", $1, 0));
                                    }
    | StructSpecifier               {
                                        $$ = newTreeNode("Specifier", NULL, @$.first_line);
                                        insertTreeNode($$,$1);
                                    }
    ;

StructSpecifier : STRUCT OptTag LC DefList RC   {
                                                    $$ = newTreeNode("StructSpecifier", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("STRUCT", NULL, 0));
                                                    insertTreeNode($$, $2);
                                                    insertTreeNode($$, newTreeNode("LC", NULL, 0));
                                                    insertTreeNode($$, $4);
                                                    insertTreeNode($$, newTreeNode("RC", NULL, 0));
                                                }
    | STRUCT Tag                                {
                                                    $$ = newTreeNode("StructSpecifier", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("STRUCT", NULL, 0));
                                                    insertTreeNode($$, $2);
                                                }
    ;

OptTag : ID                                     {
                                                    $$ = newTreeNode("OptTag", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("ID", $1, 0));
                                                }
    |                                           { $$ = NULL; } 
    ;

Tag : ID                                        {
                                                    $$ = newTreeNode("Tag", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("ID", $1, 0));
                                                }
    ;

/* Declarators */

VarDec : ID                                     {
                                                    $$ = newTreeNode("VarDec", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("ID", $1, 0));
                                                }
    | VarDec LB INT RB                          {
                                                    $$ = newTreeNode("VarDec", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("LB", NULL, 0));
                                                    insertTreeNode_INT($$, $3);
                                                    insertTreeNode($$, newTreeNode("RB", NULL, 0));
                                                }
    ;

FunDec : ID LP VarList RP                       {
                                                    $$ = newTreeNode("FunDec", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("ID", $1, 0));
                                                    insertTreeNode($$, newTreeNode("LP", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                    insertTreeNode($$, newTreeNode("RP", NULL, 0));
                                                }
    | ID LP RP                                  {
                                                    $$ = newTreeNode("FunDec", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("ID", $1, 0));
                                                    insertTreeNode($$, newTreeNode("LP", NULL, 0));
                                                    insertTreeNode($$, newTreeNode("RP", NULL, 0));

                                                }
    | ID LP error RP {$$ = NULL;}
    ;

VarList : ParamDec COMMA VarList                {
                                                    $$ = newTreeNode("VarList", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("COMMA", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | ParamDec                                  {
                                                    $$ = newTreeNode("VarList", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                }
    ;

ParamDec : Specifier VarDec                     {
                                                    $$ = newTreeNode("ParamDec", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, $2);
                                                }
    ;

/* Statements */

CompSt : LC DefList StmtList RC                 {
                                                    $$ = newTreeNode("CompSt", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("LC", NULL, 0));
                                                    insertTreeNode($$, $2);
                                                    insertTreeNode($$, $3);
                                                    insertTreeNode($$, newTreeNode("RC", NULL, 0));
                                                }
        | error RC {$$ = NULL;}
        ;

StmtList : Stmt StmtList                        {
                                                    $$ = newTreeNode("StmtList", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, $2);
                                                }
    |                                           { $$ = NULL; }
    ;

Stmt : Exp SEMI                                 {
                                                    $$ = newTreeNode("Stmt", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("SEMI", NULL, 0));
                                                }
    | CompSt                                    {
                                                    $$ = newTreeNode("Stmt", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                }
    | RETURN Exp SEMI                           {
                                                    $$ = newTreeNode("Stmt", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("RETURN", NULL, 0));
                                                    insertTreeNode($$, $2);
                                                    insertTreeNode($$, newTreeNode("SEMI", NULL, 0));
                                                }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE   {
                                                    $$ = newTreeNode("Stmt", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("IF", NULL, 0));
                                                    insertTreeNode($$, newTreeNode("LP", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                    insertTreeNode($$, newTreeNode("RP", NULL, 0));
                                                    insertTreeNode($$, $5);
                                                }
    | IF LP Exp RP Stmt ELSE Stmt               {
                                                    $$ = newTreeNode("Stmt", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("IF", NULL, 0));
                                                    insertTreeNode($$, newTreeNode("LP", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                    insertTreeNode($$, newTreeNode("RP", NULL, 0));
                                                    insertTreeNode($$, $5);
                                                    insertTreeNode($$, newTreeNode("ELSE", NULL, 0));
                                                    insertTreeNode($$, $7);
                                                }
    | WHILE LP Exp RP Stmt                      {
                                                    $$ = newTreeNode("Stmt", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("WHILE", NULL, 0));
                                                    insertTreeNode($$, newTreeNode("LP", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                    insertTreeNode($$, newTreeNode("RP", NULL, 0));
                                                    insertTreeNode($$, $5);
                                                }
    | Exp error SEMI { $$ = NULL; }
    | error SEMI {  $$ = NULL; }

/* Local Definitions */

DefList : Def DefList                           {
                                                    $$ = newTreeNode("DefList", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, $2);
                                                }
    |                                           { $$ = NULL; }
    ;

Def : Specifier DecList SEMI                    {
                                                    $$ = newTreeNode("Def", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, $2);
                                                    insertTreeNode($$, newTreeNode("SEMI", NULL, 0));
                                                }
    | Specifier error SEMI { $$ = NULL;}
    ;

DecList : Dec                                   {
                                                    $$ = newTreeNode("DecList", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                }
    | Dec COMMA DecList                         {
                                                    $$ = newTreeNode("DecList", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("COMMA", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    ;

Dec : VarDec                                    {
                                                    $$ = newTreeNode("Dec", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                }
    | VarDec ASSIGNOP Exp                       {
                                                    $$ = newTreeNode("Dec", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("ASSIGNOP", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | VarDec ASSIGNOP error {$$ = NULL;}
    ;

/* Expressions */

Exp : Exp ASSIGNOP Exp                          {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("ASSIGNOP", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | Exp AND Exp                               {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("AND", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | Exp OR Exp                                {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("OR", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | Exp RELOP Exp                             {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("RELOP", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | Exp PLUS Exp                              {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("PLUS", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | Exp MINUS Exp          %prec LOWER_THAN_NEG             {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("MINUS", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | Exp STAR Exp                              {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("STAR", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | Exp DIV Exp                               {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("DIV", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | LP Exp RP                                 {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("LP", NULL, 0));
                                                    insertTreeNode($$, $2);
                                                    insertTreeNode($$, newTreeNode("RP", NULL, 0));
                                                }
    | MINUS Exp     %prec NEG                    {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("MINUS", NULL, 0));
                                                    insertTreeNode($$, $2);
                                                }
    | NOT Exp                                   {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("NOT", NULL, 0));
                                                    insertTreeNode($$, $2);
                                                }
    | ID LP Args RP                             {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("ID", $1, 0));
                                                    insertTreeNode($$, newTreeNode("LP", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                    insertTreeNode($$, newTreeNode("RP", NULL, 0));
                                                }
    | ID LP RP                                  {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("ID", $1, 0));
                                                    insertTreeNode($$, newTreeNode("LP", NULL, 0));
                                                    insertTreeNode($$, newTreeNode("RP", NULL, 0));
                                                }
    | Exp LB Exp RB                             {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("LB", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                    insertTreeNode($$, newTreeNode("RB", NULL, 0));
                                                }
    | Exp DOT ID                                {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("DOT", NULL, 0));
                                                    insertTreeNode($$, newTreeNode("ID", $3, 0));
                                                }
    | ID                                        {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode($$, newTreeNode("ID", $1, 0));
                                                }
    | INT                                       {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode_INT($$, $1);
                                                }
    | FLOAT                                     {
                                                    $$ = newTreeNode("Exp", NULL, @$.first_line);
                                                    insertTreeNode_FLOAT($$, $1);
                                                }
    | ID LP error RP {$$ = NULL;}
    ;

Args : Exp COMMA Args                           {
                                                    $$ = newTreeNode((char*)"Args", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                    insertTreeNode($$, newTreeNode("COMMA", NULL, 0));
                                                    insertTreeNode($$, $3);
                                                }
    | Exp                                       {
                                                    $$ = newTreeNode("Args", NULL, @$.first_line);
                                                    insertTreeNode($$, $1);
                                                }
    ;




%% 

void printAST() {
    if(error_num == 0)
        printTree(root, 0);
}

