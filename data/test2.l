   1  0000            BYTETST START   0
   2  0000  032006    FIRST   LDA     ALPHA
   3  0003  0F2007            STA     BETA
   4  0006  4F0000            RSUB
   5  0009  454F46    ALPHA   BYTE    C'EOF'
   6  000C  F1        HEXVAL  BYTE    X'F1'
   7  000D            BETA    RESB    4
   8  0011            GAMMA   RESW    2
   9  0017                    END     FIRST
