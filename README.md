# N76E003(8051)初學者教學手冊 ─ 組合語言 × C 語言對照

[![build-and-test](https://github.com/ChunPingWang/8051-tutorial/actions/workflows/ci.yml/badge.svg)](https://github.com/ChunPingWang/8051-tutorial/actions/workflows/ci.yml)

這是一套給**完全初學者**的 8051 微控制器實作教材,使用新唐(Nuvoton)**N76E003** 開發板,
在 **Linux** 上以開源工具鏈(SDCC + nuvoprog)從頭學起。

最大特色:**每一課都同時提供「組合語言」與「C 語言」兩種版本,情境完全一致**,
讓你左右對照,真正理解「C 的一行,在底層到底做了什麼」。

> 所有說明文件皆為**繁體中文**。

---

## 📁 專案結構

```
8051-tutorial/
├── README.md            ← 你正在看的檔案
├── Makefile             ← 頂層:make / make test / make clean(一次操作全部)
├── docs/                ← 教學文件(先讀這裡的概念,再看範例)
│   ├── 00-環境建置與燒錄.md     安裝工具、學會把程式燒進晶片
│   ├── 01-認識N76E003與8051.md  架構、記憶體、SFR 等基礎觀念
│   ├── 02-GPIO與LED閃爍.md       對應第 1 課
│   ├── 03-按鈕輸入與GPIO.md      對應第 2 課
│   ├── 04-計時器與中斷.md        對應第 3 課
│   ├── 05-UART序列埠.md          對應第 4 課
│   ├── 06-ADC類比輸入.md         對應第 5 課
│   ├── 07-開發與測試流程.md      完整開發流程、除錯、測試、CI
│   ├── 08-PWM.md                 對應第 6 課
│   ├── 09-硬體接線與零件表.md    零件清單(BOM)、腳位總表、各課接線圖
│   ├── 10-系統架構圖.md          C4 架構圖 + 初學者理解圖(Mermaid)
│   └── 99-組語與C對照速查.md     指令/語法對照表
│
├── include/
│   └── N76E003.h        ← 共用的暫存器(SFR)定義檔,兩種語言都用它
│
├── common/
│   └── 99-nuvoton-nulink.rules  ← Linux 存取燒錄器需要的 udev 規則
│
├── tools/               ← 開發與測試工具
│   ├── build_all.sh           回歸測試:一次編譯所有範例
│   └── host_test/             主機端單元測試(在 PC 上用 gcc 測純邏輯)
│
├── .github/workflows/
│   └── ci.yml           ← 持續整合:每次 push 自動編譯 + 測試
│
├── c/                   ← C 語言範例(每課一個資料夾)
│   ├── common.mk              共用的編譯設定
│   ├── 01-blink/ 02-button/ 03-timer/ 04-uart/ 05-adc/ 06-pwm/
│   └──   └ main.c / Makefile / README.md
│
└── asm/                 ← 組合語言範例(與 c/ 一一對應)
    ├── common.mk
    ├── 01-blink/   02-button/   03-timer/   04-uart/   05-adc/
    └──   └ main.asm / Makefile / README.md
```

---

## 🧭 建議學習路線

0. **備齊零件、看懂接線**:[`docs/09-硬體接線與零件表.md`](docs/09-硬體接線與零件表.md)(零件清單、腳位、各課接線圖)。
   想先看系統全貌,可翻 [`docs/10-系統架構圖.md`](docs/10-系統架構圖.md)(C4 架構圖與初學者理解圖)。
1. **先讀** [`docs/01-認識N76E003與8051.md`](docs/01-認識N76E003與8051.md),建立基本觀念(不用寫程式)。
2. **再讀** [`docs/00-環境建置與燒錄.md`](docs/00-環境建置與燒錄.md),把工具裝好、學會燒錄。
3. 依序進行 6 課,每一課都:
   - 先讀對應的 `docs/` 章節(了解原理)
   - 再做 `c/` 版本(好懂、快速看到成果)
   - 最後對照 `asm/` 版本(理解底層如何運作)
4. 想更有紀律地開發,讀 [`docs/07-開發與測試流程.md`](docs/07-開發與測試流程.md)
   (編譯 → 測試 → 燒錄 → 除錯 → 版本控制的完整循環)。

| 課程 | 主題 | 學到的核心能力 |
|------|------|----------------|
| 第 1 課 | LED 閃爍 | GPIO 輸出、延時、主迴圈 |
| 第 2 課 | 按鈕控制 LED | GPIO 輸入、條件判斷、上拉 |
| 第 3 課 | 計時器中斷 | Timer、中斷、精準時間 |
| 第 4 課 | UART 序列埠 | 與電腦通訊、印出除錯訊息 |
| 第 5 課 | ADC 類比輸入 | 讀可變電阻/感測器的類比值 |
| 第 6 課 | PWM 呼吸燈 | 硬體 PWM、控制 LED 亮度/馬達轉速 |

---

## ⚡ 快速開始(假設已裝好工具)

```bash
# 編譯並燒錄 C 版的第 1 課
cd c/01-blink
make          # 編譯,產生 main.hex
make flash    # 燒錄到開發板

# 組語版做同樣的事
cd ../../asm/01-blink
make flash
```

工具安裝、燒錄器接線、udev 權限設定等細節,請看
[`docs/00-環境建置與燒錄.md`](docs/00-環境建置與燒錄.md)。

---

## 🧪 開發與測試流程

本教材不只教「寫程式」,也示範一套**完整的開發與測試流程**(詳見
[`docs/07-開發與測試流程.md`](docs/07-開發與測試流程.md)):

```bash
# 在專案根目錄
make          # 編譯全部 10 個範例(C + 組語),確認都建置得起來
make test     # 回歸測試:主機端單元測試 + 編譯所有範例
make clean    # 清掉所有編譯產物
```

- **主機端單元測試**:把「不碰硬體的純邏輯」(如數字轉字串、ADC 換算電壓)抽到
  [`tools/host_test/`](tools/host_test/),用電腦的 `gcc` + `assert` 秒速驗證,不必每次都燒板子。
- **回歸測試**:[`tools/build_all.sh`](tools/build_all.sh) 一次編譯所有範例,改到共用檔時確認沒弄壞任何一課。
- **持續整合 (CI)**:每次 `git push`,GitHub Actions 會自動跑 `make test`(見上方徽章)。

完整流程:`寫程式 → make → make test → make flash → 實機測試 → 除錯 → git commit/push`。

---

## 🔧 需要的硬體與軟體

- **硬體**:Nuvoton N76E003 開發板、Nu-Link(1 或 2)燒錄器、幾顆 LED、電阻、按鈕、麵包板、杜邦線。
  👉 完整零件清單(BOM)、腳位總表與每一課的接線圖:[`docs/09-硬體接線與零件表.md`](docs/09-硬體接線與零件表.md)。
- **軟體(Linux)**:
  - [SDCC](https://sdcc.sourceforge.net/) — C 編譯器 + 組譯器(`sdas8051`)
  - [nuvoprog](https://github.com/erincandescent/nuvoprog) — 燒錄工具
  - `make`

---

## 📜 授權

本教材可自由用於學習與教學。範例程式碼以 MIT 精神提供,請自由取用修改。
