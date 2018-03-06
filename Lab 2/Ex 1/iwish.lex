
%{
    #include <stdlib.h>
    #include "iwish.tab.h"
%}

word            [^ \t\n<>|&;]+

%%

"&"             { return AMPERSAND; }
"<"             { return IN;  }
">"             { return OUT; }
"|"             { return PIPE;      }
";"             { return SEMICOLON; }
"\n"            { return NEWLINE;   }

{word}          { return WORD;      }

<<EOF>>         { return END; }

%%