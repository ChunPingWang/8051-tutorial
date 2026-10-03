;=============================================================================
; 第 7 課【完整版】:狀態機 ─ 行人穿越號誌  ─  組合語言版
;=============================================================================
; 這是 main.asm(精簡版)的完整功能版本,把 C 版 (c/07-fsm) 的全部功能補齊:
;
;   + 蜂鳴器 (P0.4):行人通行時慢「嗶」,行人催促時急促「嗶」
;   + UART   (P0.6):每次狀態轉移、每次按鈕事件都印一行紀錄到序列埠
;
; 先讀 main.asm 把狀態機的骨架看懂,再回來看這一份多出來的部分
; (差異與取捨的討論見 docs/11 §11.12)。
;
;-----------------------------------------------------------------------------
; 多出來的三個技巧:
;   ① 狀態名稱字串表:每筆固定 16 bytes,所以「索引 × 16」只要一條 swap A
;   ② DPTR 加偏移:DPTR 是 DPH/DPL 兩個 SFR 湊的,要自己做 16 位元加法
;   ③ Timer0 管 1ms 時基、Timer1 管 UART 鮑率,兩個計時器各司其職
;
; 接線:
;   車道紅 P1.0  車道黃 P1.1  車道綠 P1.3  行人綠 P1.4  行人紅 P1.6
;        (每顆 LED 各自串 220Ω 到 GND)
;   行人按鈕 P1.5 ----[ 按鈕 ]---- GND
;   蜂鳴器   P0.4 ---- 有源蜂鳴器 ---- GND
;   UART     P0.6 (TXD) ----> USB-TTL 的 RX;GND 共接。電腦端 9600 8N1。
;
; 序列埠會看到(狀態名稱用英文,理由見本檔 name_table 的註解):
;   === N76E003 Pedestrian Crossing (ASM) ===
;   Press P1.5 to request crossing
;   -> VEH GREEN
;   BTN: recorded
;   -> VEH YELLOW
;   -> PED WALK
;   BTN: ignored
;   -> PED FLASH
;   -> ALL RED
;   -> VEH GREEN
;
; 編譯:  make TARGET=main_full
; 燒錄:  make TARGET=main_full flash
;=============================================================================

    .module fsm_full

    ; ---- SFR ----
    P1    = 0x90
    P1M1  = 0xB3
    P1M2  = 0xB4
    P0M1  = 0xB1
    P0M2  = 0xB2
    TMOD  = 0x89
    CKCON = 0x8E
    TL0   = 0x8A
    TH0   = 0x8C
    TH1   = 0x8D
    TL1   = 0x8B
    SCON  = 0x98
    SBUF  = 0x99
    PCON  = 0x87
    T3CON = 0xC4
    DPL   = 0x82           ; DPTR 的低位元組
    DPH   = 0x83           ; DPTR 的高位元組
    ; ---- 可定址的位元 ----
    BTN    = 0x95          ; P1.5 行人按鈕
    BUZZER = 0x84          ; P0.4 蜂鳴器
    TR0    = 0x8C          ; TCON.4 啟動 Timer0
    TR1    = 0x8E          ; TCON.6 啟動 Timer1
    TI     = 0x99          ; SCON.1 傳送完成旗標
    ET0    = 0xA9          ; IE.1  Timer0 中斷致能
    EA     = 0xAF          ; IE.7  總中斷

    ; ---- 狀態編號(同時就是各張表的索引)----
    ST_GREEN  = 0
    ST_YELLOW = 1
    ST_WALK   = 2
    ST_FLASH  = 3
    ST_RED    = 4

    DEBOUNCE  = 20         ; 按鈕要連續穩定 20ms 才承認

    ; ---- RAM 變數 ----
    state   = 0x30
    ticks   = 0x31
    ms_cnt  = 0x32
    deb     = 0x33

    ; ---- 位元變數(都在 RAM 的 0x20 這個位元組裡)----
    request    = 0x00
    btn_stable = 0x01
    tick_flag  = 0x02
    btn_flag   = 0x03

;-----------------------------------------------------------------------------
; 向量區
;-----------------------------------------------------------------------------
    .area CODE (ABS,CODE)
    .org  0x0000
    ljmp  start
    .org  0x000B
    ljmp  timer0_isr

;-----------------------------------------------------------------------------
; 開機初始化
;-----------------------------------------------------------------------------
start:
    ; P1:bit0,1,3,4,6 推挽輸出(LED);bit2,5,7 準雙向(含按鈕 P1.5)
    mov   P1M1, #0x00
    mov   P1M2, #0x5B
    mov   P1,   #0x20      ; LED 全滅;P1.5 寫 1

    ; P0:bit4 推挽輸出(蜂鳴器);bit6,7 準雙向(UART 的 TXD/RXD)
    anl   P0M1, #0x3F      ; 清掉 bit6,7
    anl   P0M1, #0xEF      ; 清掉 bit4
    anl   P0M2, #0x3F      ; 清掉 bit6,7(準雙向)
    orl   P0M2, #0x10      ; bit4 = 1(推挽)
    clr   BUZZER

    lcall uart_init

    ; 位元變數與計數器歸零
    clr   request
    setb  btn_stable
    clr   tick_flag
    clr   btn_flag
    mov   ms_cnt, #0
    mov   deb,    #0

    ; 開機訊息
    mov   DPTR, #msg_banner
    lcall puts

    ; 進入初始狀態(enter_state 會順便印出狀態名稱)
    mov   A, #ST_GREEN
    lcall enter_state
    lcall update_outputs

    ; Timer0:模式1、吃滿 16MHz、每 1ms 溢位
    ;   注意 TMOD 要「同時」容納 Timer0 模式1 與 Timer1 模式2,
    ;   所以 uart_init 用 orl 設高 4 位,這裡只能用 orl 設低 4 位,不能用 mov!
    anl   TMOD,  #0xF0
    orl   TMOD,  #0x01
    orl   CKCON, #0x08     ; T0M = 1
    mov   TH0,   #0xC1
    mov   TL0,   #0x80
    setb  ET0
    setb  EA
    setb  TR0

;-----------------------------------------------------------------------------
; 主迴圈
;-----------------------------------------------------------------------------
main_loop:
    jnb   btn_flag, chk_tick
    clr   btn_flag
    lcall ev_button
chk_tick:
    jnb   tick_flag, main_loop
    clr   tick_flag
    lcall ev_tick
    lcall update_outputs
    sjmp  main_loop

;-----------------------------------------------------------------------------
; 事件:行人按下按鈕(完整版會把處理結果印出來)
;-----------------------------------------------------------------------------
ev_button:
    mov   A, state
    jnz   eb_ignored       ; 不在車道綠燈 → 忽略
    jb    request, eb_ignored   ; 已經登記過 → 再按也沒用
    setb  request
    mov   DPTR, #msg_recorded
    lcall puts
    ret
eb_ignored:
    mov   DPTR, #msg_ignored
    lcall puts
    ret

;-----------------------------------------------------------------------------
; 事件:時間過了一個 tick
;-----------------------------------------------------------------------------
ev_tick:
    mov   A, ticks
    jz    dispatch
    dec   ticks

;-----------------------------------------------------------------------------
; 跳躍表 = C 的 switch (state)
;   ⚠ ajmp 只能跳同一個 2KB 分頁內;本程式不到 1KB,安全。
;-----------------------------------------------------------------------------
dispatch:
    mov   A, state
    rl    A
    mov   DPTR, #jump_table
    jmp   @A+DPTR

jump_table:
    ajmp  st_green
    ajmp  st_yellow
    ajmp  st_walk
    ajmp  st_flash
    ajmp  st_red

;-----------------------------------------------------------------------------
; 各狀態的轉移規則
;-----------------------------------------------------------------------------
st_green:
    mov   A, ticks
    jnz   fsm_ret          ; 守衛條件①:最短綠燈時間還沒到
    jnb   request, fsm_ret ; 守衛條件②:沒有行人請求
    clr   request
    mov   A, #ST_YELLOW
    ajmp  enter_state

st_yellow:
    mov   A, ticks
    jnz   fsm_ret
    mov   A, #ST_WALK
    ajmp  enter_state

st_walk:
    mov   A, ticks
    jnz   fsm_ret
    mov   A, #ST_FLASH
    ajmp  enter_state

st_flash:
    mov   A, ticks
    jnz   fsm_ret
    mov   A, #ST_RED
    ajmp  enter_state

st_red:
    mov   A, ticks
    jnz   fsm_ret
    mov   A, #ST_GREEN
    ; 往下落到 enter_state

;-----------------------------------------------------------------------------
; 進入新狀態(A = 新狀態編號)
;-----------------------------------------------------------------------------
enter_state:
    mov   state, A
    mov   DPTR, #dur_table
    movc  A, @A+DPTR
    mov   ticks, A
    lcall print_state      ; ← 完整版比精簡版多這一行
fsm_ret:
    ret

;-----------------------------------------------------------------------------
; 輸出:燈號(查表)+ 蜂鳴器
;-----------------------------------------------------------------------------
update_outputs:
    ; ---- P1 的五顆 LED ----
    mov   A, state
    mov   DPTR, #out_table
    movc  A, @A+DPTR
    mov   R7, A

    mov   A, state
    cjne  A, #ST_FLASH, uo_write
    mov   A, ticks         ; 0.5 秒 = 4 個 tick → 倒數計數器的 bit2 就是閃爍相位
    anl   A, #0x04
    jz    uo_write
    mov   A, R7
    orl   A, #0x10         ; 點亮行人綠燈 (P1.4)
    mov   R7, A
uo_write:
    mov   A, R7
    orl   A, #0x20         ; P1.5(按鈕腳)維持 1
    mov   P1, A

    ; ---- P0.4 的蜂鳴器 ----
    mov   A, state
    cjne  A, #ST_WALK, uo_chk_flash
    mov   A, ticks
    anl   A, #0x04         ; 每 4 tick = 0.5 秒 → 慢「嗶」
    jnz   uo_buz_on
    sjmp  uo_buz_off
uo_chk_flash:
    cjne  A, #ST_FLASH, uo_buz_off   ; (cjne 不會改到 A,所以 A 還是 state)
    mov   A, ticks
    anl   A, #0x02         ; 每 2 tick = 0.25 秒 → 急促「嗶」
    jnz   uo_buz_on
uo_buz_off:
    clr   BUZZER
    ret
uo_buz_on:
    setb  BUZZER
    ret

;-----------------------------------------------------------------------------
; 印出「-> 狀態名稱」
;   技巧:字串表每筆固定 16 bytes,所以「索引 × 16」只要一條 swap A
;         (swap 把 A 的高低 4 位互換;state ≤ 4,所以等於乘 16)。
;-----------------------------------------------------------------------------
print_state:
    mov   DPTR, #msg_arrow
    lcall puts

    mov   A, state
    swap  A                ; A = state × 16
    mov   DPTR, #name_table
    add   A, DPL           ; DPTR = name_table + 偏移
    mov   DPL, A           ;   DPTR 是 DPH/DPL 兩個 SFR 湊的,
    clr   A                ;   所以 16 位元加法要自己做:
    addc  A, DPH           ;   低位相加產生的進位,用 addc 帶到高位
    mov   DPH, A
    lcall puts

    mov   DPTR, #msg_crlf
    lcall puts
    ret

;-----------------------------------------------------------------------------
; UART(與第 4 課相同:9600 8N1,用 Timer1 當鮑率產生器)
;-----------------------------------------------------------------------------
uart_init:
    mov   SCON,  #0x50     ; 模式1,REN=1
    anl   TMOD,  #0x0F     ; 清 Timer1 的設定位
    orl   TMOD,  #0x20     ; Timer1 模式2(8 位元自動重載)
    orl   PCON,  #0x80     ; SMOD = 1(鮑率加倍)
    orl   CKCON, #0x10     ; T1M = 1(Timer1 吃滿 16MHz)
    anl   T3CON, #0xDF     ; BRCK = 0(鮑率來源選 Timer1)
    mov   TH1,   #0x98     ; → 約 9600 bps
    mov   TL1,   #0x98
    setb  TR1
    ret

; puts:送出 DPTR 指向的字串(以 0 結尾)
puts:
    clr   A
    movc  A, @A+DPTR
    jz    puts_done
    lcall putc
    inc   DPTR
    sjmp  puts
puts_done:
    ret

; putc:送出 A 裡的一個字元
putc:
    mov   SBUF, A
wait_ti:
    jnb   TI, wait_ti
    clr   TI
    ret

;-----------------------------------------------------------------------------
; Timer0 中斷服務程式:每 1ms 一次 ─ 時間基準 + 按鈕去彈跳
;-----------------------------------------------------------------------------
timer0_isr:
    push  acc
    push  psw

    mov   TH0, #0xC1
    mov   TL0, #0x80

    inc   ms_cnt
    mov   A, ms_cnt
    cjne  A, #125, isr_debounce   ; 1ms × 125 = 一個 tick (125ms)
    mov   ms_cnt, #0
    setb  tick_flag

isr_debounce:
    jb    BTN, btn_is_high

btn_is_low:                       ; 現在讀到低電位(按下)
    jnb   btn_stable, deb_clear
    inc   deb
    mov   A, deb
    cjne  A, #DEBOUNCE, isr_done
    mov   deb, #0
    clr   btn_stable
    setb  btn_flag                ; ★ 只在「按下的那一刻」產生事件
    sjmp  isr_done

btn_is_high:                      ; 現在讀到高電位(放開)
    jb    btn_stable, deb_clear
    inc   deb
    mov   A, deb
    cjne  A, #DEBOUNCE, isr_done
    mov   deb, #0
    setb  btn_stable
    sjmp  isr_done

deb_clear:
    mov   deb, #0

isr_done:
    pop   psw
    pop   acc
    reti

;-----------------------------------------------------------------------------
; 資料表
;-----------------------------------------------------------------------------
; 每個狀態停留幾個 tick(1 tick = 125ms)
dur_table:
    .db  40                ; ST_GREEN  最短綠燈 5.0 秒
    .db  24                ; ST_YELLOW 3.0 秒
    .db  64                ; ST_WALK   8.0 秒
    .db  40                ; ST_FLASH  5.0 秒
    .db  16                ; ST_RED    2.0 秒

; 每個狀態的 P1 燈號遮罩
;   bit0 車道紅  bit1 車道黃  bit3 車道綠  bit4 行人綠  bit6 行人紅
out_table:
    .db  0x48              ; ST_GREEN  車道綠 + 行人紅
    .db  0x42              ; ST_YELLOW 車道黃 + 行人紅
    .db  0x11              ; ST_WALK   車道紅 + 行人綠
    .db  0x01              ; ST_FLASH  車道紅(行人綠由程式加上,閃爍)
    .db  0x41              ; ST_RED    車道紅 + 行人紅

;-----------------------------------------------------------------------------
; 狀態名稱字串表
;   ⚠ 每一筆**必須剛好 16 bytes**(字串 + 結尾 0 + 補零),print_state 的
;     swap A(×16)才算得對。改名字時記得重新算補零的個數。
;   註:這裡刻意用英文而非中文 —— 中文一個字佔 3 bytes (UTF-8),
;       4 個字就 12 bytes,補零的算術會變得很容易出錯;英文也不挑終端機編碼。
;-----------------------------------------------------------------------------
name_table:
    .ascii "VEH GREEN"                   ; 9
    .db  0                               ; +1 = 10
    .db  0, 0, 0, 0, 0, 0                ; +6 = 16

    .ascii "VEH YELLOW"                  ; 10
    .db  0                               ; +1 = 11
    .db  0, 0, 0, 0, 0                   ; +5 = 16

    .ascii "PED WALK"                    ; 8
    .db  0                               ; +1 = 9
    .db  0, 0, 0, 0, 0, 0, 0             ; +7 = 16

    .ascii "PED FLASH"                   ; 9
    .db  0                               ; +1 = 10
    .db  0, 0, 0, 0, 0, 0                ; +6 = 16

    .ascii "ALL RED"                     ; 7
    .db  0                               ; +1 = 8
    .db  0, 0, 0, 0, 0, 0, 0, 0          ; +8 = 16

;-----------------------------------------------------------------------------
; 其他字串
;-----------------------------------------------------------------------------
msg_banner:
    .db    0x0D, 0x0A
    .ascii "=== N76E003 Pedestrian Crossing (ASM) ==="
    .db    0x0D, 0x0A
    .ascii "Press P1.5 to request crossing"
    .db    0x0D, 0x0A, 0x00

msg_arrow:
    .ascii "-> "
    .db    0x00

msg_recorded:
    .ascii "BTN: recorded"
    .db    0x0D, 0x0A, 0x00

msg_ignored:
    .ascii "BTN: ignored"
    .db    0x0D, 0x0A, 0x00

msg_crlf:
    .db    0x0D, 0x0A, 0x00
