/*============================================================================
 * 第 4 課:UART 序列埠 ─ 讓晶片對電腦說話  ─  C 語言版
 *============================================================================
 * 學習目標:
 *   1. 設定 UART0,用「序列埠」把文字送到電腦
 *   2. 了解鮑率 (baud rate) 怎麼來的
 *   3. 學會 uart_putc / uart_puts,日後可拿來印除錯訊息
 *
 * 成果:每隔約 1 秒,從序列埠送出一行 "Hello from N76E003! (count=N)"。
 *       在電腦上用序列埠終端機就能看到。
 *
 * 接線(需要一個 USB-TTL 轉接器):
 *   N76E003 P0.6 (TXD) --> USB-TTL 的 RX
 *   N76E003 GND        --> USB-TTL 的 GND
 *   (這一課只示範「送」,所以 RXD 可以先不接)
 *
 * 電腦端設定:鮑率 9600、8 資料位元、無同位、1 停止位元 (9600 8N1)。
 *   Linux 可用:  screen /dev/ttyUSB0 9600
 *            或:  picocom -b 9600 /dev/ttyUSB0
 *
 * 鮑率怎麼算(詳見 docs/05):
 *   用 Timer1 當鮑率產生器,SMOD=1(倍速)、T1M=1(Timer1 吃滿 16MHz):
 *     baud = Fsys / (16 × (256 − TH1))
 *   要 9600 → 256 − TH1 = 16e6 / (16 × 9600) ≈ 104 → TH1 = 152 = 0x98
 *   實際得到約 9615 bps,誤差僅 0.16%,非常可靠。
 *===========================================================================*/

#include "N76E003.h"

void uart_init(void)
{
    /* P0.6(TXD)、P0.7(RXD) 設為準雙向模式(UART 腳位的標準設定)*/
    P0M1 &= ~((1 << 6) | (1 << 7));
    P0M2 &= ~((1 << 6) | (1 << 7));

    SCON  = 0x50;        /* UART0 模式1(8 位元可變鮑率),REN=1 允許接收 */
    TMOD &= ~0xF0;       /* 清除 Timer1 的設定位 */
    TMOD |=  0x20;       /* Timer1 → 模式2(8 位元自動重載),專門當鮑率產生器 */

    PCON  |= 0x80;       /* SMOD = 1:鮑率加倍 */
    CKCON |= 0x10;       /* T1M = 1:Timer1 吃完整 16MHz */
    T3CON &= ~0x20;      /* BRCK = 0:UART0 的鮑率時脈來源選 Timer1 */

    TH1 = 0x98;          /* 重載值 → 約 9600 bps */
    TL1 = 0x98;
    TR1 = 1;             /* 啟動 Timer1 */
}

/* 送出一個字元:把資料丟進 SBUF,然後等硬體送完(TI 變 1)*/
void uart_putc(char c)
{
    SBUF = c;
    while (!TI)          /* 等「傳送完成」旗標 */
        ;
    TI = 0;              /* 旗標要自己清 0,下次才能再用 */
}

/* 送出一個字串(以 '\0' 結尾)*/
void uart_puts(char *s)
{
    while (*s)
        uart_putc(*s++);
}

/* 送出一個 0~65535 的數字(十進位)*/
void uart_put_uint(unsigned int n)
{
    char buf[6];
    signed char i = 0;
    if (n == 0) { uart_putc('0'); return; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (i > 0) uart_putc(buf[--i]);   /* 反過來輸出 */
}

/* 粗略的延時(不精準,只為了隔開每次輸出)*/
void delay_about_1s(void)
{
    volatile unsigned int i, j;
    for (i = 0; i < 1100; i++)
        for (j = 0; j < 120; j++)
            ;
}

void main(void)
{
    unsigned int count = 0;

    uart_init();

    while (1)
    {
        uart_puts("Hello from N76E003! (count=");
        uart_put_uint(count);
        uart_puts(")\r\n");
        count++;
        delay_about_1s();
    }
}
