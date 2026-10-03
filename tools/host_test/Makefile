# 主機端單元測試(用電腦的 gcc 編譯執行,與 8051 無關)
CC     ?= gcc
CFLAGS ?= -Wall -Wextra -O2

test: test_logic
	./test_logic

test_logic: test_logic.c logic.c logic.h
	$(CC) $(CFLAGS) -o test_logic test_logic.c logic.c

clean:
	rm -f test_logic

.PHONY: test clean
