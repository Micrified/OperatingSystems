

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "list.h"
    
    /* External Flex Variables */
    extern int yylex();
    extern void yylex_destroy();
    extern char *yytext;

    /* Handler for Bison parse errors */
    int yyerror(char *s) {
        printf("PARSE ERROR!\n");
        exit(EXIT_SUCCESS);
    }
%}

/*
********************************************************************************
*                               Token Declarations
********************************************************************************
*/

// Complex Tokens.
%token  WORD

// Simple Tokens.
%token  NEWLINE
%token  AMPERSAND
%token  SEMICOLON
%token  REDIR_IN
%token  REDIR_OUT
%token  PIPE
%token  I_EOF

// The starting rule.
%start program

%%

program          : linebreak complete_commands linebreak I_EOF    { printf("ACCEPTED!\n"); printTokenList(); YYACCEPT; }
                 | linebreak I_EOF                                { printf("ACCEPTED!\n"); printTokenList(); YYACCEPT; } 
                 ;

complete_commands: complete_commands newline_list complete_command
                 |                                complete_command
                 ;

complete_command : list separator_op
                 | list
                 ;

list             : list separator_op pipe_sequence { printf("COMMAND = "); printTokenList(); printf("\n"); }
                 |                   pipe_sequence { printf("COMMAND = "); printTokenList(); printf("\n"); }
                 ;

pipe_sequence    :                             simple_command
                 | pipe_sequence PIPE { appendOperator(TYPE_PIPE); } linebreak simple_command
                 ;

simple_command   : cmd_name cmd_suffix
                 | cmd_name
                 ;

cmd_name         : WORD { appendWord(yytext); }
                 ;

cmd_suffix       :            io_file
                 | cmd_suffix io_file
                 |            WORD { appendWord(yytext); }
                 | cmd_suffix WORD { appendWord(yytext); }
                 ;

io_file          : REDIR_IN     { appendOperator(TYPE_REDIR_IN);  }    filename 
                 | REDIR_OUT    { appendOperator(TYPE_REDIR_OUT); }    filename 
                 ;

filename         : WORD { appendWord(yytext); }                
                 ;

newline_list     :              NEWLINE
                 | newline_list NEWLINE
                 ;

linebreak        : newline_list
                 | /* empty */
                 ;

separator_op     : AMPERSAND    { appendOperator(TYPE_AMP); }    
                 | SEMICOLON    { appendOperator(TYPE_SCOLON); } 
                 ;

%% 

int main (void) {
    yyparse();

    freeTokenList();
    yylex_destroy();
    return EXIT_SUCCESS;
}
