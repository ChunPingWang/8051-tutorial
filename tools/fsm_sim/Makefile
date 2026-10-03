# 第 7 課狀態機的主機端模擬器(用電腦的 gcc 編譯執行,與 8051 無關)
CC     ?= gcc
CFLAGS ?= -Wall -Wextra -O2
FSMDIR ?= ../../c/07-fsm

run: fsm_sim
	./fsm_sim

fsm_sim: sim.c $(FSMDIR)/fsm.c $(FSMDIR)/fsm.h
	$(CC) $(CFLAGS) -I$(FSMDIR) -o fsm_sim sim.c $(FSMDIR)/fsm.c

clean:
	rm -f fsm_sim

.PHONY: run clean
