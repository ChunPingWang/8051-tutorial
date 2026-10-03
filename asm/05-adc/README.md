# 第 5 課:ADC 類比輸入 — 讀可變電阻的電壓(組合語言版)

## 這個範例做什麼
讀取 AIN0(P1.7)上可變電阻的 12 位元 ADC 值,透過 UART 以十六進位印到電腦。轉動電阻時數字跟著變。

## 接線
```
可變電阻一端 ──► VDD (3.3V)
可變電阻另一端 ──► GND
可變電阻中間(可動端)──► P1.7 (AIN0)

UART:P0.6 (TXD) ──► USB-TTL 的 RX,GND 共接
```
- 序列埠參數:**9600 8N1**。

## 編譯與燒錄
```bash
make        # 組譯 + 連結,產生 main.hex
make flash  # 燒錄到 N76E003
make clean  # 清除編譯產物
```
> 首次燒錄前,請先完成 [`../../docs/00-環境建置與燒錄.md`](../../docs/00-環境建置與燒錄.md) 的 udev 設定。

## 預期結果
終端機約每 0.3 秒出現一行,轉動電阻時數字在 000~FFF 之間變化:
```
ADC=0x000
ADC=0x7FF
ADC=0xFFF
...
```
終端機指令:`picocom -b 9600 /dev/ttyUSB0`(或 `screen /dev/ttyUSB0 9600`)。

## 程式重點
- `adc_init`:P1.7 設僅輸入、`AINDIDS` 禁用數位輸入、`ADCCON0` 選通道 0、`ADCCON1` 開 `ADCEN`。
- `adc_read`:`clr ADCF` → `setb ADCS` → `jnb ADCF, wait` → 把 `ADCRH`/`ADCRL` 存進 R6/R7。
- `put_hex`:用 `swap A` 取高/低 nibble,查 `hextab` 表轉成十六進位字元送出。
- 原理詳見 [`../../docs/06-ADC類比輸入.md`](../../docs/06-ADC類比輸入.md)。

## 對照另一個版本
→ C 版:[`../../c/05-adc/`](../../c/05-adc/)。讀的是同一個 ADC 值,但 C 印**十進位**(0~4095),本組語版為求單純印**十六進位**(000~FFF)。

## 練習
1. 替組語版加上十進位輸出(需要 16 位元除以 10 的迴圈)。
2. 當高位元組超過門檻時點亮一顆 LED。
3. 改讀 AIN1(P3.0),對照 `include/N76E003.h` 調整通道與腳位設定。
