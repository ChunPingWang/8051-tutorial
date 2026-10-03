# ============================================================================
#  asm/common.mk  ─  組合語言範例共用的 Makefile 設定
# ----------------------------------------------------------------------------
#  各課的 Makefile 只要寫一行： include ../common.mk
#
#  指令:
#    make          組譯 + 連結,產生可燒錄的 main.hex
#    make flash     編譯並燒錄到 N76E003 開發板
#    make clean     刪除所有編譯產生的檔案
# ============================================================================

# ---- 工具 ----
# 註:這裡用 SDLD 而非 LD,因為 LD 是 make 的內建變數(預設 ld),會造成衝突
ASM      ?= sdas8051
SDLD     ?= sdld
PACKIHX  ?= packihx
NUVOPROG ?= nuvoprog

# TARGET:原始碼檔名(不含副檔名),預設 main → main.asm
TARGET   ?= main

# ---- 組譯參數 ----
#   -p 不印分頁標題  -l 產生 .lst 列表  -o 產生目的檔  -s 產生符號表
#   -g 未定義符號視為全域  -f 在列表中標示相對位址  -ff 加強標示
ASMFLAGS ?= -plosgff

# ---------------------------------------------------------------------------
all: $(TARGET).hex

# 組譯:main.asm → main.rel
$(TARGET).rel: $(TARGET).asm
	$(ASM) $(ASMFLAGS) $(TARGET).asm

# 連結:main.rel → main.ihx(-i 輸出 Intel HEX)
$(TARGET).ihx: $(TARGET).rel
	$(SDLD) -i $(TARGET).ihx $(TARGET).rel

# 整理成標準 .hex
$(TARGET).hex: $(TARGET).ihx
	$(PACKIHX) $(TARGET).ihx > $(TARGET).hex

# 燒錄到開發板
#   N76E003 燒錄時一定要指定「設定位元組 (config)」,否則 nuvoprog 會拒絕。
#   CONFIG=FFFFFFFF 是出廠預設:從 APROM 開機、不啟用任何保護鎖。
CONFIG ?= FFFFFFFF
flash: $(TARGET).hex
	$(NUVOPROG) program -t n76e003 -c $(CONFIG) -a $(TARGET).hex

clean:
	rm -f *.ihx *.hex *.rel *.map *.lst *.sym *.rst *.lk

.PHONY: all flash clean
