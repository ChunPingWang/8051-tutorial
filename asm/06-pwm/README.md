# 第 6 課:PWM 呼吸燈(組合語言版)

## 這個範例做什麼
用硬體 PWM 控制 P1.2 的 LED 亮度,做出由暗漸亮、再漸暗、不斷循環的「呼吸燈」。
與 C 版 (`c/06-pwm`) 完全相同。

## 接線
```
P1.2 ──► 220Ω ──► LED 長腳(+),LED 短腳(−)──► GND
```
- ⚠️ LED 接 **P1.2**(PWM0 的輸出腳),不是前幾課的 P1.4。
- ⚠️ 板載 LED 極性可能相反,必要時調整。

## 編譯與燒錄
```bash
make        # 組譯+連結,產生 main.hex
make flash  # 燒錄到 N76E003
make clean  # 清除編譯產物
```
> 首次燒錄前,請先完成 [`../../docs/00-環境建置與燒錄.md`](../../docs/00-環境建置與燒錄.md) 的 udev 設定。

## 預期結果
P1.2 的 LED 平滑地呼吸(漸亮→漸暗→循環),一個循環約 2~3 秒。

## 程式重點
- `pwm_init` 設定:`PIOCON0=0x01`(PWM0→P1.2)、`PWMCON1=0x07`(Fsys/128)、
  週期 `PWMPL=0xFF`,最後 `setb PWMRUN` 啟動。
- `set_duty` 把 A 寫進 `PWM0L`,再 `setb LOAD` 讓新亮度生效。
- 呼吸迴圈用 `R7` 當 duty,`inc`/`dec` 到 0 時用 `jnz` 判斷結束。
- 小細節:`PWMCON1`(SFR 0xDF,整體存取)與 `PWMRUN`(位元,PWMCON0.7)剛好都是
  位址 0xDF,但一個用 `mov`、一個用 `setb`,定址方式不同、互不衝突。
- 原理詳見 [`../../docs/08-PWM.md`](../../docs/08-PWM.md)。

## 進階範例:正弦加減速控制直流馬達
同資料夾另附 [`motor_sine.asm`](motor_sine.asm):用正弦查表讓**直流馬達**轉速平滑
加減速。組語版直接用「已壓縮成 40~255 的正弦表」查表(省去乘除)。
- ⚠️ **馬達不可直接接 MCU**,必須經驅動器、獨立電源、續流二極體、共地。
  電路與原理見 [`../../docs/08-PWM.md`](../../docs/08-PWM.md) 的 §8.8。
- 編譯/燒錄:
  ```bash
  make TARGET=motor_sine
  make TARGET=motor_sine flash
  ```

## 對照另一個版本
→ C 版:[`../../c/06-pwm/`](../../c/06-pwm/)。設定逐行對應,只有 duty 迴圈寫法不同
(C 用 `for`、組語用 `inc`/`dec` + `jnz`)。馬達範例的表壓縮方式差異見 docs/08 §8.8。

## 練習
1. 調整 `step_delay` 的 `R5` 值,改變呼吸速度。
2. 改 `PWM0L` 固定值,讓 LED 停在某個亮度。
3. 進階:啟用第二個通道(如 `PIOCON0 |= 0x04` 的 PWM2→P1.0),雙燈不同亮度。
