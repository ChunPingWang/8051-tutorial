/*============================================================================
 * test_fsm.c ─ 第 7 課狀態機的單元測試(在電腦上用 gcc 執行)
 *============================================================================
 * 狀態機最棒的地方:它是純邏輯,所以可以「完整」測試。
 * 這支測試不需要開發板、不需要 LED,幾毫秒就跑完,卻涵蓋了:
 *
 *   ① 初始狀態是否正確
 *   ② 沒有輸入時會不會亂跑(車道綠燈應該永遠等下去)
 *   ③ 守衛條件:太早按按鈕,是否「記住」而不是立刻動作
 *   ④ 守衛條件:已滿最短綠燈時間時,按下是否立刻生效
 *   ⑤ 每個狀態的停留時間是否精確符合設定
 *   ⑥ 事件忽略:不該理按鈕的狀態有沒有乖乖忽略
 *   ⑦ 安全不變式:掃過整個循環,車道綠燈與行人綠燈「絕不可能同時亮」
 *   ⑧ 異常復原:state 被亂改成不存在的值時,會不會回到安全狀態
 *
 * 執行:  make -C tools/host_test test
 *===========================================================================*/

#include <stdio.h>
#include <assert.h>
#include "fsm.h"

static int checks = 0;

#define CHECK(cond, msg)  do {                                   \
        printf("  %-56s %s\n", (msg), (cond) ? "OK" : "失敗");   \
        assert(cond);                                            \
        checks++;                                                \
    } while (0)

/* 餵 n 個時間事件進去 */
static void feed_ticks(fsm_t *m, unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
        (void)fsm_handle(m, EV_TICK);
}

/*----------------------------------------------------------------------------
 * ① 初始狀態
 *---------------------------------------------------------------------------*/
static void test_init(void)
{
    fsm_t m;
    printf("\n[1] 初始狀態\n");
    fsm_init(&m);
    CHECK(m.state == ST_VEH_GREEN, "開機後應該在「車道綠燈」");
    CHECK(m.ticks == 0,            "開機後計時應歸零");
    CHECK(m.request == 0,          "開機後不應有待處理的行人請求");
}

/*----------------------------------------------------------------------------
 * ② 沒有人按按鈕 → 永遠停在車道綠燈
 *---------------------------------------------------------------------------*/
static void test_idle_forever(void)
{
    fsm_t m;
    printf("\n[2] 沒有輸入時不會自己亂跑\n");
    fsm_init(&m);
    feed_ticks(&m, 3000);          /* 300 秒 */
    CHECK(m.state == ST_VEH_GREEN, "空等 300 秒後仍應停在「車道綠燈」");
}

/*----------------------------------------------------------------------------
 * ③ 太早按按鈕:請求被記住,等滿最短綠燈時間才轉移
 *---------------------------------------------------------------------------*/
static void test_guard_early_press(void)
{
    fsm_t m;
    fsm_result_t r;
    printf("\n[3] 守衛條件:綠燈第 1.0 秒就按按鈕\n");
    fsm_init(&m);
    feed_ticks(&m, 10);                       /* 1.0 秒 */

    r = fsm_handle(&m, EV_BUTTON);
    CHECK(r == FSM_RECORDED,       "按鈕應回報「記下請求」");
    CHECK(m.state == ST_VEH_GREEN, "此時還不該轉移(未滿最短綠燈時間)");
    CHECK(m.request == 1,          "請求應該被記住");

    r = fsm_handle(&m, EV_BUTTON);
    CHECK(r == FSM_IGNORED,        "已登記後再按,應回報「忽略事件」");

    feed_ticks(&m, T_VEH_GREEN_MIN - 10 - 1); /* 再走到第 4.9 秒 */
    CHECK(m.state == ST_VEH_GREEN, "第 4.9 秒時仍應是「車道綠燈」");

    (void)fsm_handle(&m, EV_TICK);            /* 第 5.0 秒 */
    CHECK(m.state == ST_VEH_YELLOW, "剛好滿 5.0 秒應轉到「車道黃燈」");
    CHECK(m.request == 0,           "請求被服務後應清除");
    CHECK(m.ticks == 0,             "進入新狀態時計時應歸零");
}

/*----------------------------------------------------------------------------
 * ④ 已滿最短綠燈時間才按 → 當下立刻轉移
 *---------------------------------------------------------------------------*/
static void test_guard_late_press(void)
{
    fsm_t m;
    fsm_result_t r;
    printf("\n[4] 守衛條件:綠燈已滿 7.0 秒才按按鈕\n");
    fsm_init(&m);
    feed_ticks(&m, 70);
    r = fsm_handle(&m, EV_BUTTON);
    CHECK(r == FSM_CHANGED,         "按鈕應回報「狀態轉移」");
    CHECK(m.state == ST_VEH_YELLOW, "應立刻轉到「車道黃燈」");
}

/*----------------------------------------------------------------------------
 * ⑤ 一整輪的時間是否精確
 *---------------------------------------------------------------------------*/
static void test_full_cycle_timing(void)
{
    fsm_t m;
    printf("\n[5] 一整輪各狀態的停留時間\n");
    fsm_init(&m);
    feed_ticks(&m, T_VEH_GREEN_MIN);
    (void)fsm_handle(&m, EV_BUTTON);
    CHECK(m.state == ST_VEH_YELLOW, "進入「車道黃燈」");

    feed_ticks(&m, T_VEH_YELLOW - 1);
    CHECK(m.state == ST_VEH_YELLOW, "黃燈差 1 格時仍在黃燈");
    (void)fsm_handle(&m, EV_TICK);
    CHECK(m.state == ST_PED_WALK,   "黃燈剛好 3.0 秒後進入「行人通行」");

    feed_ticks(&m, T_PED_WALK - 1);
    CHECK(m.state == ST_PED_WALK,   "通行差 1 格時仍在通行");
    (void)fsm_handle(&m, EV_TICK);
    CHECK(m.state == ST_PED_FLASH,  "通行剛好 8.0 秒後進入「行人催促」");

    feed_ticks(&m, T_PED_FLASH - 1);
    CHECK(m.state == ST_PED_FLASH,  "催促差 1 格時仍在催促");
    (void)fsm_handle(&m, EV_TICK);
    CHECK(m.state == ST_ALL_RED,    "催促剛好 5.0 秒後進入「全紅淨空」");

    feed_ticks(&m, T_ALL_RED - 1);
    CHECK(m.state == ST_ALL_RED,    "全紅差 1 格時仍在全紅");
    (void)fsm_handle(&m, EV_TICK);
    CHECK(m.state == ST_VEH_GREEN,  "全紅剛好 2.0 秒後回到「車道綠燈」");
}

/*----------------------------------------------------------------------------
 * ⑥ 不該理按鈕的狀態要忽略它
 *---------------------------------------------------------------------------*/
static void test_button_ignored(void)
{
    fsm_t m;
    fsm_result_t r;
    printf("\n[6] 行人通行/催促/全紅時,按鈕應被忽略\n");
    fsm_init(&m);
    feed_ticks(&m, T_VEH_GREEN_MIN);
    (void)fsm_handle(&m, EV_BUTTON);          /* → 黃燈 */

    r = fsm_handle(&m, EV_BUTTON);
    CHECK(r == FSM_IGNORED && m.state == ST_VEH_YELLOW, "黃燈時按鈕被忽略");

    feed_ticks(&m, T_VEH_YELLOW);             /* → 行人通行 */
    r = fsm_handle(&m, EV_BUTTON);
    CHECK(r == FSM_IGNORED && m.state == ST_PED_WALK,   "通行時按鈕被忽略");

    feed_ticks(&m, T_PED_WALK);               /* → 行人催促 */
    r = fsm_handle(&m, EV_BUTTON);
    CHECK(r == FSM_IGNORED && m.state == ST_PED_FLASH,  "催促時按鈕被忽略");

    feed_ticks(&m, T_PED_FLASH);              /* → 全紅淨空 */
    r = fsm_handle(&m, EV_BUTTON);
    CHECK(r == FSM_IGNORED && m.state == ST_ALL_RED,    "全紅時按鈕被忽略");
}

/*----------------------------------------------------------------------------
 * ⑦ 安全不變式(invariant)─ 這是最重要的一項測試
 *---------------------------------------------------------------------------*
 * 「不變式」是指:不管走到哪個狀態、哪個時間點,都必須成立的規則。
 * 號誌最致命的錯誤就是「兩邊同時綠燈」,所以我們把整輪循環掃一遍,
 * 每一格都檢查一次。這種測試在真實專案裡能救命。
 *---------------------------------------------------------------------------*/
static void test_safety_invariants(void)
{
    fsm_t m;
    fsm_out_t o;
    unsigned int t;
    int both_green = 0;
    int veh_not_one = 0;

    printf("\n[7] 安全不變式:掃過整輪循環逐格檢查\n");
    fsm_init(&m);

    for (t = 0; t < 1000; t++)
    {
        if (t == 60 || t == 500)              /* 中途按幾次按鈕增加變化 */
            (void)fsm_handle(&m, EV_BUTTON);
        (void)fsm_handle(&m, EV_TICK);

        fsm_outputs(&m, &o);

        /* 不變式 1:車道綠燈與行人綠燈不可同時亮 */
        if (o.veh_green && o.ped_green)
            both_green = 1;

        /* 不變式 2:車道的紅/黃/綠,任何時刻「恰好」有一盞亮 */
        if (o.veh_red + o.veh_yellow + o.veh_green != 1)
            veh_not_one = 1;
    }

    CHECK(both_green == 0,  "1000 格內,車道綠與行人綠從未同時亮");
    CHECK(veh_not_one == 0, "1000 格內,車道燈恆為「恰好一盞亮」");
}

/*----------------------------------------------------------------------------
 * ⑧ 異常復原:state 被弄壞時要回到安全狀態
 *---------------------------------------------------------------------------*/
static void test_recover_from_bad_state(void)
{
    fsm_t m;
    printf("\n[8] 異常復原\n");
    fsm_init(&m);
    m.state = (fsm_state_t)99;                /* 故意弄壞 */
    (void)fsm_handle(&m, EV_TICK);
    CHECK(m.state == ST_VEH_GREEN, "遇到不存在的狀態應回到「車道綠燈」");
}

int main(void)
{
    printf("===== 狀態機單元測試(第 7 課)=====\n");

    test_init();
    test_idle_forever();
    test_guard_early_press();
    test_guard_late_press();
    test_full_cycle_timing();
    test_button_ignored();
    test_safety_invariants();
    test_recover_from_bad_state();

    printf("-----------------------------------------\n");
    printf("\xE2\x9C\x85 全部 %d 項測試通過\n", checks);
    return 0;
}
