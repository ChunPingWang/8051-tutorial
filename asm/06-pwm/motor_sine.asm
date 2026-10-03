;=============================================================================
; 第 6 課【進階】:用正弦曲線控制直流馬達轉速(平滑加減速)─ 組合語言版
;=============================================================================
; 與 C 版 (c/06-pwm/motor_sine.c) 做的事相同:讓馬達轉速依正弦曲線起伏,
; 平滑地加速、減速、循環。
;
; ★★ 馬達不能直接接 MCU 腳位!必須經過馬達驅動器(見下)。★★
;     P1.2 (PWM0) ──► 馬達驅動器輸入(L9110 / TB6612 / N-MOSFET…)
;     馬達由獨立電源供電,並聯續流二極體,與 MCU 共地。
;     詳見 docs/08-PWM.md。
;
; 與 C 版的差異:
;   C 版用「0~255 正弦表」再即時壓縮到 [40,255];組語版為了省去乘除運算,
;   直接改用「已經壓縮好的正弦表 (40~255)」查表,結果完全相同。
;
; 編譯/燒錄:
;     make TARGET=motor_sine
;     make TARGET=motor_sine flash
;=============================================================================

    .module motor_sine

    P1M1    = 0xB3
    P1M2    = 0xB4
    PWMCON0 = 0xD8
    PWMCON1 = 0xDF
    PNP     = 0xD6
    PIOCON0 = 0xDE
    PWMPH   = 0xD1
    PWMPL   = 0xD9
    PWM0H   = 0xD2
    PWM0L   = 0xDA
    PWMRUN  = 0xDF        ; PWMCON0.7
    LOAD    = 0xDE        ; PWMCON0.6

    .area CODE (ABS,CODE)
    .org  0x0000
    ljmp  start

start:
    lcall pwm_init
    mov   R7, #0          ; R7 = 查表索引 (0~63)

main_loop:
    ; A = sine_table[R7]
    mov   A, R7
    mov   DPTR, #sine_table
    movc  A, @A+DPTR
    lcall set_duty        ; 依正弦值設定轉速

    inc   R7
    cjne  R7, #64, next   ; 索引到 64 就歸零(走完一個完整循環)
    mov   R7, #0
next:
    lcall step_delay
    sjmp  main_loop

;-----------------------------------------------------------------------------
pwm_init:
    anl   P1M1, #0xFB
    orl   P1M2, #0x04     ; P1.2 推挽輸出
    mov   PWMCON0, #0x00
    mov   PWMCON1, #0x07  ; Fsys/128
    mov   PNP,     #0x00
    mov   PIOCON0, #0x01  ; PWM0 → P1.2
    mov   PWMPH,   #0x00
    mov   PWMPL,   #0xFF  ; 週期 256 階
    mov   PWM0H,   #0x00
    mov   PWM0L,   #0x00
    setb  LOAD
    setb  PWMRUN
    ret

; set_duty:A = duty
set_duty:
    mov   PWM0H, #0x00
    mov   PWM0L, A
    setb  LOAD
    ret

; step_delay:決定加減速快慢(用 R4/R5/R6,不動主迴圈的 R7)
step_delay:
    mov   R4, #12
t2:
    mov   R5, #0
t1:
    mov   R6, #0
t0:
    djnz  R6, t0
    djnz  R5, t1
    djnz  R4, t2
    ret

;-----------------------------------------------------------------------------
; 已壓縮的正弦查表:64 點,值域 40~255(事先用電腦算好)
;   value[i] = round(40 + 215·(0.5+0.5·sin(2π·i/64)))
;-----------------------------------------------------------------------------
sine_table:
    .db 148, 158, 168, 179, 188, 199, 207, 215
    .db 224, 231, 237, 242, 247, 251, 253, 254
    .db 255, 254, 253, 251, 247, 242, 237, 231
    .db 224, 215, 207, 199, 188, 179, 168, 158
    .db 148, 137, 127, 116, 107, 96, 88, 80
    .db 71, 64, 58, 53, 48, 44, 42, 41
    .db 40, 41, 42, 44, 48, 53, 58, 64
    .db 71, 80, 88, 96, 107, 116, 127, 137
