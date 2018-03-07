
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <assert.h>
    #include "strtab.h"
    #include "queue.h"
    #include "util.h"
    #include "eval.h"

    // [Flex] External variables.
    extern int  yylex();
    extern void yylex_destroy();
    extern char *yytext;
    
    // [Bison] Parse error routine.
    int yyerror (char *s) {
        fprintf(stderr, "Error: Bad Parse!\n");
        exit(EXIT_FAILURE);
    }
%}

%token AMPERSAND
%token END
%token IN
%token NEWLINE
%token PIPE
%token OUT
%token SEMICOLON
%token WORD

%%

program             : break commandSequence break END   { evalQueue(); YYACCEPT; }
                    | break END                         { YYACCEPT; }
                    ;

commandSequence     : commandSequence newlines compoundCommand
                    | compoundCommand
                    ;

compoundCommand     : sequence delim_op             { evalQueue(); }            
                    | sequence                      { evalQueue(); }  
                    ;

sequence            : sequence delim_op             { evalQueue();  } 
                    pipeSequence
                    | pipeSequence
                    ;

pipeSequence        : pipeSequence PIPE             { enqueue((Item){PIPE, NIL}); } 
                    break command
                    | command
                    ;

command             : word args
                    | word                          
                    ;

args                : args word                     
                    | args redirection
                    | word                                        
                    | redirection
                    ;

redirection         : IN                            { enqueue((Item){IN, NIL});  }
                    word                                           
                    | OUT                           { enqueue((Item){OUT, NIL}); }
                    word                         
                    ;

break               : newlines
                    | /* Epsilon */
                    ;

newlines            : newlines NEWLINE
                    | NEWLINE
                    ;

delim_op            : AMPERSAND                     { enqueue((Item){AMPERSAND, NIL}); }
                    | SEMICOLON                     { enqueue((Item){SEMICOLON, NIL}); }
                    ;

word                : WORD                          { enqueue((Item){WORD, putString(yytext)}); }
                    ;

%%

int main (void) {

    // Print prompt token.
    printf("iwish$ ");

    // Run the input.
    yyparse();

    // Free lexeme buffer.
    freeStringTable();

    // Free path buffer.
    freePaths();

    // Free flex memory.
    yylex_destroy();

    return EXIT_SUCCESS;
}


