# -*- coding: utf-8 -*-
import re, sys
p='/data/k530/k530/keymaps/default/bluetooth.c'
s=open(p,encoding='utf-8').read()
orig=s

# ---- 1) A5 builder + sender, inserted right after keyboard builder ----
anchor1='    out[20] = k530_checksum(out, 20);\n}'
assert s.count(anchor1)==1, 'anchor1 count=%d'%s.count(anchor1)
A5_BLOCK='''

// ============================================================================
// A5 \u2014 \u043a\u0430\u0434\u0440 \"\u043d\u0430\u043c\u0435\u0440\u0435\u043d\u0438\u044f \u0441\u043e\u0435\u0434\u0438\u043d\u0435\u043d\u0438\u044f\" (advertise/reconnect) \u2014 \u043d\u0435\u0434\u043e\u0441\u0442\u0430\u044e\u0449\u0438\u0439 \u043a\u0443\u0441\u043e\u043a \u0441\u0442\u043e\u043a\u0430.
// \u0421\u0442\u043e\u043a (FUN_000002ac) \u0441\u0442\u0440\u043e\u0438\u0442 \u0435\u0433\u043e \u0438 \u043e\u0442\u0434\u0430\u0451\u0442 \u043c\u043e\u0434\u0443\u043b\u044e; \u0448\u043b\u0451\u0442 \u043f\u0435\u0440\u0438\u043e\u0434\u0438\u0447\u0435\u0441\u043a\u0438,
// \u043f\u043e\u043a\u0430 BT \u0412\u041a\u041b\u042e\u0427\u0401\u041d (\u0433\u0435\u0439\u0442 FUN_000042de: BB==2). \u041d\u0430 OFF \u0441\u0442\u043e\u043a \u043f\u0435\u0440\u0435\u0441\u0442\u0430\u0451\u0442
// \u0435\u0433\u043e \u0441\u043b\u0430\u0442\u044c \u2014 \u0438\u043c\u0435\u043d\u043d\u043e \u044d\u0442\u043e \u0433\u0430\u0441\u0438\u0442 \u0440\u0435\u043a\u043b\u0430\u043c\u0443/\u0430\u0432\u0442\u043e-\u0440\u0435\u043a\u043e\u043d\u043d\u0435\u043a\u0442 (\u0441\u043c. log23).
// \u0420\u0430\u043d\u044c\u0448\u0435 \u043c\u044b A5 \u043d\u0435 \u0441\u043b\u0430\u043b\u0438 \u0432\u043e\u043e\u0431\u0449\u0435 => \u043c\u043e\u0434\u0443\u043b\u044c \u0440\u0435\u043a\u043e\u043d\u043d\u0435\u043a\u0442\u0438\u043b\u0441\u044f \u0441\u0430\u043c \u043f\u043e \u0431\u043e\u043d\u0434\u0443.
//
// \u0420\u0430\u0441\u043a\u043b\u0430\u0434\u043a\u0430 (\u043f\u043e\u0434\u0442\u0432\u0435\u0440\u0436\u0434\u0435\u043d\u0430 RE, FUN_000002ac):
//   [0]=0xA5 (BC-\u043e\u043f\u043a\u043e\u0434)  [1]=mode (BT on->2, off->1)  [2]=B6  [3]=C2
//   [4]=0x0A  [5]=0x19  [6]=C3(=0)  [7]=BB  [8..19]=0  [20]=checksum
//   advertise: C2=0, BB=2 ; reconnect: C2=3, BB=1
#define K530_CMD_LINK_INTENT 0xA5u
// \u041f\u0435\u0440\u0438\u043e\u0434 keep-alive \u0432 \u0432\u044b\u0437\u043e\u0432\u0430\u0445 bluetooth_task() \u2014 \u043f\u043e\u0434\u0431\u0435\u0440\u0451\u0442\u0441\u044f \u043f\u043e \u0447\u0430\u0441\u0442\u043e\u0442\u0435 \u0432\u044b\u0437\u043e\u0432\u0430.
#ifndef K530_A5_KEEPALIVE_PERIOD
#define K530_A5_KEEPALIVE_PERIOD 500u
#endif

// \u041d\u0430\u043c\u0435\u0440\u0435\u043d\u0438\u0435: 2 = advertise (\u0440\u0435\u043a\u043b\u0430\u043c\u0430), 1 = reconnect \u043f\u043e \u0431\u043e\u043d\u0434\u0443.
static volatile uint8_t k530_link_bb = 2u;

static void k530_build_a5_frame(uint8_t out[K530_FRAME_LEN], uint8_t bb) {
    memset(out, 0, K530_FRAME_LEN);
    out[0] = K530_CMD_LINK_INTENT;
    out[1] = k530_bt_enabled_stable ? 0x02u : 0x01u;
    out[2] = 0x00u;                      // B6 \u2014 \u0442\u043e\u0447\u043d\u0430\u044f \u0441\u0435\u043c\u0430\u043d\u0442\u0438\u043a\u0430 \u043d\u0435 \u043f\u043e\u0434\u0442\u0432\u0435\u0440\u0436\u0434\u0435\u043d\u0430, 0
    out[3] = (bb == 1u) ? 0x03u : 0x00u; // C2: reconnect=3, advertise=0
    out[4] = 0x0Au;
    out[5] = 0x19u;
    out[6] = 0x00u;                      // C3
    out[7] = bb;                         // BB: 2=advertise, 1=reconnect
    // [8..19] \u043d\u0443\u043b\u0438. \u0421\u043b\u043e\u0442 \u0445\u043e\u0441\u0442\u0430 (draconic-N) \u0432 \u044d\u0442\u043e\u043c \u043a\u0430\u0434\u0440\u0435 \u041d\u0415\n    // \u043f\u0435\u0440\u0435\u0434\u0430\u0451\u0442\u0441\u044f: \u0441\u0442\u043e\u043a \u0437\u0430\u0434\u0430\u0451\u0442 \u0435\u0433\u043e \u0438\u043d\u0434\u0435\u043a\u0441\u043e\u043c 0x20000013, \u043a\u043e\u0442\u043e\u0440\u044b\u0439
    // \u0433\u0435\u0439\u0442\u0438\u0442 \u043e\u0442\u0434\u0435\u043b\u044c\u043d\u0443\u044e \u043a\u043e\u043c\u0430\u043d\u0434\u0443 A6/\u043f\u0440\u043e\u0444\u0438\u043b\u044c (\u043f\u0440\u043e\u0444\u0438\u043b\u0438 \u0432 \u0434\u0430\u043c\u043f\u0435 \u043f\u0443\u0441\u0442\u044b\u0435).
    out[20] = k530_checksum(out, 20);
}

// \u0417\u0430\u0433\u0440\u0443\u0437\u0438\u0442\u044c A5 \u0432 TX \u0438 \u0434\u0451\u0440\u043d\u0443\u0442\u044c strobe \u2014 \u0442\u043e\u043b\u044c\u043a\u043e \u0435\u0441\u043b\u0438 BT \u0432\u043a\u043b\u044e\u0447\u0451\u043d
// \u0438 SPI0 \u0433\u043e\u0442\u043e\u0432. \u041d\u0430 OFF \u0441\u044e\u0434\u0430 \u043d\u0435 \u0437\u0430\u0445\u043e\u0434\u0438\u043c (\u0433\u0435\u0439\u0442 \u0432 bluetooth_task).
static void k530_send_a5(uint8_t bb) {
    if (!k530_bt_enabled_stable || !k530_spi0_slave_ready) return;
    uint8_t frame[K530_FRAME_LEN];
    k530_build_a5_frame(frame, bb);
    k530_spi0_load_tx_buffer(frame, K530_FRAME_LEN);
    k530_bt_pulse_ready();
}'''
s=s.replace(anchor1, anchor1+A5_BLOCK, 1)

# ---- 2) ON-branch: immediately advertise on toggle ON ----
anchor2='            k530_bt_active = true;'
assert s.count(anchor2)==1, 'anchor2 count=%d'%s.count(anchor2)
repl2=('            k530_bt_active = true;\n'
       '            k530_link_bb = 2u;          // \u043f\u0440\u0438 \u0432\u043a\u043b\u044e\u0447\u0435\u043d\u0438\u0438 \u2014 \u0440\u0435\u0436\u0438\u043c \u0440\u0435\u043a\u043b\u0430\u043c\u044b\n'
       '            k530_send_a5(2u);           // \u0441\u0440\u0430\u0437\u0443 \u0437\u0430\u044f\u0432\u0438\u0442\u044c \u043d\u0430\u043c\u0435\u0440\u0435\u043d\u0438\u0435 \u0441\u043e\u0435\u0434\u0438\u043d\u0435\u043d\u0438\u044f (\u043a\u0430\u043a \u0441\u0442\u043e\u043a)')
s=s.replace(anchor2, repl2, 1)

# ---- 3) periodic keep-alive block inserted before the keep-alive TODO ----
marker='periodic keep-alive'
i=s.index(marker)
line_start=s.rfind('\n',0,i)+1
KA_BLOCK=('    // A5 keep-alive: \u043f\u043e\u043a\u0430 BT \u0432\u043a\u043b\u044e\u0447\u0451\u043d, \u043f\u0435\u0440\u0438\u043e\u0434\u0438\u0447\u0435\u0441\u043a\u0438 \u0448\u043b\u0451\u043c \"\u043d\u0430\u043c\u0435\u0440\u0435\u043d\u0438\u0435\n'
          '    // \u0441\u043e\u0435\u0434\u0438\u043d\u0435\u043d\u0438\u044f\" (\u043a\u0430\u043a \u0441\u0442\u043e\u043a). \u041d\u0430 OFF \u0421\u042e\u0414\u0410 \u041d\u0415 \u0417\u0410\u0425\u041e\u0414\u0418\u041c => \u0440\u0435\u043a\u043b\u0430\u043c\u0430/\u0440\u0435\u043a\u043e\u043d\u043d\u0435\u043a\u0442 \u0433\u0430\u0441\u043d\u0443\u0442.\n'
          '    if (k530_bt_active && k530_bt_enabled_stable && k530_spi0_slave_ready) {\n'
          '        static uint16_t k530_a5_tick = 0;\n'
          '        if (++k530_a5_tick >= K530_A5_KEEPALIVE_PERIOD) {\n'
          '            k530_a5_tick = 0;\n'
          '            k530_send_a5(k530_link_bb);\n'
          '        }\n'
          '    }\n\n')
s=s[:line_start]+KA_BLOCK+s[line_start:]

assert s!=orig
open(p,'w',encoding='utf-8').write(s)
print('OK open=%d close=%d'%(s.count('{'),s.count('}')))
