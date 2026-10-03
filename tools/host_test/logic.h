/* logic.h ─ 純邏輯函式的宣告(可在 PC 與 8051 共用) */
#ifndef LOGIC_H
#define LOGIC_H

void uint_to_dec(unsigned int n, char *out);
unsigned int adc_to_mv(unsigned int adc, unsigned int vdd_mv);

#endif /* LOGIC_H */
