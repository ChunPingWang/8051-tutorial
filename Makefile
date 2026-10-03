# ============================================================================
#  頂層 Makefile ─ 一次操作所有範例
# ----------------------------------------------------------------------------
#    make all     編譯全部範例(C + 組語)
#    make test    回歸測試:編譯全部範例 + 執行主機端單元測試
#    make sim     執行第 7 課的狀態機模擬器(純電腦,不需要開發板)
#    make clean   清掉所有範例的編譯產物
#
#  個別一課的編譯/燒錄,請進到該課資料夾下 make / make flash(見各課 README)。
# ============================================================================

LESSONS := c/01-blink c/02-button c/03-timer c/04-uart c/05-adc c/06-pwm c/07-fsm \
           asm/01-blink asm/02-button asm/03-timer asm/04-uart asm/05-adc asm/06-pwm \
           asm/07-fsm

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

# 第 7 課狀態機模擬器:在電腦上餵不同輸入、看輸出(完全不需要硬體)
sim:
	@$(MAKE) --no-print-directory -C tools/fsm_sim run

clean:
	@for d in $(LESSONS); do $(MAKE) --no-print-directory -C $$d clean; done
	@$(MAKE) --no-print-directory -C tools/host_test clean
	@$(MAKE) --no-print-directory -C tools/fsm_sim clean
	@echo "已清除所有編譯產物"

.PHONY: all test sim clean
