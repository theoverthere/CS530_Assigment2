   1  0000            BASETST START   0
   2  0000            FIRST   LDB     #BIGTAB    *** Displacement out of range and no BASE set
   3  0003                    BASE    BIGTAB
   4  0003  034000            LDA     BIGTAB
   5  0006  0F4003            STA     TARGET
   6  0009                    NOBASE
   7  0009  4F0000            RSUB
   8  000C            PAD     RESB    5000
   9  1394            BIGTAB  RESW    1
  10  1397            TARGET  RESW    1
  11  139A                    END     FIRST
