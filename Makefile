# ============================================================================
#  頂層 Makefile ─ 一次操作所有範例
# ----------------------------------------------------------------------------
#    make all     編譯全部 10 個範例(C + 組語)
#    make test    回歸測試:編譯全部範例 + 執行主機端單元測試
#    make clean   清掉所有範例的編譯產物
#
#  個別一課的編譯/燒錄,請進到該課資料夾下 make / make flash(見各課 README)。
# ============================================================================

LESSONS := c/01-blink c/02-button c/03-timer c/04-uart c/05-adc c/06-pwm \
           asm/01-blink asm/02-button asm/03-timer asm/04-uart asm/05-adc asm/06-pwm

all:
	@for d in $(LESSONS); do \
		echo ">>> 編譯 $$d"; \
		$(MAKE) --no-print-directory -C $$d || exit 1; \
	done
	@echo "✅ 全部範例編譯完成"

# 回歸測試:先跑主機端單元測試,再編譯所有範例
test:
	@$(MAKE) --no-print-directory -C tools/host_test test
	@echo ""
	@bash tools/build_all.sh

clean:
	@for d in $(LESSONS); do $(MAKE) --no-print-directory -C $$d clean; done
	@$(MAKE) --no-print-directory -C tools/host_test clean
	@echo "已清除所有編譯產物"

.PHONY: all test clean
