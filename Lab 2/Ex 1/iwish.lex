

%{
    /*
    *************************************************************************
    *                                Ex.1                                   *
    * Author(s):   Barnabas Busa, Joe Jones, Charles Randolph.              *
    *************************************************************************
    */ 
    #include <stdlib.h>
    #include "iwish.tab.h"
%}

word            [^ \t\n<>|&;]+

%%

"<"             { printf("IN-DIR \"%s\"\n", yytext); return REDIR_IN;  }
">"             { printf("OUT-DIR \"%s\"\n", yytext); return REDIR_OUT; }
"|"             { printf("PIPE \"%s\"\n", yytext); return PIPE;      }
"&"             { printf("AMP \"%s\"\n", yytext); return AMPERSAND; }
";"             { printf("S-COLON \"%s\"\n", yytext); return SEMICOLON; }
"\n"            { printf("NEWLINE \"%s\"\n", yytext); return NEWLINE;   }

{word}          { printf("Word \"%s\"\n", yytext); return WORD;      }

<<EOF>>         { printf("Accepted EOF!\n"); return I_EOF;       }

%%