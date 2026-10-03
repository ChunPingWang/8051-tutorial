# 主機端單元測試(用電腦的 gcc 編譯執行,與 8051 無關)
CC     ?= gcc
CFLAGS ?= -Wall -Wextra -O2

# 第 7 課的狀態機原始碼就放在課程資料夾裡,這裡直接拿來測
# (同一份 fsm.c 也會被 SDCC 編進 N76E003 —— 測的就是上機的那份程式)
FSMDIR ?= ../../c/07-fsm

test: test_logic test_fsm
	./test_logic
	@echo ""
	./test_fsm

test_logic: test_logic.c logic.c logic.h
	$(CC) $(CFLAGS) -o test_logic test_logic.c logic.c

test_fsm: test_fsm.c $(FSMDIR)/fsm.c $(FSMDIR)/fsm.h
	$(CC) $(CFLAGS) -I$(FSMDIR) -o test_fsm test_fsm.c $(FSMDIR)/fsm.c

clean:
	rm -f test_logic test_fsm

.PHONY: test clean
