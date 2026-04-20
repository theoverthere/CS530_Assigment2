   1                  .  SOURCE CODE FOR THE XE VERSION OF THE SIC FAMILY OF COMPUTER
   2  0000            SUM       START   0                      SAMPLE PROGRAM
   3  0000            FIRST     LDX    #0          
   4  0003                      LDA    #0          
   5  0006                     +LDB    #TABLE2     
   6  000A                      BASE    TABLE2
   7  000A            LOOP      ADD     TABLE,X    
   8  000D                      ADD     TABLE2,X   
   9  0010                      TIX     COUNT      
  10  0013                      JLT     LOOP       
  11  0016                     +STA     TOTAL      
  12  001A                      RSUB               
  13  001D            MYLIT    +LDA    =C'EOF'
  14  0021            COUNT     RESW    1
  15  0024            TABLE     RESW    2000
  16  1794            TABLE2    RESW    2000
  17  2F04            TOTAL     RESW    1
  18  2F07            *        =C'EOF'    *** Invalid opcode: *
  19  2F07                      END     FIRST
