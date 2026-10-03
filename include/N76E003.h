/*----------------------------------------------------------------------------
 * N76E003.h  ─  Nuvoton N76E003 特殊功能暫存器 (SFR) 定義檔  (SDCC 專用)
 *----------------------------------------------------------------------------
 * 本檔把 N76E003 內部的「暫存器名稱」對應到它在晶片中的「記憶體位址」,
 * 讓你在 C 程式裡可以直接寫 P1 = 0x00; 而不必記住 0x90 這種數字。
 *
 * SDCC 語法說明:
 *   __sfr  __at (位址) 名稱;   → 定義一個 8 位元的特殊功能暫存器 (SFR)
 *   __sbit __at (位址) 名稱;   → 定義一個「可單獨存取」的位元 (bit-addressable)
 *
 * 位址對照自 Nuvoton N76E003 資料手冊 (Datasheet) 的 SFR Map。
 * 標有 "TA" 的暫存器受「Timed Access」保護,寫入前要先解鎖 (見下方 TA 說明)。
 * 標有 "Page1" 的暫存器與同位址的另一個暫存器共用,需切換 SFRS 頁面才能存取。
 *--------------------------------------------------------------------------*/

#ifndef __N76E003_H__
#define __N76E003_H__

/* ====== 核心 / 標準 8051 暫存器 (0x80 ~ 0x8F) ====== */
__sfr __at (0x80) P0;        /* Port 0 資料暫存器 */
__sfr __at (0x81) SP;        /* 堆疊指標 Stack Pointer */
__sfr __at (0x82) DPL;       /* 資料指標低位元組 */
__sfr __at (0x83) DPH;       /* 資料指標高位元組 */
__sfr __at (0x84) RCTRIM0;   /* 內部振盪器校正 0 */
__sfr __at (0x85) RCTRIM1;   /* 內部振盪器校正 1 */
__sfr __at (0x86) RWK;       /* 喚醒計時器重載值 */
__sfr __at (0x87) PCON;      /* 電源控制 (含串列埠 SMOD 位元) */

__sfr __at (0x88) TCON;      /* Timer/計數器 控制 */
__sfr __at (0x89) TMOD;      /* Timer/計數器 模式 */
__sfr __at (0x8A) TL0;       /* Timer0 低位元組 */
__sfr __at (0x8B) TL1;       /* Timer1 低位元組 */
__sfr __at (0x8C) TH0;       /* Timer0 高位元組 */
__sfr __at (0x8D) TH1;       /* Timer1 高位元組 */
__sfr __at (0x8E) CKCON;     /* 時脈控制 (Timer 時脈來源等) */
__sfr __at (0x8F) WKCON;     /* 喚醒計時器控制 */

/* ====== Port 1 與時脈 / 輸入捕捉 (0x90 ~ 0x97) ====== */
__sfr __at (0x90) P1;        /* Port 1 資料暫存器 */
__sfr __at (0x91) SFRS;      /* SFR 頁面選擇 (TA 保護) 0=Page0 1=Page1 */
__sfr __at (0x92) CAPCON0;   /* 輸入捕捉控制 0 */
__sfr __at (0x93) CAPCON1;   /* 輸入捕捉控制 1 */
__sfr __at (0x94) CAPCON2;   /* 輸入捕捉控制 2 */
__sfr __at (0x95) CKDIV;     /* 系統時脈除頻 */
__sfr __at (0x96) CKSWT;     /* 時脈切換 (TA 保護) */
__sfr __at (0x97) CKEN;      /* 時脈致能 (TA 保護) */

/* ====== 串列埠 UART0 與擴充中斷 (0x98 ~ 0x9F) ====== */
__sfr __at (0x98) SCON;      /* UART0 控制 */
__sfr __at (0x99) SBUF;      /* UART0 資料緩衝器 */
__sfr __at (0x9A) SBUF_1;    /* UART1 資料緩衝器 */
__sfr __at (0x9B) EIE;       /* 擴充中斷致能 */
__sfr __at (0x9C) EIE1;      /* 擴充中斷致能 1 */
__sfr __at (0x9F) CHPCON;    /* 晶片控制 (TA 保護, IAP/開機來源) */

/* ====== Port 2 與 IAP (0xA0 ~ 0xA7) ====== */
__sfr __at (0xA0) P2;        /* Port 2 (N76E003 僅 P2.0,通常作 /RST) */
__sfr __at (0xA2) AUXR1;     /* 輔助暫存器 1 */
__sfr __at (0xA3) BODCON0;   /* 低電壓偵測控制 0 (TA 保護) */
__sfr __at (0xA4) IAPTRG;    /* IAP 觸發 (TA 保護) */
__sfr __at (0xA5) IAPUEN;    /* IAP 更新致能 (TA 保護) */
__sfr __at (0xA6) IAPAL;     /* IAP 位址低位元組 */
__sfr __at (0xA7) IAPAH;     /* IAP 位址高位元組 */

/* ====== 中斷致能 / Port 3 模式 (0xA8 ~ 0xAF) ====== */
__sfr __at (0xA8) IE;        /* 中斷致能 */
__sfr __at (0xA9) SADDR;     /* UART0 從機位址 */
__sfr __at (0xAA) WDCON;     /* 看門狗控制 (TA 保護) */
__sfr __at (0xAB) BODCON1;   /* 低電壓偵測控制 1 (TA 保護) */
__sfr __at (0xAC) P3M1;      /* Port 3 模式暫存器 1 */
__sfr __at (0xAC) P3S;       /* (Page1) Port 3 施密特觸發 */
__sfr __at (0xAD) P3M2;      /* Port 3 模式暫存器 2 */
__sfr __at (0xAD) P3SR;      /* (Page1) Port 3 迴轉率 */
__sfr __at (0xAE) IAPFD;     /* IAP 資料 */
__sfr __at (0xAF) IAPCN;     /* IAP 命令 */

/* ====== Port 3 / Port 模式暫存器 (0xB0 ~ 0xB7) ====== */
__sfr __at (0xB0) P3;        /* Port 3 資料暫存器 (N76E003 僅 P3.0) */
__sfr __at (0xB1) P0M1;      /* Port 0 模式暫存器 1 */
__sfr __at (0xB1) P0S;       /* (Page1) Port 0 施密特觸發 */
__sfr __at (0xB2) P0M2;      /* Port 0 模式暫存器 2 */
__sfr __at (0xB2) P0SR;      /* (Page1) Port 0 迴轉率 */
__sfr __at (0xB3) P1M1;      /* Port 1 模式暫存器 1 */
__sfr __at (0xB3) P1S;       /* (Page1) Port 1 施密特觸發 */
__sfr __at (0xB4) P1M2;      /* Port 1 模式暫存器 2 */
__sfr __at (0xB4) P1SR;      /* (Page1) Port 1 迴轉率 */
__sfr __at (0xB5) P2S;       /* Port 2 施密特觸發 */
__sfr __at (0xB7) IPH;       /* 中斷優先權 (高位) */
__sfr __at (0xB7) PWMINTC;   /* (Page1) PWM 中斷控制 */

/* ====== 中斷優先權 / I2C / ADC 結果 (0xB8 ~ 0xBF) ====== */
__sfr __at (0xB8) IP;        /* 中斷優先權 (低位) */
__sfr __at (0xB9) SADEN;     /* UART0 位址遮罩 */
__sfr __at (0xBA) SADEN_1;   /* UART1 位址遮罩 */
__sfr __at (0xBB) SADDR_1;   /* UART1 從機位址 */
__sfr __at (0xBC) I2DAT;     /* I2C 資料 */
__sfr __at (0xBD) I2STAT;    /* I2C 狀態 */
__sfr __at (0xBE) I2CLK;     /* I2C 時脈 */
__sfr __at (0xBF) I2TOC;     /* I2C 逾時計數 */

/* ====== I2C / ADC 結果 / Timer3 / PWM (0xC0 ~ 0xCF) ====== */
__sfr __at (0xC0) I2CON;     /* I2C 控制 */
__sfr __at (0xC1) I2ADDR;    /* I2C 位址 */
__sfr __at (0xC2) ADCRL;     /* ADC 結果低位元 (低 4 位) */
__sfr __at (0xC3) ADCRH;     /* ADC 結果高位元 (高 8 位) */
__sfr __at (0xC4) T3CON;     /* Timer3 控制 */
__sfr __at (0xC4) PWM4H;     /* (Page1) PWM 通道4 高位 */
__sfr __at (0xC5) RL3;       /* Timer3 重載低位元組 */
__sfr __at (0xC5) PWM5H;     /* (Page1) PWM 通道5 高位 */
__sfr __at (0xC6) RH3;       /* Timer3 重載高位元組 */
__sfr __at (0xC6) PIOCON1;   /* (Page1) PWM 輸出腳位控制 1 */
__sfr __at (0xC7) TA;        /* Timed Access 解鎖暫存器 (寫 0xAA 再 0x55) */

__sfr __at (0xC8) T2CON;     /* Timer2 控制 */
__sfr __at (0xC9) T2MOD;     /* Timer2 模式 */
__sfr __at (0xCA) RCMP2L;    /* Timer2 比較低位元組 */
__sfr __at (0xCB) RCMP2H;    /* Timer2 比較高位元組 */
__sfr __at (0xCC) TL2;       /* Timer2 低位元組 */
__sfr __at (0xCC) PWM4L;     /* (Page1) PWM 通道4 低位 */
__sfr __at (0xCD) TH2;       /* Timer2 高位元組 */
__sfr __at (0xCD) PWM5L;     /* (Page1) PWM 通道5 低位 */
__sfr __at (0xCE) ADCMPL;    /* ADC 比較低位元組 */
__sfr __at (0xCF) ADCMPH;    /* ADC 比較高位元組 */

/* ====== PSW / PWM 高位 (0xD0 ~ 0xD7) ====== */
__sfr __at (0xD0) PSW;       /* 程式狀態字 */
__sfr __at (0xD1) PWMPH;     /* PWM 週期高位 */
__sfr __at (0xD2) PWM0H;     /* PWM0 高位 */
__sfr __at (0xD3) PWM1H;     /* PWM1 高位 */
__sfr __at (0xD4) PWM2H;     /* PWM2 高位 */
__sfr __at (0xD5) PWM3H;     /* PWM3 高位 */
__sfr __at (0xD6) PNP;       /* PWM 正反相輸出選擇 */
__sfr __at (0xD7) FBD;       /* 錯誤偵測 (fault brake) */

/* ====== PWM 控制 / 低位 (0xD8 ~ 0xDF) ====== */
__sfr __at (0xD8) PWMCON0;   /* PWM 控制 0 */
__sfr __at (0xD9) PWMPL;     /* PWM 週期低位 */
__sfr __at (0xDA) PWM0L;     /* PWM0 低位 */
__sfr __at (0xDB) PWM1L;     /* PWM1 低位 */
__sfr __at (0xDC) PWM2L;     /* PWM2 低位 */
__sfr __at (0xDD) PWM3L;     /* PWM3 低位 */
__sfr __at (0xDE) PIOCON0;   /* PWM 輸出腳位控制 0 */
__sfr __at (0xDF) PWMCON1;   /* PWM 控制 1 */

/* ====== ACC / ADC 設定 / 比較器 (0xE0 ~ 0xEF) ====== */
__sfr __at (0xE0) ACC;       /* 累加器 */
__sfr __at (0xE1) ADCCON1;   /* ADC 控制 1 */
__sfr __at (0xE2) ADCCON2;   /* ADC 控制 2 */
__sfr __at (0xE3) ADCDLY;    /* ADC 延遲 */
__sfr __at (0xE4) C0L;       /* 捕捉通道0 低位 */
__sfr __at (0xE5) C0H;       /* 捕捉通道0 高位 */
__sfr __at (0xE6) C1L;       /* 捕捉通道1 低位 */
__sfr __at (0xE7) C1H;       /* 捕捉通道1 高位 */

__sfr __at (0xE8) ADCCON0;   /* ADC 控制 0 (含通道選擇與啟動位) */
__sfr __at (0xE9) PICON;     /* 腳位中斷控制 */
__sfr __at (0xEA) PINEN;     /* 腳位中斷下降緣致能 */
__sfr __at (0xEB) PIPEN;     /* 腳位中斷上升緣致能 */
__sfr __at (0xEC) PIF;       /* 腳位中斷旗標 */
__sfr __at (0xED) C2L;       /* 捕捉通道2 低位 */
__sfr __at (0xEE) C2H;       /* 捕捉通道2 高位 */
__sfr __at (0xEF) EIP;       /* 擴充中斷優先權 */

/* ====== B / CAPCON / SPI / 擴充 (0xF0 ~ 0xFF) ====== */
__sfr __at (0xF0) B;         /* B 暫存器 (乘除法用) */
__sfr __at (0xF1) CAPCON3;   /* 輸入捕捉控制 3 */
__sfr __at (0xF2) CAPCON4;   /* 輸入捕捉控制 4 */
__sfr __at (0xF3) SPCR;      /* SPI 控制 */
__sfr __at (0xF3) SPCR2;     /* (Page1) SPI 控制 2 */
__sfr __at (0xF4) SPSR;      /* SPI 狀態 */
__sfr __at (0xF5) SPDR;      /* SPI 資料 */
__sfr __at (0xF6) AINDIDS;   /* 類比輸入腳數位輸入禁用 */
__sfr __at (0xF7) EIPH;      /* 擴充中斷優先權 (高位) */

__sfr __at (0xF8) SCON_1;    /* UART1 控制 */
__sfr __at (0xF9) PDTEN;     /* PWM 死區致能 (TA 保護) */
__sfr __at (0xFA) PDTCNT;    /* PWM 死區計數 (TA 保護) */
__sfr __at (0xFB) PMEN;      /* 腳位監測致能 */
__sfr __at (0xFC) PMD;       /* 腳位監測資料 */
__sfr __at (0xFE) EIP1;      /* 擴充中斷優先權 1 */
__sfr __at (0xFF) EIPH1;     /* 擴充中斷優先權 1 (高位) */

/* =========================================================================
 *  可單獨定址的位元 (bit-addressable)
 * ========================================================================= */

/* ---- IE 中斷致能 (0xA8) ---- */
__sbit __at (0xAF) EA;       /* 總中斷開關 (1=開啟全部中斷) */
__sbit __at (0xAE) EADC;     /* ADC 中斷致能 */
__sbit __at (0xAD) EBOD;     /* 低電壓中斷致能 */
__sbit __at (0xAC) ES;       /* UART0 中斷致能 */
__sbit __at (0xAB) ET1;      /* Timer1 中斷致能 */
__sbit __at (0xAA) EX1;      /* 外部中斷1 致能 */
__sbit __at (0xA9) ET0;      /* Timer0 中斷致能 */
__sbit __at (0xA8) EX0;      /* 外部中斷0 致能 */

/* ---- TCON Timer 控制 (0x88) ---- */
__sbit __at (0x8F) TF1;      /* Timer1 溢位旗標 */
__sbit __at (0x8E) TR1;      /* Timer1 啟動 (1=跑) */
__sbit __at (0x8D) TF0;      /* Timer0 溢位旗標 */
__sbit __at (0x8C) TR0;      /* Timer0 啟動 (1=跑) */
__sbit __at (0x8B) IE1;      /* 外部中斷1 旗標 */
__sbit __at (0x8A) IT1;      /* 外部中斷1 觸發方式 (1=邊緣) */
__sbit __at (0x89) IE0;      /* 外部中斷0 旗標 */
__sbit __at (0x88) IT0;      /* 外部中斷0 觸發方式 (1=邊緣) */

/* ---- SCON UART0 控制 (0x98) ---- */
__sbit __at (0x9F) SM0;      /* 串列模式位 0 */
__sbit __at (0x9F) FE;       /* 框架錯誤 (與 SM0 共用,由 PCON.6 選擇) */
__sbit __at (0x9E) SM1;      /* 串列模式位 1 */
__sbit __at (0x9D) SM2;      /* 多機通訊致能 */
__sbit __at (0x9C) REN;      /* 接收致能 (1=允許接收) */
__sbit __at (0x9B) TB8;      /* 傳送第 9 位 */
__sbit __at (0x9A) RB8;      /* 接收第 9 位 */
__sbit __at (0x99) TI;       /* 傳送完成旗標 (需軟體清除) */
__sbit __at (0x98) RI;       /* 接收完成旗標 (需軟體清除) */

/* ---- PSW 程式狀態字 (0xD0) ---- */
__sbit __at (0xD7) CY;       /* 進位旗標 */
__sbit __at (0xD6) AC;       /* 輔助進位 */
__sbit __at (0xD5) F0;       /* 使用者旗標 0 */
__sbit __at (0xD4) RS1;      /* 暫存器庫選擇 1 */
__sbit __at (0xD3) RS0;      /* 暫存器庫選擇 0 */
__sbit __at (0xD2) OV;       /* 溢位旗標 */
__sbit __at (0xD0) P;        /* 奇偶旗標 */

/* ---- IP 中斷優先權 (0xB8) ---- */
__sbit __at (0xBE) PADC;
__sbit __at (0xBD) PBOD;
__sbit __at (0xBC) PS;
__sbit __at (0xBB) PT1;
__sbit __at (0xBA) PX1;
__sbit __at (0xB9) PT0;
__sbit __at (0xB8) PX0;

/* ---- ADCCON0 ADC 控制 (0xE8) ---- */
__sbit __at (0xEF) ADCF;     /* ADC 轉換完成旗標 (需軟體清除) */
__sbit __at (0xEE) ADCS;     /* ADC 啟動轉換 (寫 1 開始) */
__sbit __at (0xED) ETGSEL1;  /* 外部觸發來源選擇 1 */
__sbit __at (0xEC) ETGSEL0;  /* 外部觸發來源選擇 0 */
__sbit __at (0xEB) ADCHS3;   /* ADC 通道選擇 bit3 */
__sbit __at (0xEA) ADCHS2;   /* ADC 通道選擇 bit2 */
__sbit __at (0xE9) ADCHS1;   /* ADC 通道選擇 bit1 */
__sbit __at (0xE8) ADCHS0;   /* ADC 通道選擇 bit0 */

/* ---- PWMCON0 (0xD8) ---- */
__sbit __at (0xDF) PWMRUN;   /* PWM 執行 */
__sbit __at (0xDE) LOAD;     /* PWM 載入 */
__sbit __at (0xDD) PWMF;     /* PWM 中斷旗標 */
__sbit __at (0xDC) CLRPWM;   /* 清除 PWM 計數 */

/* ---- Port 0 各腳位 (0x80) ---- */
__sbit __at (0x87) P07;
__sbit __at (0x87) RXD;      /* UART0 接收腳 (與 P0.7 共用) */
__sbit __at (0x86) P06;
__sbit __at (0x86) TXD;      /* UART0 傳送腳 (與 P0.6 共用) */
__sbit __at (0x85) P05;
__sbit __at (0x84) P04;
__sbit __at (0x83) P03;
__sbit __at (0x82) P02;
__sbit __at (0x81) P01;
__sbit __at (0x80) P00;

/* ---- Port 1 各腳位 (0x90) ---- */
__sbit __at (0x97) P17;
__sbit __at (0x96) P16;
__sbit __at (0x95) P15;
__sbit __at (0x94) P14;
__sbit __at (0x93) P13;
__sbit __at (0x92) P12;
__sbit __at (0x91) P11;
__sbit __at (0x90) P10;

/* ---- Port 2 / Port 3 可定址腳位 ---- */
__sbit __at (0xA0) P20;      /* P2.0 (通常作 /RST) */
__sbit __at (0xB0) P30;      /* P3.0 */

/* =========================================================================
 *  常用常數 / 輔助巨集
 * ========================================================================= */

/* --- GPIO 腳位模式 (每支腳由 PxM1、PxM2 的同一位元共同決定) ---
 *   PxM1 位元 : 0 0 1 1
 *   PxM2 位元 : 0 1 0 1
 *   模式      : 準雙向 推挽 僅輸入 開漏
 *
 *   準雙向 (Quasi-bidirectional): 預設值,可輸入也可輸出,驅動力弱,適合一般用途
 *   推挽   (Push-Pull)         : 強力輸出,驅動 LED 建議用這個
 *   僅輸入 (Input-only)        : 高阻抗,讀取按鈕/類比訊號用
 *   開漏   (Open-Drain)        : 需外接上拉電阻,適合 I2C 等匯流排
 */

/* Timed Access 解鎖巨集:寫入受 TA 保護的暫存器前,必須「連續」執行這兩行 */
#define TA_UNLOCK()  do { TA = 0xAA; TA = 0x55; } while (0)

#endif /* __N76E003_H__ */
