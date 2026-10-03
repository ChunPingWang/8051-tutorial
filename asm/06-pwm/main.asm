;=============================================================================
; 第 6 課:PWM 呼吸燈 ─ 用硬體控制 LED 亮度  ─  組合語言版
;=============================================================================
; 與 C 版 (c/06-pwm) 做的事完全一樣:P1.2 的 LED 由暗漸亮、再漸暗地呼吸。
;
; 接線:P1.2 --> 220Ω --> LED 長腳(+),LED 短腳(−)--> GND
;       (PWM0 固定對應 P1.2,所以這一課 LED 接 P1.2)
;
; 重點:PWM 設定好之後,「變亮變暗」靠的是一直改工作週期 PWM0L (0~255),
;       每次改完要 setb LOAD 讓新值生效。
;=============================================================================

    .module pwm

    ; ---- SFR ----
    P1M1    = 0xB3
    P1M2    = 0xB4
    PWMCON0 = 0xD8
    PWMCON1 = 0xDF        ; 注意:這是 SFR(整體存取)
    PNP     = 0xD6
    PIOCON0 = 0xDE
    PWMPH   = 0xD1
    PWMPL   = 0xD9
    PWM0H   = 0xD2
    PWM0L   = 0xDA
    ; ---- bits(PWMCON0 可位元定址,0xD8~0xDF)----
    PWMRUN  = 0xDF        ; PWMCON0.7 啟動 PWM
    LOAD    = 0xDE        ; PWMCON0.6 載入新的週期/duty

    .area CODE (ABS,CODE)
    .org  0x0000
    ljmp  start

start:
    lcall pwm_init

main_loop:
    ; ---- 由暗漸亮:duty 0 → 255 ----
    mov   R7, #0
up:
    mov   A, R7
    lcall set_duty
    lcall step_delay
    inc   R7
    mov   A, R7
    jnz   up               ; R7 加到 0(即做完 255)才停

    ; ---- 由亮漸暗:duty 255 → 1 ----
    mov   R7, #255
down:
    mov   A, R7
    lcall set_duty
    lcall step_delay
    dec   R7
    mov   A, R7
    jnz   down

    sjmp  main_loop

;-----------------------------------------------------------------------------
; PWM 初始化:PWM0 → P1.2
;-----------------------------------------------------------------------------
pwm_init:
    anl   P1M1, #0xFB      ; 1111 1011 → 清 P1M1.2
    orl   P1M2, #0x04      ; 0000 0100 → 設 P1M2.2 → P1.2 推挽輸出
    mov   PWMCON0, #0x00
    mov   PWMCON1, #0x07   ; PWM 時脈 = Fsys/128,邊緣對齊、獨立模式
    mov   PNP,     #0x00   ; 正常極性
    mov   PIOCON0, #0x01   ; PWM0 輸出到 P1.2
    mov   PWMPH,   #0x00
    mov   PWMPL,   #0xFF   ; 週期 256 階
    mov   PWM0H,   #0x00
    mov   PWM0L,   #0x00   ; 初始 duty = 0
    setb  LOAD
    setb  PWMRUN           ; 啟動 PWM
    ret

;-----------------------------------------------------------------------------
; set_duty:把 A 設為 PWM0 的工作週期 (0~255)
;-----------------------------------------------------------------------------
set_duty:
    mov   PWM0H, #0x00
    mov   PWM0L, A
    setb  LOAD             ; 讓新 duty 生效
    ret

;-----------------------------------------------------------------------------
; step_delay:很短的延時(用 R5/R6,不動到主迴圈的 R7)
;-----------------------------------------------------------------------------
step_delay:
    mov   R5, #2
sd1:
    mov   R6, #0
sd0:
    djnz  R6, sd0
    djnz  R5, sd1
    ret
