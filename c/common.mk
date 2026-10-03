# ============================================================================
#  c/common.mk  ─  C 語言範例共用的 Makefile 設定
# ----------------------------------------------------------------------------
#  各課的 Makefile 只要寫一行： include ../common.mk
#  就會自動擁有 make / make flash / make clean 等指令。
#
#  指令:
#    make          編譯,產生可燒錄的 main.hex
#    make flash     編譯並燒錄到 N76E003 開發板(需先裝好 nuvoprog)
#    make clean     刪除所有編譯產生的檔案
# ============================================================================

# ---- 工具 ----
SDCC    ?= sdcc
PACKIHX ?= packihx
NUVOPROG ?= nuvoprog

# ---- 路徑與目標 ----
# INCDIR:共用的 N76E003.h 放的位置
INCDIR  ?= ../../include
# TARGET:原始碼檔名(不含副檔名),預設 main → main.c
TARGET  ?= main

# ---- N76E003 的編譯參數 ----
#   -mmcs51        : 目標是 8051 架構
#   --model-small  : 變數預設放在內部 RAM(小而快,適合 N76E003)
#   --code-size    : 程式空間上限 18KB (18432 bytes = N76E003 的 APROM)
#   --xram-size    : 外部 RAM 768 bytes
#   --iram-size    : 內部 RAM 256 bytes
MCUFLAGS ?= -mmcs51 --model-small --code-size 18432 --xram-size 768 --iram-size 256

# ---------------------------------------------------------------------------
all: $(TARGET).hex

# C 原始碼 → .ihx (SDCC 的 Intel HEX 格式)
$(TARGET).ihx: $(TARGET).c
	$(SDCC) $(MCUFLAGS) -I$(INCDIR) $(TARGET).c -o $(TARGET).ihx

# .ihx → 標準 .hex (用 packihx 整理成燒錄工具吃的格式)
$(TARGET).hex: $(TARGET).ihx
	$(PACKIHX) $(TARGET).ihx > $(TARGET).hex

# 燒錄到開發板
flash: $(TARGET).hex
	$(NUVOPROG) program -t n76e003 -a $(TARGET).hex

clean:
	rm -f *.ihx *.hex *.rel *.map *.mem *.lst *.sym *.rst *.lk *.asm *.adb *.cdb

.PHONY: all flash clean
