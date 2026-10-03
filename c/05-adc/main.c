/*============================================================================
 * 第 5 課:ADC 類比輸入 ─ 讀可變電阻的電壓  ─  C 語言版
 *============================================================================
 * 學習目標:
 *   1. 認識 ADC(類比轉數位):把連續的電壓變成數字
 *   2. 設定 N76E003 的 ADC,讀取 AIN0(P1.7)腳上的電壓
 *   3. 把讀到的值透過 UART(第 4 課)印到電腦上觀察
 *
 * 成果:轉動可變電阻,電腦上看到的數字會跟著在 0 ~ 4095 之間變化。
 *       (N76E003 的 ADC 是 12 位元,所以最大值是 2^12 − 1 = 4095)
 *
 * 接線:
 *   可變電阻(電位器)有三支腳:
 *     一端 ──► VDD(3.3V)
 *     另一端 ──► GND
 *     中間(可動端)──► P1.7 (AIN0)
 *   UART:P0.6 (TXD) ──► USB-TTL 的 RX,GND 共接,電腦端 9600 8N1。
 *
 * 原理:ADC 把 0V~VDD 的電壓,對應成 0~4095 的整數。
 *       電壓 ≈ (ADC值 / 4095) × VDD。
 *===========================================================================*/

#include "N76E003.h"

/* ---------- UART(與第 4 課相同)---------- */
void uart_init(void)
{
    P0M1 &= ~((1 << 6) | (1 << 7));
    P0M2 &= ~((1 << 6) | (1 << 7));
    SCON  = 0x50;
    TMOD &= ~0xF0;
    TMOD |=  0x20;
    PCON  |= 0x80;
    CKCON |= 0x10;
    T3CON &= ~0x20;
    TH1 = 0x98;
    TL1 = 0x98;
    TR1 = 1;
}
void uart_putc(char c) { SBUF = c; while (!TI) ; TI = 0; }
void uart_puts(char *s) { while (*s) uart_putc(*s++); }
void uart_put_uint(unsigned int n)
{
    char buf[6];
    signed char i = 0;
    if (n == 0) { uart_putc('0'); return; }
    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (i > 0) uart_putc(buf[--i]);
}

/* ---------- ADC ---------- */
void adc_init(void)
{
    /* P1.7 設為「僅輸入」模式(類比腳要高阻抗):P1M1.7=1、P1M2.7=0 */
    P1M1 |=  (1 << 7);
    P1M2 &= ~(1 << 7);

    ADCCON0 &= 0xF0;      /* 低 4 位 = 通道選擇,清為 0 → 選 AIN0 */
    AINDIDS  = 0x00;      /* 先全部清除 */
    AINDIDS |= 0x01;      /* 禁用 P1.7 的「數位輸入」(當類比腳用)*/
    ADCCON1 |= 0x01;      /* ADCEN = 1:開啟 ADC 電路 */
}

/* 讀一次 ADC,回傳 0~4095 */
unsigned int adc_read(void)
{
    ADCF = 0;                        /* 清除「轉換完成」旗標 */
    ADCS = 1;                        /* 啟動一次轉換 */
    while (ADCF == 0)                /* 等轉換完成 */
        ;
    /* 12 位元結果:ADCRH 是高 8 位,ADCRL 的低 4 位是最低 4 位 */
    return ((unsigned int)ADCRH << 4) | (ADCRL & 0x0F);
}

void delay_about_300ms(void)
{
    volatile unsigned int i, j;
    for (i = 0; i < 350; i++)
        for (j = 0; j < 120; j++)
            ;
}

void main(void)
{
    unsigned int value;

    uart_init();
    adc_init();

    uart_puts("ADC demo start (turn the potentiometer)\r\n");

    while (1)
    {
        value = adc_read();
        uart_puts("ADC = ");
        uart_put_uint(value);
        uart_puts("\r\n");
        delay_about_300ms();
    }
}
