#!/usr/bin/env bash
# ============================================================================
#  build_all.sh ─ 回歸測試:一次編譯所有範例,確認全部都能建置成功
# ----------------------------------------------------------------------------
#  用途:每次改動共用檔(N76E003.h、common.mk)後,跑這支確認沒有弄壞任何一課。
#  用法:  bash tools/build_all.sh        (在專案根目錄或任何地方執行皆可)
#  回傳:  全部成功 → 離開碼 0;有任何一課失敗 → 離開碼 1(方便 CI 判斷)
# ============================================================================
set -u

# 切換到專案根目錄(本腳本在 tools/ 底下)
cd "$(dirname "$0")/.." || exit 1

LESSONS="
c/01-blink c/02-button c/03-timer c/04-uart c/05-adc c/06-pwm
asm/01-blink asm/02-button asm/03-timer asm/04-uart asm/05-adc asm/06-pwm
"

pass=0
fail=0
echo "===== 編譯回歸測試 ====="
for d in $LESSONS; do
    make -C "$d" clean >/dev/null 2>&1
    if make -C "$d" >/dev/null 2>&1 && [ -f "$d/main.hex" ]; then
        printf "  \033[32m[ OK ]\033[0m  %s\n" "$d"
        pass=$((pass + 1))
    else
        printf "  \033[31m[FAIL]\033[0m  %s\n" "$d"
        echo "         ↳ 重新顯示錯誤訊息:"
        make -C "$d" 2>&1 | sed 's/^/           /'
        fail=$((fail + 1))
    fi
    make -C "$d" clean >/dev/null 2>&1
done

# 進階範例:需用 TARGET 指定的額外原始碼(第 6 課的正弦馬達)
for d in c/06-pwm asm/06-pwm; do
    make -C "$d" TARGET=motor_sine clean >/dev/null 2>&1
    if make -C "$d" TARGET=motor_sine >/dev/null 2>&1 && [ -f "$d/motor_sine.hex" ]; then
        printf "  \033[32m[ OK ]\033[0m  %s (motor_sine)\n" "$d"
        pass=$((pass + 1))
    else
        printf "  \033[31m[FAIL]\033[0m  %s (motor_sine)\n" "$d"
        fail=$((fail + 1))
    fi
    make -C "$d" TARGET=motor_sine clean >/dev/null 2>&1
done

echo "-----------------------------------------"
echo "通過 $pass 課,失敗 $fail 課。"
[ "$fail" -eq 0 ] && echo "✅ 全部範例建置成功" || echo "❌ 有範例建置失敗"
exit $([ "$fail" -eq 0 ] && echo 0 || echo 1)
