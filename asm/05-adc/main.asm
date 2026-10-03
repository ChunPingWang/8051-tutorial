;=============================================================================
; 第 5 課:ADC 類比輸入 ─ 讀可變電阻的電壓  ─  組合語言版
;=============================================================================
; 與 C 版 (c/05-adc) 讀的是同一個東西:AIN0 (P1.7) 的 12 位元 ADC 值。
;
; 差異:C 版把數值印成「十進位 (0~4095)」;為了讓組語保持單純,
;       組語版把數值印成「十六進位 3 位 (000~FFF)」。核心動作完全相同。
;
; 接線:可變電阻中間腳 ──► P1.7;兩端接 VDD / GND。
;       P0.6 (TXD) ──► USB-TTL 的 RX,GND 共接,電腦端 9600 8N1。
;=============================================================================

    .module adc

    ; ---- SFR ----
    P0M1    = 0xB1
    P0M2    = 0xB2
    P1M1    = 0xB3
    P1M2    = 0xB4
    SCON    = 0x98
    SBUF    = 0x99
    PCON    = 0x87
    TMOD    = 0x89
    CKCON   = 0x8E
    T3CON   = 0xC4
    TH1     = 0x8D
    TL1     = 0x8B
    ADCCON0 = 0xE8
    ADCCON1 = 0xE1
    ADCRL   = 0xC2
    ADCRH   = 0xC3
    AINDIDS = 0xF6
    ; ---- bits ----
    TR1     = 0x8E        ; TCON.6
    TI      = 0x99        ; SCON.1
    ADCF    = 0xEF        ; ADCCON0.7 轉換完成旗標
    ADCS    = 0xEE        ; ADCCON0.6 啟動轉換

    .area CODE (ABS,CODE)
    .org  0x0000
    ljmp  start

start:
    lcall uart_init
    lcall adc_init

loop:
    lcall adc_read         ; 結果:R6 = ADCRH(高8位),R7 = ADCRL 低4位
    ; 印出 "ADC=0x"
    mov   DPTR, #msg_pre
    lcall puts
    ; 印高 nibble(ADCRH 的高 4 位)
    mov   A, R6
    swap  A
    anl   A, #0x0F
    lcall put_hex
    ; 印中 nibble(ADCRH 的低 4 位)
    mov   A, R6
    anl   A, #0x0F
    lcall put_hex
    ; 印低 nibble(ADCRL 的低 4 位)
    mov   A, R7
    anl   A, #0x0F
    lcall put_hex
    ; 換行
    mov   DPTR, #msg_crlf
    lcall puts
    lcall delay
    sjmp  loop

;-----------------------------------------------------------------------------
; UART 初始化(同第 4 課,9600 8N1)
;-----------------------------------------------------------------------------
uart_init:
    anl   P0M1, #0x3F
    anl   P0M2, #0x3F
    mov   SCON,  #0x50
    anl   TMOD,  #0x0F
    orl   TMOD,  #0x20
    orl   PCON,  #0x80
    orl   CKCON, #0x10
    anl   T3CON, #0xDF
    mov   TH1,   #0x98
    mov   TL1,   #0x98
    setb  TR1
    ret

;-----------------------------------------------------------------------------
; ADC 初始化:AIN0 = P1.7
;-----------------------------------------------------------------------------
adc_init:
    orl   P1M1, #0x80      ; P1M1.7 = 1
    anl   P1M2, #0x7F      ; P1M2.7 = 0  → P1.7 僅輸入
    anl   ADCCON0, #0xF0   ; 通道選 0 (AIN0)
    mov   AINDIDS, #0x01   ; 禁用 P1.7 數位輸入
    orl   ADCCON1, #0x01   ; ADCEN = 1 開啟 ADC
    ret

;-----------------------------------------------------------------------------
; adc_read:啟動一次轉換,把結果放到 R6(ADCRH)、R7(ADCRL)
;-----------------------------------------------------------------------------
adc_read:
    clr   ADCF             ; 清完成旗標
    setb  ADCS             ; 啟動轉換
adc_wait:
    jnb   ADCF, adc_wait   ; 等完成
    mov   R6, ADCRH
    mov   R7, ADCRL
    ret

;-----------------------------------------------------------------------------
; put_hex:把 A 的低 4 位(0~15)印成一個十六進位字元
;-----------------------------------------------------------------------------
put_hex:
    anl   A, #0x0F
    mov   DPTR, #hextab
    movc  A, @A+DPTR       ; 查表得到 '0'~'F'
    lcall putc
    ret

;-----------------------------------------------------------------------------
; puts / putc(同第 4 課)
;-----------------------------------------------------------------------------
puts:
    clr   A
    movc  A, @A+DPTR
    jz    puts_done
    lcall putc
    inc   DPTR
    sjmp  puts
puts_done:
    ret

putc:
    mov   SBUF, A
wait_ti:
    jnb   TI, wait_ti
    clr   TI
    ret

delay:
    mov   R2, #15
e2:
    mov   R1, #0
e1:
    mov   R0, #0
e0:
    djnz  R0, e0
    djnz  R1, e1
    djnz  R2, e2
    ret

;-----------------------------------------------------------------------------
; 字串與查表資料
;-----------------------------------------------------------------------------
hextab:
    .ascii "0123456789ABCDEF"
msg_pre:
    .ascii "ADC=0x"
    .db    0x00
msg_crlf:
    .db    0x0D, 0x0A, 0x00
