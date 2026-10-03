/*============================================================================
 * sim.c ─ 第 7 課狀態機的「電腦模擬器」
 *============================================================================
 * 這支程式用電腦的 gcc 編譯,直接執行 c/07-fsm/fsm.c 這份**一模一樣**的
 * 狀態機原始碼,只是把輸入換成腳本、把輸出畫成文字表格。
 *
 * 所以你不需要開發板、不需要 LED、不需要按鈕,就能看清楚:
 *   ‧ 同一個「按鈕」事件,在不同狀態下會有完全不同的結果
 *   ‧ 守衛條件(綠燈最短 5 秒)怎麼擋住太早按的請求,又怎麼把它記住
 *   ‧ 每個狀態對應的燈號與蜂鳴器輸出
 *
 * 執行:  make -C tools/fsm_sim run      (或在專案根目錄 make sim)
 * 教學:  docs/11-狀態機.md
 *===========================================================================*/

#include <stdio.h>
#include <string.h>
#include "fsm.h"

/*---------------------------------------------------------------------------
 * 一個「模擬場景」= 要跑多久 + 什麼時候按按鈕
 *--------------------------------------------------------------------------*/
#define MAX_PRESS  8

typedef struct {
    const char  *title;              /* 場景名稱 */
    const char  *note;               /* 這個場景要看的重點 */
    unsigned int duration;           /* 總共模擬幾個 tick(1 tick = 100ms) */
    unsigned int press[MAX_PRESS];   /* 在第幾個 tick 按下按鈕 */
    unsigned int n_press;            /* 按幾次 */
    unsigned int zoom_from;          /* 逐格列印的起點 tick(0 = 不逐格印) */
    unsigned int zoom_to;            /* 逐格列印的終點 tick */
} scenario_t;

/*---------------------------------------------------------------------------
 * 把輸出快照畫成文字
 *   車道燈:三格,依序是 紅/黃/綠,亮的那格印字母,不亮印 '-'
 *   行人燈:兩格,依序是 紅/綠
 *--------------------------------------------------------------------------*/
static void veh_lights(const fsm_out_t *o, char *buf)
{
    buf[0] = o->veh_red    ? 'R' : '-';
    buf[1] = ' ';
    buf[2] = o->veh_yellow ? 'Y' : '-';
    buf[3] = ' ';
    buf[4] = o->veh_green  ? 'G' : '-';
    buf[5] = '\0';
}

static void ped_lights(const fsm_out_t *o, char *buf)
{
    buf[0] = o->ped_red   ? 'R' : '-';
    buf[1] = ' ';
    buf[2] = o->ped_green ? 'G' : '-';
    buf[3] = '\0';
}

/* 時間軸用的單字母代號 */
static char state_char(fsm_state_t s)
{
    switch (s) {
    case ST_VEH_GREEN:  return 'G';
    case ST_VEH_YELLOW: return 'Y';
    case ST_PED_WALK:   return 'W';
    case ST_PED_FLASH:  return 'F';
    case ST_ALL_RED:    return 'R';
    default:            return '?';
    }
}

/* 給這一行一句「為什麼」的說明 */
static const char *explain(fsm_result_t r, const fsm_t *m)
{
    switch (r) {
    case FSM_RECORDED:
        return "行人請求登記了,但要等綠燈滿 5 秒才會動作";
    case FSM_IGNORED:
        return (m->state == ST_VEH_GREEN)
                 ? "已經登記過,重複按沒有用"
                 : "這個狀態不理按鈕(號誌正在跑完它的流程)";
    case FSM_CHANGED:
        switch (m->state) {
        case ST_VEH_YELLOW: return "條件成立!車道開始黃燈警示";
        case ST_PED_WALK:   return "行人可以走了,慢速「嗶」提示";
        case ST_PED_FLASH:  return "通行時間快到,綠燈開始閃、急促「嗶」";
        case ST_ALL_RED:    return "兩邊全紅,確保路口淨空";
        case ST_VEH_GREEN:  return "一輪結束,回到車道通行";
        default:            return "";
        }
    default:
        return "";
    }
}

/* 印一行紀錄 */
static void print_row(unsigned int tick, const char *ev,
                      fsm_result_t r, const fsm_t *m, const char *why)
{
    fsm_out_t o;
    char veh[8], ped[8];

    fsm_outputs(m, &o);
    veh_lights(&o, veh);
    ped_lights(&o, ped);

    printf("  %5.1fs | %s | %s | %s | %s | %s | %s\n",
           tick * (FSM_TICK_MS / 1000.0),
           ev,
           fsm_result_name(r),
           fsm_state_name(m->state),
           veh, ped,
           o.buzzer ? "嗶" : "- ");

    if (why && why[0])
        printf("         |      |          |          |       |     |    ↳ %s\n", why);
}

static void print_header(void)
{
    printf("   時間  | 事件 | 處理結果 |   狀態   | 車道燈| 行人| 嗶\n");
    printf("  -------+------+----------+----------+-------+-----+----\n");
}

/*---------------------------------------------------------------------------
 * 跑一個場景
 *--------------------------------------------------------------------------*/
static void run_scenario(const scenario_t *sc)
{
    fsm_t        m;
    fsm_result_t r;
    unsigned int t, i, idx;
    char  timeline[256];
    char  marker[256];
    unsigned int tl = 0;          /* timeline 已填入幾格 */

    printf("\n");
    printf("==============================================================================\n");
    printf(" %s\n", sc->title);
    printf("   看點:%s\n", sc->note);
    printf("==============================================================================\n");

    memset(timeline, 0, sizeof(timeline));
    memset(marker, ' ', sizeof(marker));

    fsm_init(&m);
    print_header();
    print_row(0, "開機", FSM_CHANGED, &m, "預設狀態:車子走,行人紅燈");

    for (t = 1; t <= sc->duration; t++)
    {
        /* ---- 輸入①:時間又過了 100ms(來自 Timer0) ---- */
        r = fsm_handle(&m, EV_TICK);

        if (r == FSM_CHANGED)
            print_row(t, "時間", r, &m, explain(r, &m));
        else if (sc->zoom_from && t >= sc->zoom_from && t <= sc->zoom_to)
            print_row(t, "時間", r, &m, "");   /* 逐格觀察閃爍/蜂鳴器 */

        /* ---- 輸入②:這個時間點有人按按鈕嗎? ---- */
        for (i = 0; i < sc->n_press; i++)
        {
            if (sc->press[i] == t)
            {
                r = fsm_handle(&m, EV_BUTTON);
                print_row(t, "按鈕", r, &m, explain(r, &m));
                idx = (t >= 5) ? (t / 5 - 1) : 0;      /* 對齊時間軸的格子 */
                if (idx < sizeof(marker) - 1)
                    marker[idx] = '^';
            }
        }

        /* ---- 每 0.5 秒在時間軸上記一格 ---- */
        if (t % 5 == 0 && tl < sizeof(timeline) - 1)
            timeline[tl++] = state_char(m.state);
    }
    timeline[tl] = '\0';
    marker[tl ? tl : 1] = '\0';

    printf("\n  時間軸(每格 0.5 秒)  G=車道綠 Y=車道黃 W=行人通行 F=行人催促 R=全紅\n");
    printf("    %s\n", timeline);
    if (sc->n_press)
        printf("    %s  (^ = 按鈕按下的時刻)\n", marker);
}

/*---------------------------------------------------------------------------
 * 場景一覽
 *--------------------------------------------------------------------------*/
static const scenario_t scenarios[] = {
    {
        "場景 1:完全沒有人按按鈕",
        "車道綠燈會一直亮著不動 —— 狀態機在「等事件」,不是在「跑流程」",
        150,                    /* 15 秒 */
        {0}, 0,
        0, 0
    },
    {
        "場景 2:綠燈已經亮超過 5 秒才按按鈕",
        "守衛條件已滿足,按下的那一刻就立刻轉移(記下請求 → 馬上狀態轉移)",
        250,                    /* 25 秒 */
        {70}, 1,                /* 第 7.0 秒按 */
        0, 0
    },
    {
        "場景 3:綠燈才亮 1 秒就按按鈕",
        "請求被「記住」而不是丟掉:等綠燈滿 5.0 秒,系統自己動作",
        250,
        {10}, 1,                /* 第 1.0 秒按 */
        0, 0
    },
    {
        "場景 4:行人通行途中一直亂按按鈕",
        "同一個輸入在不同狀態有不同下場:這些按壓全部被忽略,流程不受干擾",
        300,
        {70, 100, 130, 160, 220}, 5,
        0, 0
    },
    {
        "場景 5:放大看「行人催促」的閃爍與急促嗶聲",
        "輸出是狀態(加上相位)的純函式:同一個狀態,燈號依 0.5 秒規律閃動",
        230,
        {50}, 1,
        185, 200                /* 逐格印出 18.5s ~ 20.0s */
    },
};

int main(void)
{
    unsigned int i;

    printf("=============================================================================\n");
    printf(" 第 7 課 狀態機模擬器 ─ 行人穿越號誌路口\n");
    printf("-----------------------------------------------------------------------------\n");
    printf(" 這支程式在電腦上執行 c/07-fsm/fsm.c(與燒進 N76E003 的是同一份原始碼),\n");
    printf(" 用腳本餵入不同的輸入,把輸出畫成表格,讓你不用硬體就看懂狀態機怎麼運作。\n");
    printf("\n");
    printf(" 狀態與停留時間(1 tick = %d ms):\n", FSM_TICK_MS);
    printf("   車道綠燈  等按鈕(至少 %.1f 秒)   車道黃燈  %.1f 秒\n",
           T_VEH_GREEN_MIN * 0.1, T_VEH_YELLOW * 0.1);
    printf("   行人通行  %.1f 秒                  行人催促  %.1f 秒\n",
           T_PED_WALK * 0.1, T_PED_FLASH * 0.1);
    printf("   全紅淨空  %.1f 秒\n", T_ALL_RED * 0.1);
    printf("\n");
    printf(" 燈號欄位讀法: 車道燈「R Y G」 行人燈「R G」,亮=字母、不亮=「-」\n");
    printf("=============================================================================\n");

    for (i = 0; i < sizeof(scenarios) / sizeof(scenarios[0]); i++)
        run_scenario(&scenarios[i]);

    printf("\n");
    printf("=============================================================================\n");
    printf(" 模擬結束。想自己試別的輸入?改 tools/fsm_sim/sim.c 裡的 scenarios[],\n");
    printf(" 加一組 press[] 時間點再執行一次就好 —— 完全不用燒板子。\n");
    printf("=============================================================================\n");
    return 0;
}
