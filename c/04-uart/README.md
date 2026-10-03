# 第 4 課:UART 序列埠 — 讓晶片對電腦說話(C 版)

## 這個範例做什麼
每隔約 1 秒,從序列埠送出一行 `Hello from N76E003! (count=N)`,N 會遞增。讓你學會把訊息印到電腦。

## 接線
需要一個 USB-TTL 轉接器:
```
N76E003 P0.6 (TXD) --> USB-TTL 的 RX
N76E003 GND        --> USB-TTL 的 GND
```
- 本課只示範「送」,RXD 可先不接。
- 序列埠參數:**9600 8N1**(9600 bps、8 資料位、無同位、1 停止位)。

## 編譯與燒錄
```bash
make        # 編譯,產生 main.hex
make flash  # 燒錄到 N76E003
make clean  # 清除編譯產物
```
> 首次燒錄前,請先完成 [`../../docs/00-環境建置與燒錄.md`](../../docs/00-環境建置與燒錄.md) 的 udev 設定。

## 預期結果
在電腦開啟序列埠終端機,會每秒看到一行:
```
Hello from N76E003! (count=0)
Hello from N76E003! (count=1)
Hello from N76E003! (count=2)
...
```
終端機指令(擇一;裝置名稱依實際情況,常見 `/dev/ttyUSB0`):
```bash
picocom -b 9600 /dev/ttyUSB0
# 或
screen /dev/ttyUSB0 9600
```

## 程式重點
- 用 Timer1 當鮑率產生器(`SMOD=1`、`T1M=1`、`BRCK=0`),`TH1=0x98` → 約 9600 bps。
- `uart_putc` 把字元寫入 `SBUF`,等 `TI` 變 1 表示送完,再手動清 `TI`。
- `uart_put_uint` 示範把數字轉成字串逐位送出。
- 原理與鮑率計算詳見 [`../../docs/05-UART序列埠.md`](../../docs/05-UART序列埠.md)。

## 對照另一個版本
→ 組語版:[`../../asm/04-uart/`](../../asm/04-uart/)。組語版送固定字串(不印遞增數字),並示範用 `movc A,@A+DPTR` 從程式記憶體逐字讀取字串。

## 練習
1. 改鮑率為 115200(同時調整終端機設定),比較穩定度。
2. 接上 RXD,做「收到字元就回送」的回音程式。
3. 把第 3 課的 Timer 計數值定時印出來。
