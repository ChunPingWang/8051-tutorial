;=============================================================================
; 第 7 課【精簡版】:狀態機 ─ 行人穿越號誌  ─  組合語言版
;=============================================================================
; 與 C 版 (c/07-fsm) 的狀態機規則完全相同,但刻意**砍掉 UART 與蜂鳴器**,
; 只留 5 顆 LED + 1 顆按鈕,好讓你專心看三件事:
;
;   ① C 的 switch (state)  →  組語的「跳躍表 (jump table)」
;   ② C 的 fsm_outputs()   →  組語的「查表 (movc)」:一個位元組就是一組燈號
;   ③ C 的 ticks++ 加計時比較  →  組語的「倒數到 0」:省掉 16 位元比較
;
; 想看功能完整(含 UART 印狀態轉移紀錄 + 蜂鳴器)的組語版 → 同資料夾 main_full.asm
;
;-----------------------------------------------------------------------------
; 兩個讓組語變簡單的設計決定(與 C 版的差異,原因見 docs/11 §11.12):
;
;   ‧ tick 改成 125ms(C 版是 100ms)
;     → 最長的狀態 8 秒 = 64 個 tick,**所有計時都塞得進一個位元組**,
;       16 位元加減與比較全部消失。整台狀態機只佔 2 bytes RAM + 4 個位元!
;     → 而且 0.5 秒 = 4 個 tick(2 的次方),閃爍相位可以直接測位元,不必除法。
;
;   ‧ 計時改成「倒數」而非「累加」
;     → C 版要寫 if (ticks >= 50);組語只要 jnz,一條指令就判斷完。
;
; 各狀態時間(單位:125ms 的 tick):
;   車道綠燈 最短 40 (5.0s) / 車道黃燈 24 (3.0s) / 行人通行 64 (8.0s)
;   行人催促 40 (5.0s)      / 全紅淨空 16 (2.0s)
;
; 接線(與 C 版相同,每顆 LED 都要各自串 220Ω):
;   車道紅 P1.0  車道黃 P1.1  車道綠 P1.3  行人綠 P1.4  行人紅 P1.6
;   行人按鈕 P1.5 ----[ 按鈕 ]---- GND      (內部弱上拉,按下讀到 0)
;=============================================================================

    .module fsm

    ; ---- SFR ----
    P1    = 0x90
    P1M1  = 0xB3
    P1M2  = 0xB4
    TMOD  = 0x89
    CKCON = 0x8E
    TL0   = 0x8A
    TH0   = 0x8C
    ; ---- 可定址的位元 ----
    BTN   = 0x95          ; P1.5 行人按鈕
    TR0   = 0x8C          ; TCON.4 啟動 Timer0
    ET0   = 0xA9          ; IE.1  Timer0 中斷致能
    EA    = 0xAF          ; IE.7  總中斷

    ; ---- 狀態編號(同時就是各張表的索引)----
    ST_GREEN  = 0         ; 車道綠燈
    ST_YELLOW = 1         ; 車道黃燈
    ST_WALK   = 2         ; 行人通行
    ST_FLASH  = 3         ; 行人催促(綠燈閃爍)
    ST_RED    = 4         ; 全紅淨空

    DEBOUNCE  = 20        ; 按鈕要連續穩定 20ms 才承認

    ; ---- 自己配置的 RAM 變數(內部 RAM 一般區)----
    state   = 0x30        ; 目前狀態 (0~4)
    ticks   = 0x31        ; 本狀態還剩幾個 tick(倒數)
    ms_cnt  = 0x32        ; 1ms 累積到 125 = 一個 tick
    deb     = 0x33        ; 去彈跳計數

    ; ---- 自己配置的位元變數(全部住在 RAM 的 0x20 這一個位元組裡)----
    request    = 0x00     ; 有待處理的行人請求
    btn_stable = 0x01     ; 已確認的按鈕電位 (1 = 放開)
    tick_flag  = 0x02     ; 又過了一個 tick
    btn_flag   = 0x03     ; 偵測到一次「按下」

;-----------------------------------------------------------------------------
; 向量區
;-----------------------------------------------------------------------------
    .area CODE (ABS,CODE)
    .org  0x0000
    ljmp  start            ; 重置向量
    .org  0x000B
    ljmp  timer0_isr       ; Timer0 中斷向量

;-----------------------------------------------------------------------------
; 開機初始化
;-----------------------------------------------------------------------------
start:
    ; P1 腳位模式:bit0,1,3,4,6 → 推挽輸出(接 LED)
    ;              bit2,5,7     → 準雙向(其中 P1.5 是按鈕)
    mov   P1M1, #0x00
    mov   P1M2, #0x5B      ; 0101 1011
    mov   P1,   #0x20      ; LED 全滅;P1.5 寫 1(準雙向腳讀取前必須先寫 1)

    ; 位元變數與計數器歸零
    clr   request
    setb  btn_stable       ; 開機時按鈕是「放開」狀態
    clr   tick_flag
    clr   btn_flag
    mov   ms_cnt, #0
    mov   deb,    #0

    ; 進入初始狀態。借用 enter_state 幫我們順便載入倒數時間
    mov   A, #ST_GREEN
    lcall enter_state
    lcall update_outputs

    ; Timer0:模式1(16 位元)、吃滿 16MHz、每 1ms 溢位(算法同第 3 課)
    mov   TMOD,  #0x01
    orl   CKCON, #0x08     ; T0M = 1
    mov   TH0,   #0xC1     ; 65536 - 16000 = 0xC180
    mov   TL0,   #0x80
    setb  ET0
    setb  EA
    setb  TR0

;-----------------------------------------------------------------------------
; 主迴圈:和 C 版一樣短,而且完全沒有 delay
;-----------------------------------------------------------------------------
main_loop:
    jnb   btn_flag, chk_tick
    clr   btn_flag         ; 旗子要自己清掉
    lcall ev_button        ;   (小技巧:JBC btn_flag,xx 可一條指令完成「測試+清除」)
chk_tick:
    jnb   tick_flag, main_loop
    clr   tick_flag
    lcall ev_tick
    lcall update_outputs   ; 每個 tick 都更新輸出,閃爍才會動
    sjmp  main_loop

;-----------------------------------------------------------------------------
; 事件:行人按下按鈕
;   只有「車道綠燈」會理它,其餘狀態一概忽略(對應 C 版的 FSM_IGNORED)。
;   因為 ST_GREEN = 0,所以「A 非 0」就是「不在車道綠燈」→ 一行 jnz 搞定。
;-----------------------------------------------------------------------------
ev_button:
    mov   A, state
    jnz   eb_done
    setb  request          ; 記住這個請求(即使現在還不能動作)
eb_done:
    ret

;-----------------------------------------------------------------------------
; 事件:時間過了一個 tick
;-----------------------------------------------------------------------------
ev_tick:
    mov   A, ticks
    jz    dispatch         ; 已經倒數到 0 就別再減,免得繞回 255
    dec   ticks

;-----------------------------------------------------------------------------
; 跳躍表:這就是 C 的 switch (state)
;   state × 2,因為表裡每一筆 ajmp 佔 2 bytes;再用 jmp @A+DPTR 跳進去。
;   ⚠ ajmp 只能跳到「同一個 2KB 分頁」內。本程式總共不到 1KB,所以安全;
;     程式變大時要改用 ljmp(每筆 3 bytes,索引就要 ×3)。
;-----------------------------------------------------------------------------
dispatch:
    mov   A, state
    rl    A                ; ×2(state 一定小於 128,rl 就等於乘 2)
    mov   DPTR, #jump_table
    jmp   @A+DPTR

jump_table:
    ajmp  st_green
    ajmp  st_yellow
    ajmp  st_walk
    ajmp  st_flash
    ajmp  st_red

;-----------------------------------------------------------------------------
; 各狀態的轉移規則(對照 C 版 fsm_handle() 裡的每個 case)
;-----------------------------------------------------------------------------
st_green:
    mov   A, ticks
    jnz   fsm_ret          ; 守衛條件①:最短綠燈時間還沒到
    jnb   request, fsm_ret ; 守衛條件②:沒有人按過按鈕 → 可以一直等下去
    clr   request          ; 請求被服務了,清掉
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
    ; 直接往下落到 enter_state(省一條 ajmp)

;-----------------------------------------------------------------------------
; 進入新狀態(A = 新狀態編號)
;   這是整支程式唯一會改 state 的地方,對應 C 版的
;       if (next != m->state) { m->state = next; m->ticks = 0; }
;   「進入動作」只寫一次:從 dur_table 查出這個狀態要待幾個 tick。
;
;   註:這裡的 ret 會回到 main_loop 的 lcall ev_tick ——
;       因為 jmp @A+DPTR 不動堆疊,ev_tick 的返回位址還好端端地留在上面。
;-----------------------------------------------------------------------------
enter_state:
    mov   state, A
    mov   DPTR, #dur_table
    movc  A, @A+DPTR       ; A = dur_table[新狀態]
    mov   ticks, A
fsm_ret:
    ret

;-----------------------------------------------------------------------------
; 輸出:由「狀態」決定燈號 ─ 對應 C 版的 fsm_outputs()
;   C 版用一個 switch 逐欄填 struct;組語直接查一張「燈號遮罩表」,
;   一個位元組就代表 P1 上五顆 LED 的開關狀態。這裡組語反而比 C 更乾淨。
;-----------------------------------------------------------------------------
update_outputs:
    mov   A, state
    mov   DPTR, #out_table
    movc  A, @A+DPTR       ; A = 這個狀態的燈號遮罩
    mov   R7, A

    ; 「行人催促」要讓行人綠燈每 0.5 秒閃一次。
    ; 0.5 秒 = 4 個 tick,所以倒數計數器的 bit2 剛好就是閃爍相位 → 測一個位元就好。
    mov   A, state
    cjne  A, #ST_FLASH, uo_write
    mov   A, ticks
    anl   A, #0x04
    jz    uo_write         ; 相位 = 暗 → 不加行人綠燈
    mov   A, R7
    orl   A, #0x10         ; 相位 = 亮 → 點亮行人綠燈 (P1.4)
    mov   R7, A

uo_write:
    mov   A, R7
    orl   A, #0x20         ; P1.5(按鈕腳)務必維持 1,否則準雙向腳讀不到按下
    mov   P1, A            ; 本課只用 P1,沒用到的腳一律輸出 0
    ret

;-----------------------------------------------------------------------------
; Timer0 中斷服務程式:每 1ms 進來一次
;   只做兩件事:① 累積成 125ms 的 tick  ② 按鈕去彈跳
;   真正的決策留給主迴圈 —— ISR 要短。
;-----------------------------------------------------------------------------
timer0_isr:
    push  acc
    push  psw

    mov   TH0, #0xC1       ; 重新裝填預載值(模式1 不會自動重載)
    mov   TL0, #0x80

    ; ① 1ms × 125 = 一個 tick
    inc   ms_cnt
    mov   A, ms_cnt
    cjne  A, #125, isr_debounce
    mov   ms_cnt, #0
    setb  tick_flag

    ; ② 去彈跳:要求電位連續 20ms 都是新值,才承認按鈕真的變了
isr_debounce:
    jb    BTN, btn_is_high

btn_is_low:                ; 現在讀到低電位(按下)
    jnb   btn_stable, deb_clear   ; 已確認是「按下」→ 沒有變化
    inc   deb
    mov   A, deb
    cjne  A, #DEBOUNCE, isr_done
    mov   deb, #0
    clr   btn_stable
    setb  btn_flag         ; ★ 只在「按下的那一刻」產生事件
    sjmp  isr_done

btn_is_high:               ; 現在讀到高電位(放開)
    jb    btn_stable, deb_clear   ; 已確認是「放開」→ 沒有變化
    inc   deb
    mov   A, deb
    cjne  A, #DEBOUNCE, isr_done
    mov   deb, #0
    setb  btn_stable       ; 放開不產生事件,只更新「已確認電位」
    sjmp  isr_done

deb_clear:
    mov   deb, #0          ; 電位又跳回去了,重新計數

isr_done:
    pop   psw
    pop   acc
    reti

;-----------------------------------------------------------------------------
; 資料表(放在程式記憶體 CODE,用 movc 讀)
;-----------------------------------------------------------------------------
; 每個狀態要停留幾個 tick(1 tick = 125ms)
dur_table:
    .db  40                ; ST_GREEN  最短綠燈 5.0 秒
    .db  24                ; ST_YELLOW 3.0 秒
    .db  64                ; ST_WALK   8.0 秒
    .db  40                ; ST_FLASH  5.0 秒
    .db  16                ; ST_RED    2.0 秒

; 每個狀態的 P1 燈號遮罩
;   bit0 = P1.0 車道紅   bit1 = P1.1 車道黃   bit3 = P1.3 車道綠
;   bit4 = P1.4 行人綠   bit6 = P1.6 行人紅
out_table:
    .db  0x48              ; ST_GREEN  車道綠 + 行人紅
    .db  0x42              ; ST_YELLOW 車道黃 + 行人紅
    .db  0x11              ; ST_WALK   車道紅 + 行人綠
    .db  0x01              ; ST_FLASH  車道紅(行人綠由程式加上,閃爍)
    .db  0x41              ; ST_RED    車道紅 + 行人紅
