   1                  .  SOURCE CODE FOR THE XE VERSION OF THE SIC FAMILY OF COMPUTER
   2  0000            SUM       START   0                      SAMPLE PROGRAM
   3  0000  050000    FIRST     LDX    #0          
   4  0003  010000              LDA    #0          
   5  0006  69101794           +LDB    #TABLE2     
   6  000A                      BASE    TABLE2
   7  000A  1BA017    LOOP      ADD     TABLE,X    
   8  000D  1BC000              ADD     TABLE2,X   
   9  0010  2F200E              TIX     COUNT      
  10  0013  3B2FF4              JLT     LOOP       
  11  0016  0F102F04           +STA     TOTAL      
  12  001A  4F0000              RSUB               
  13  001D  03102F07  MYLIT    +LDA    =C'EOF'
  14  0021            COUNT     RESW    1
  15  0024            TABLE     RESW    2000
  16  1794            TABLE2    RESW    2000
  17  2F04            TOTAL     RESW    1
  18  2F07  454F46    *        =C'EOF'
  19  2F0A                      END     FIRST
