   1  1000            MODES   START   1000
   2  1000  010005    FIRST   LDA     #5
   3  1003  692010            LDB     #TABLE
   4  1006  02200A            LDA     @VALUE
   5  1009  1BA00A            ADD     TABLE,X
   6  100C  4B101022          +JSUB   SUBRTN
   7  1010  4F0000            RSUB
   8  1013  00000A    VALUE   WORD    10
   9  1016            TABLE   RESW    4
  10  1022  4F0000    SUBRTN  RSUB
  11  1025                    END     FIRST
