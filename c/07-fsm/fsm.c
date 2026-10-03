/*============================================================================
 * fsm.c ─ 「行人穿越號誌」狀態機的實作(純邏輯,不含任何硬體相依)
 *============================================================================
 * 情境:一個馬路路口。平常車道是綠燈,行人要過馬路時按一下按鈕,
 *       號誌會在「車道綠燈至少亮滿 5 秒」之後,才依序切換:
 *
 *   車道綠燈 ──(有人按 + 綠燈滿 5 秒)──> 車道黃燈 ─(3s)─> 行人通行
 *       ^                                                        │(8s)
 *       │                                                        v
 *   全紅淨空 <───────(5s)──────── 行人催促(行人綠燈閃爍)<───────┘
 *       └──(2s)──> 回到車道綠燈
 *
 * 三個值得細看的設計(docs/11-狀態機.md 有完整解說):
 *   ① 守衛條件 (guard):綠燈不滿 5 秒,按鈕不會立刻生效(避免車流被一直打斷)
 *   ② 請求記憶:太早按的請求不會被丟掉,會「記住」,等條件滿足才動作
 *   ③ 事件忽略:行人正在通行時再怎麼按都沒用 —— 同一個輸入,不同狀態不同反應
 *===========================================================================*/

#include "fsm.h"

/*----------------------------------------------------------------------------
 * 初始化:一律回到最安全的狀態
 *---------------------------------------------------------------------------*/
void fsm_init(fsm_t *m)
{
    m->state   = ST_VEH_GREEN;   /* 預設讓車子走 */
    m->ticks   = 0;              /* 進入狀態的計時歸零 */
    m->request = 0;              /* 沒有待處理的行人請求 */
}

/*----------------------------------------------------------------------------
 * 核心:事件處理(狀態轉移函式)
 *---------------------------------------------------------------------------*
 * 整個狀態機只有這一個地方會改變 state。所有「什麼時候換到哪」的規則
 * 都集中在這個 switch 裡 —— 這就是狀態機好維護的原因:規則有單一出處。
 *---------------------------------------------------------------------------*/
fsm_result_t fsm_handle(fsm_t *m, fsm_event_t ev)
{
    fsm_state_t  next = m->state;          /* 預設「留在原地」 */
    fsm_result_t res  = FSM_NO_CHANGE;

    /* 時間事件:先把「待在這個狀態多久了」加上去,再判斷時間到了沒。
       加上限是為了防止車道綠燈一直沒人按時,計數器一路加到溢位。 */
    if (ev == EV_TICK && m->ticks < FSM_TICKS_MAX)
        m->ticks++;

    switch (m->state)
    {
    /*------------------------------------------------------------------
     * 車道綠燈:唯一會「等外界輸入」的狀態,可以待很久
     *-----------------------------------------------------------------*/
    case ST_VEH_GREEN:
        if (ev == EV_BUTTON)
        {
            if (m->request)
                res = FSM_IGNORED;         /* 已經登記過了,再按也沒用 */
            else
            {
                m->request = 1;            /* ② 記住這個請求 */
                res = FSM_RECORDED;
            }
        }
        /* ① 守衛條件:兩個條件要同時成立才轉移 */
        if (m->request && m->ticks >= T_VEH_GREEN_MIN)
        {
            m->request = 0;                /* 請求被服務了,清掉 */
            next = ST_VEH_YELLOW;
        }
        break;

    /*------------------------------------------------------------------
     * 以下都是「純計時」狀態:時間到就往下走,按鈕一概不理 (③)
     *-----------------------------------------------------------------*/
    case ST_VEH_YELLOW:
        if (ev == EV_BUTTON)      res = FSM_IGNORED;
        if (m->ticks >= T_VEH_YELLOW)  next = ST_PED_WALK;
        break;

    case ST_PED_WALK:
        if (ev == EV_BUTTON)      res = FSM_IGNORED;   /* 你已經在過馬路了 */
        if (m->ticks >= T_PED_WALK)    next = ST_PED_FLASH;
        break;

    case ST_PED_FLASH:
        if (ev == EV_BUTTON)      res = FSM_IGNORED;
        if (m->ticks >= T_PED_FLASH)   next = ST_ALL_RED;
        break;

    case ST_ALL_RED:
        if (ev == EV_BUTTON)      res = FSM_IGNORED;
        if (m->ticks >= T_ALL_RED)     next = ST_VEH_GREEN;
        break;

    /*------------------------------------------------------------------
     * 保險箱:萬一 state 被雜訊/程式錯誤弄成不存在的值,回到安全狀態
     * (真實產品一定要寫這段,不然一次異常可能讓號誌永遠卡住)
     *-----------------------------------------------------------------*/
    default:
        next = ST_VEH_GREEN;
        break;
    }

    /* 真的要換狀態了 → 這裡是唯一的「進入動作 (entry action)」:計時歸零 */
    if (next != m->state)
    {
        m->state = next;
        m->ticks = 0;
        res = FSM_CHANGED;
    }

    return res;
}

/*----------------------------------------------------------------------------
 * 輸出:由「狀態」決定燈號
 *---------------------------------------------------------------------------*
 * 這是純函式 —— 給它同一個狀態,永遠得到同一組輸出。
 * 好處:燈號錯了,你只要看這個函式;狀態錯了,你只要看 fsm_handle()。
 *       責任分得乾乾淨淨,不會在幾百行程式裡到處找「是誰把燈關掉的」。
 *
 * 註:下面用 % 取餘數來做閃爍相位,好讀但 8051 做 16 位元除法要呼叫
 *     函式庫、比較慢。若要省空間/時間,可以另外用一個 0~9 的計數器代替。
 *---------------------------------------------------------------------------*/
void fsm_outputs(const fsm_t *m, fsm_out_t *o)
{
    /* 先全部關掉,再依狀態只打開該亮的 —— 這樣絕不會忘記關上一個狀態的燈 */
    o->veh_red    = 0;
    o->veh_yellow = 0;
    o->veh_green  = 0;
    o->ped_red    = 0;
    o->ped_green  = 0;
    o->buzzer     = 0;

    switch (m->state)
    {
    case ST_VEH_GREEN:
        o->veh_green = 1;
        o->ped_red   = 1;
        break;

    case ST_VEH_YELLOW:
        o->veh_yellow = 1;
        o->ped_red    = 1;
        break;

    case ST_PED_WALK:
        o->veh_red   = 1;
        o->ped_green = 1;
        /* 慢「嗶」:亮 0.5 秒、停 0.5 秒,提示視障者可以通行 */
        o->buzzer = (unsigned char)((m->ticks % 10) < 5);
        break;

    case ST_PED_FLASH:
        o->veh_red = 1;
        /* 行人綠燈 0.5 秒閃一次:快走! */
        o->ped_green = (unsigned char)((m->ticks % 10) < 5);
        /* 急促「嗶」:0.2 秒一次 */
        o->buzzer    = (unsigned char)((m->ticks % 4) < 2);
        break;

    case ST_ALL_RED:
    default:
        o->veh_red = 1;
        o->ped_red = 1;
        break;
    }
}

/*----------------------------------------------------------------------------
 * 給人看的名字
 *---------------------------------------------------------------------------*
 * 每個名字都刻意用「4 個中文字」,印表格時寬度才會整齊。
 *---------------------------------------------------------------------------*/
const char *fsm_state_name(fsm_state_t s)
{
    switch (s)
    {
    case ST_VEH_GREEN:  return "車道綠燈";
    case ST_VEH_YELLOW: return "車道黃燈";
    case ST_PED_WALK:   return "行人通行";
    case ST_PED_FLASH:  return "行人催促";
    case ST_ALL_RED:    return "全紅淨空";
    default:            return "未知狀態";
    }
}

const char *fsm_result_name(fsm_result_t r)
{
    switch (r)
    {
    case FSM_NO_CHANGE: return "維持原狀";
    case FSM_CHANGED:   return "狀態轉移";
    case FSM_RECORDED:  return "記下請求";
    case FSM_IGNORED:   return "忽略事件";
    default:            return "未知結果";
    }
}
