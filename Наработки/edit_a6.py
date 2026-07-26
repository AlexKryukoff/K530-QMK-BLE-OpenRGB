# -*- coding: utf-8 -*-
import io
p='/data/k530/k530/keymaps/default/bluetooth.c'
s=io.open(p,encoding='utf-8').read()
orig=s

def repl(old,new,n=1):
    global s
    c=s.count(old)
    assert c>=n, 'anchor not found (%d): %r'%(c, old[:60])
    s=s.replace(old,new,n)

# 1) K530_CMD_FRAME_LEN near top (must precede tx buffer/ISR uses)
repl('#define K530_FRAME_LEN 21u',
     '#define K530_FRAME_LEN 21u\n'
     '// \u041f\u043e\u043b\u043d\u0430\u044f \u0434\u043b\u0438\u043d\u0430 \u0441\u0442\u043e\u043a\u043e\u0432\u043e\u0433\u043e \u041a\u041e\u041c\u0410\u041d\u0414\u041d\u041e\u0413\u041e \u043a\u0430\u0434\u0440\u0430 (A5/A6): 39 \u0431\u0430\u0439\u0442; \u0447\u0435\u043a-\u0441\u0443\u043c\u043c\u0430 \u0432 [0x26]=38.\n'
     '// RX-\u0444\u0440\u0435\u0439\u043c\u0438\u043d\u0433 \u0438 \u043a\u0430\u0434\u0440 \u043a\u043b\u0430\u0432\u0438\u0430\u0442\u0443\u0440\u044b \u043e\u0441\u0442\u0430\u044e\u0442\u0441\u044f 21-\u0431\u0430\u0439\u0442\u043d\u044b\u043c\u0438 (K530_FRAME_LEN).\n'
     '#define K530_CMD_FRAME_LEN 39u')

# 2) enlarge TX buffer
repl('static volatile uint8_t k530_tx_frame[K530_FRAME_LEN];',
     'static volatile uint8_t k530_tx_frame[K530_CMD_FRAME_LEN];')

# 3) load fn: cap to CMD len + zero-pad whole buffer, then copy
repl('    if (len > K530_FRAME_LEN) len = K530_FRAME_LEN;\n    memcpy((void *)k530_tx_frame, frame, len);',
     '    if (len > K530_CMD_FRAME_LEN) len = K530_CMD_FRAME_LEN;\n'
     '    memset((void *)k530_tx_frame, 0, K530_CMD_FRAME_LEN);\n'
     '    memcpy((void *)k530_tx_frame, frame, len);')

# 4) ISR TX index bound -> allow clocking out full 39-byte command frames
repl('        SPI0_REG_DATA = (tx_idx < K530_FRAME_LEN) ? k530_tx_frame[tx_idx] : 0;',
     '        SPI0_REG_DATA = (tx_idx < K530_CMD_FRAME_LEN) ? k530_tx_frame[tx_idx] : 0;')

# 5) host-intent opcode define next to link-intent
repl('#define K530_CMD_LINK_INTENT 0xA5u',
     '#define K530_CMD_LINK_INTENT 0xA5u\n#define K530_CMD_HOST_INTENT 0xA6u')

# 6) rewrite A5 builder (39-byte, checksum @38) - replace whole function by boundaries
st=s.index('static void k530_build_a5_frame(')
en=s.index('\n}\n', st)+3
new_a5=(
'static void k530_build_a5_frame(uint8_t out[K530_CMD_FRAME_LEN], uint8_t bb) {\n'
'    memset(out, 0, K530_CMD_FRAME_LEN);\n'
'    out[0] = K530_CMD_LINK_INTENT;                    // 0xA5\n'
'    out[1] = k530_bt_enabled_stable ? 0x02u : 0x01u;  // stock: P1.14?1:2 (\u0430\u043f\u043f\u0440\u043e\u043a\u0441. mode)\n'
'    out[2] = 0x00u;                                   // B6\n'
'    out[3] = (bb == 1u) ? 0x03u : 0x00u;              // C2: reconnect=3, advertise=0\n'
'    out[4] = 0x0Au;\n'
'    out[5] = 0x19u;\n'
'    out[6] = 0x00u;                                   // C3\n'
'    out[7] = bb;                                      // BB: 2=advertise, 1=reconnect\n'
'    // \u041f\u043e\u043b\u043d\u044b\u0439 \u0441\u0442\u043e\u043a\u043e\u0432\u044b\u0439 \u043a\u0430\u0434\u0440 = 39 \u0431\u0430\u0439\u0442, \u0447\u0435\u043a-\u0441\u0443\u043c\u043c\u0430 (\u0441\u0443\u043c\u043c\u0430 [0..37]) \u0432 [0x26]=38.\n'
'    out[38] = k530_checksum(out, 38);\n'
'}\n')
s=s[:st]+new_a5+s[en:]

# 7) rewrite send_a5 + append A6 builder/sender
st=s.index('static void k530_send_a5(')
en=s.index('\n}\n', st)+3
new_send=(
'static void k530_send_a5(uint8_t bb) {\n'
'    if (!k530_bt_enabled_stable || !k530_spi0_slave_ready) return;\n'
'    uint8_t frame[K530_CMD_FRAME_LEN];\n'
'    k530_build_a5_frame(frame, bb);\n'
'    k530_spi0_load_tx_buffer(frame, K530_CMD_FRAME_LEN);\n'
'    k530_bt_pulse_ready();\n'
'}\n'
'\n'
'// --- \u041a\u043e\u043c\u0430\u043d\u0434\u0430 A6: \u0441\u043c\u0435\u043d\u0430 \u0430\u043a\u0442\u0438\u0432\u043d\u043e\u0433\u043e \u0445\u043e\u0441\u0442\u0430/\u0438\u043c\u0435\u043d\u0438 (draconic-N) --------------\n'
'// \u0421\u0442\u043e\u043a\u043e\u0432\u044b\u0439 \u0441\u0435\u0440\u0438\u0430\u043b\u0438\u0437\u0430\u0442\u043e\u0440 FUN_000002AC, \u0432\u0435\u0442\u043a\u0430 0x0336:\n'
'//   [0]=0xA6  [1]=P1.14?1:2  [2]=B6(0)  [3]=0xBE(=2)  [4]=\u0438\u043d\u0434\u0435\u043a\u0441 \u0445\u043e\u0441\u0442\u0430 (\u043f\u0440\u043e\u0444\u0438\u043b\u044c[0x15])\n'
'//   ... \u043d\u0443\u043b\u0438 ... [0x26]=38 = \u0447\u0435\u043a-\u0441\u0443\u043c\u043c\u0430 (\u0441\u0443\u043c\u043c\u0430 [0..37]).\n'
'// \u0418\u043c\u044f draconic-N \u0438 \u0431\u043e\u043d\u0434 \u043f\u043e \u0438\u043d\u0434\u0435\u043a\u0441\u0443 \u0434\u0435\u043b\u0430\u0435\u0442 \u0441\u0430\u043c \u043c\u043e\u0434\u0443\u043b\u044c; \u043c\u044b \u043b\u0438\u0448\u044c \u0441\u043e\u043e\u0431\u0449\u0430\u0435\u043c \u0438\u043d\u0434\u0435\u043a\u0441.\n'
'static void k530_build_a6_frame(uint8_t out[K530_CMD_FRAME_LEN], uint8_t host_index) {\n'
'    memset(out, 0, K530_CMD_FRAME_LEN);\n'
'    out[0] = K530_CMD_HOST_INTENT;                    // 0xA6\n'
'    out[1] = k530_bt_enabled_stable ? 0x02u : 0x01u;\n'
'    out[2] = 0x00u;                                   // B6\n'
'    out[3] = 0x02u;                                   // 0xBE = 2 (\u043a\u043e\u043d\u0441\u0442\u0430\u043d\u0442\u0430 \u0441\u0442\u043e\u043a\u0430)\n'
'    out[4] = host_index;                              // 0/1/2 -> draconic-1/2/3\n'
'    out[38] = k530_checksum(out, 38);\n'
'}\n'
'\n'
'static void k530_send_a6(uint8_t host_index) {\n'
'    if (!k530_bt_enabled_stable || !k530_spi0_slave_ready) return;\n'
'    uint8_t frame[K530_CMD_FRAME_LEN];\n'
'    k530_build_a6_frame(frame, host_index);\n'
'    k530_spi0_load_tx_buffer(frame, K530_CMD_FRAME_LEN);\n'
'    k530_bt_pulse_ready();\n'
'}\n')
s=s[:st]+new_send+s[en:]

# 8) wire A6 into host-slot change handler (insert after FIRST indicator call)
anchor='        k530_set_host_indicator_led((uint8_t)k530_host_slot_stable);'
i=s.index(anchor)
ins=(anchor+'\n'
'        // \u0421\u043e\u043e\u0431\u0449\u0430\u0435\u043c \u043c\u043e\u0434\u0443\u043b\u044e \u0430\u043a\u0442\u0438\u0432\u043d\u044b\u0439 \u0445\u043e\u0441\u0442 \u043f\u043e \u0438\u043d\u0434\u0435\u043a\u0441\u0443 (0/1/2) -> draconic-N\n'
'        k530_send_a6(k530_host_slot_stable ? (uint8_t)(k530_host_slot_stable - 1u) : 0u);')
s=s[:i]+ins+s[i+len(anchor):]

# 9) wire A6 into ON branch (after immediate A5 advertise)
anchor2='            k530_send_a5(2u);'
i2=s.index(anchor2)
eol=s.index('\n', i2)
ins2='\n            k530_send_a6(k530_host_slot_stable ? (uint8_t)(k530_host_slot_stable - 1u) : 0u); // \u0430\u043a\u0442\u0438\u0432\u043d\u044b\u0439 \u0445\u043e\u0441\u0442'
s=s[:eol]+ins2+s[eol:]

assert s!=orig
io.open(p,'w',encoding='utf-8').write(s)
print('edits applied; new length lines=%d'%(s.count(chr(10))+1))
print('A6 builder present:', 'k530_build_a6_frame' in s, ' A6 sender:', 'k530_send_a6' in s)
print('CMD_FRAME_LEN uses:', s.count('K530_CMD_FRAME_LEN'))
