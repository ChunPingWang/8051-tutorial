/*============================================================================
 * test_logic.c ─ logic.c 的單元測試(在電腦上用 gcc 執行)
 *============================================================================
 * 這是最簡單的單元測試寫法:給一組「已知輸入 → 期望輸出」,用 assert 檢查。
 * 全部通過才印出成功訊息;只要有一項不符,程式會當場中止並指出哪一行失敗。
 *
 * 執行: make test   (或 gcc -o test_logic test_logic.c logic.c && ./test_logic)
 *===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "logic.h"

static int checks = 0;

static void expect_dec(unsigned int n, const char *expected)
{
    char out[8];
    uint_to_dec(n, out);
    printf("  uint_to_dec(%u) = \"%s\"  (期望 \"%s\")\n", n, out, expected);
    assert(strcmp(out, expected) == 0);
    checks++;
}

static void expect_mv(unsigned int adc, unsigned int vdd, unsigned int expected)
{
    unsigned int mv = adc_to_mv(adc, vdd);
    printf("  adc_to_mv(%u, %u) = %u mV  (期望 %u)\n", adc, vdd, mv, expected);
    assert(mv == expected);
    checks++;
}

int main(void)
{
    printf("===== 主機端單元測試 =====\n");

    /* uint_to_dec:邊界與一般值 */
    expect_dec(0, "0");
    expect_dec(7, "7");
    expect_dec(42, "42");
    expect_dec(123, "123");
    expect_dec(4095, "4095");       /* ADC 最大值 */
    expect_dec(65535, "65535");     /* unsigned int 最大值 */

    /* adc_to_mv:VDD = 3300mV */
    expect_mv(0, 3300, 0);          /* 0V */
    expect_mv(4095, 3300, 3300);    /* 滿刻度 = VDD */
    expect_mv(2048, 3300, 1650);    /* 約一半 ≈ 1.65V */
    expect_mv(1241, 3300, 1000);    /* 1.0V 附近 */

    printf("-----------------------------------------\n");
    printf("\xE2\x9C\x85 全部 %d 項測試通過\n", checks);
    return 0;
}
