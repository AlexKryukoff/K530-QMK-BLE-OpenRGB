--[[
    Skydimo controller plugin for Redragon K530 (SonixQMK, sn32_master_openrgb,
    OpenRGB community protocol, https://gitlab.com/OpenRGBDevelopers/QMK-OpenRGB).

    Протокол взят ДОСЛОВНО из файлов, присланных пользователем из его текущей
    сборки прошивки: quantum/openrgb.c и quantum/openrgb.h. Значения команд,
    смещения байт и размеры пакетов ниже не придуманы — переписаны построчно
    из этих файлов.

    API Skydimo (device:write/device:read/device:output_led_count/
    device:get_rgb_bytes) ПОДТВЕРЖДЁН по реальному рабочему плагину
    controller.drgb_hid. Структура модуля (local plugin = {} / return plugin,
    device как ГЛОБАЛЬНАЯ переменная окружения, а не параметр функций)
    ПОДТВЕРЖДЕНА по второму рабочему образцу (FRENZY Keyboard plugin) —
    предыдущая версия использовала глобальные функции on_xxx(device) с
    device как параметром, что, вероятно, и было причиной, по которой
    плагин не регистрировался загрузчиком Skydimo вообще.

    Единственное, что осталось неподтверждённым — точная сигнатура
    device:add_output() в on_init: во ВТОРОМ образце (FRENZY) она уже
    встретилась (id/name/type/size/matrix/capabilities для "matrix"-типа
    выхода) — используем эту форму как более достоверную, чем предыдущая
    предположительная версия (id/x/y), но полной уверенности всё ещё нет,
    т.к. K530 не матричная по расположению LED в том же смысле (это RGB
    per-key на клавиатуре, не адресная лента) — см. TODO в on_init.

    ВАЖНО: это RAW HID (raw_hid_send/raw_hid_receive в прошивке), то есть
    Output/Input HID-репорты (interrupt-транзакции), а НЕ Get/Set Feature
    Report — подтверждено, что device:write()/device:read() работают
    именно с этим транспортом.
]]

local plugin = {}

-- ============================================================================
-- Протокол OpenRGB QMK (дословно из openrgb.h / openrgb.c)
-- ============================================================================

local RAW_EPSIZE = 64

local CMD = {
    GET_PROTOCOL_VERSION       = 1,
    GET_QMK_VERSION            = 2,
    GET_DEVICE_INFO            = 3,
    GET_MODE_INFO               = 4,
    GET_LED_INFO                = 5,
    GET_ENABLED_MODES           = 6,
    SET_MODE                     = 7,
    DIRECT_MODE_SET_SINGLE_LED   = 8,
    DIRECT_MODE_SET_LEDS         = 9,
}

local RESP = {
    FAILURE        = 25,
    SUCCESS        = 50,
    END_OF_MESSAGE = 100,
}

local EXPECTED_PROTOCOL_VERSION = 0x0D -- 13, см. OPENRGB_PROTOCOL_VERSION в openrgb.h

-- Единственный зарегистрированный output — строковый id, ОДИН И ТОТ ЖЕ
-- используется и в add_output() (on_init), и в output_led_count()/
-- get_rgb_bytes() (on_tick). Вынесено в константу намеренно: рассинхрон
-- этих двух мест (числовой id в одном месте, другой id/тип в другом) был
-- причиной нерабочей подсветки — подтверждено на двух независимых рабочих
-- примерах (FRENZY: id="keys", Skydimo SK0301: id="out1").
local OUTPUT_ID = "leds"

-- Максимум светодиодов в одном пакете DIRECT_MODE_SET_LEDS:
-- (64 - 2) / 4 = 15 (см. openrgb_direct_mode_set_leds в openrgb.c: data[1]=count,
-- затем по 4 байта на LED: [led_idx, r, g, b])
local MAX_LEDS_PER_SET_PACKET = 15

-- Максимум светодиодов в одном ответе GET_LED_INFO:
-- 7 байт/LED (x,y,flags,r,g,b,keycode), полезная область буфера ответа
-- начинается с индекса 1 и не должна залезать на data[63]=END_OF_MESSAGE,
-- поэтому безопасный предел: floor(62/7) = 8
local MAX_LEDS_PER_GET_INFO = 8

-- ============================================================================
-- Низкоуровневый HID ввод-вывод.
-- ============================================================================
-- device:write()/device:read() ПОДТВЕРЖДЕНЫ по рабочему плагину
-- controller.drgb_hid — имена и сигнатуры менять не нужно.
-- ПРИМЕЧАНИЕ: device здесь по-прежнему передаётся ПАРАМЕТРОМ во внутренние
-- вспомогательные функции модуля — это обычный Lua-стиль передачи данных
-- между локальными функциями одного файла и никак не противоречит тому,
-- что САМ Skydimo передаёт объект устройства плагину через глобальную
-- переменную `device`. Публичные же функции жизненного цикла
-- (plugin.on_validate/on_init/on_tick/on_shutdown, см. ниже) эту
-- глобальную `device` не принимают параметром, а берут напрямую из
-- окружения — именно так, как в обоих рабочих образцах.

local function hid_write(device, payload)
    -- payload — таблица чисел 0..255 длиной RAW_EPSIZE (64 байта).
    return device:write(payload)
end

local function hid_read(device, timeout_ms)
    -- Должна вернуть таблицу чисел 0..255 длиной RAW_EPSIZE (64 байта),
    -- или nil/false при таймауте/ошибке.
    return device:read(timeout_ms or 200)
end

-- Собирает 64-байтный пакет-запрос: cmd + произвольные доп. байты, остаток
-- забивается нулями (RAW_EPSIZE фиксированный размер и на приёме, и на
-- отправке — см. raw_hid_receive/raw_hid_send в прошивке).
local function build_packet(cmd, extra_bytes)
    local buf = {}
    buf[1] = cmd
    if extra_bytes then
        for i, b in ipairs(extra_bytes) do
            buf[i + 1] = b
        end
    end
    for i = (buf[#buf] and #buf or 1) + 1, RAW_EPSIZE do
        buf[i] = buf[i] or 0
    end
    return buf
end

-- Отправляет команду и (если ожидается ответ) читает его.
-- expect_response = false только для DIRECT_MODE_SET_LEDS (id=9) — это
-- единственная команда протокола, на которую прошивка НИЧЕГО не отвечает
-- (см. условие `if (*data != OPENRGB_DIRECT_MODE_SET_LEDS)` в openrgb.c).
local function send_command(device, cmd, extra_bytes, expect_response)
    local packet = build_packet(cmd, extra_bytes)
    local ok = hid_write(device, packet)
    if not ok then
        return nil, "hid_write failed"
    end
    if expect_response == false then
        return true
    end
    local resp = hid_read(device)
    if not resp then
        return nil, "hid_read timeout"
    end
    return resp
end

-- ============================================================================
-- Состояние плагина
-- ============================================================================

local state = {
    initialized = false,
    led_count = 0,
    matrix_size = 0,
    direct_mode_id = nil, -- узнаём динамически из GET_ENABLED_MODES
    led_positions = {},   -- [led_index] = {x=, y=}
}

---------------------------------------------------------------------------
-- Lifecycle callbacks
---------------------------------------------------------------------------

-- on_validate — хэндшейк: подтверждаем, что это действительно наша
-- прошивка с совместимой версией протокола OpenRGB. `device` берётся из
-- глобального окружения (инжектируется загрузчиком Skydimo), не параметр.
function plugin.on_validate()
    local resp, err = send_command(device, CMD.GET_PROTOCOL_VERSION, nil, true)
    if not resp then
        device:log("K530 OpenRGB: no response to GET_PROTOCOL_VERSION: " .. tostring(err))
        return false
    end

    -- resp[1] = эхо команды (1), resp[2] = версия протокола
    if resp[1] ~= CMD.GET_PROTOCOL_VERSION then
        device:log("K530 OpenRGB: unexpected command echo in response")
        return false
    end
    if resp[2] ~= EXPECTED_PROTOCOL_VERSION then
        device:log(string.format("K530 OpenRGB: protocol version mismatch: got 0x%02X, expected 0x%02X", resp[2], EXPECTED_PROTOCOL_VERSION))
        return false
    end

    device:log("K530 OpenRGB: validated (protocol version 0x" .. string.format("%02X", resp[2]) .. ")")
    return true
end

-- on_init — читаем инфо об устройстве, узнаём ID direct-режима, строим
-- карту светодиодов и переключаем прошивку в OpenRGB Direct mode.
-- Ошибки НЕ бросаются через error() (это могло рушить загрузчик плагина
-- целиком) — вместо этого логируются через device:log и функция мягко
-- завершается, оставляя state.initialized = false, как в рабочем образце.
function plugin.on_init()
    device:log("K530 OpenRGB: on_init ENTERED")

    -- 1) GET_DEVICE_INFO: узнаём реальное число LED и размер матрицы
    --    (не полагаемся на захардкоженные 63 — берём то, что реально
    --    сообщает прошивка)
    local dev_info = send_command(device, CMD.GET_DEVICE_INFO, nil, true)
    if not dev_info then
        device:log("K530 OpenRGB: GET_DEVICE_INFO failed")
        return
    end
    state.led_count   = dev_info[2]
    state.matrix_size = dev_info[3]

    -- 2) GET_ENABLED_MODES: последний ненулевой байт ответа — это ID режима
    --    "OpenRGB Direct" (RGB_MATRIX_OPENRGB_DIRECT), присваиваемый
    --    компилятором динамически — НЕ захардкожен нигде в прошивке,
    --    поэтому берём его строго из ответа, не угадываем число.
    local modes_resp = send_command(device, CMD.GET_ENABLED_MODES, nil, true)
    if not modes_resp then
        device:log("K530 OpenRGB: GET_ENABLED_MODES failed")
        return
    end
    local last_mode_byte = nil
    for i = 2, RAW_EPSIZE - 1 do
        if modes_resp[i] ~= 0 then
            last_mode_byte = modes_resp[i]
        else
            break
        end
    end
    state.direct_mode_id = last_mode_byte
    if not state.direct_mode_id then
        device:log("K530 OpenRGB: could not determine OpenRGB Direct mode id from GET_ENABLED_MODES response")
        return
    end

    -- 3) GET_LED_INFO постранично — строим карту позиций светодиодов.
    --    Координаты x,y приходят в "родных" единицах QMK LED point-space
    --    (обычно 0..224 по X, 0..64 по Y — стандартная сетка g_led_config
    --    в QMK). Нормализация под конкретный формат device:add_output()
    --    ниже не проверена отдельно — см. TODO там же.
    local first_led = 0
    while first_led < state.led_count do
        local count = math.min(MAX_LEDS_PER_GET_INFO, state.led_count - first_led)
        local resp = send_command(device, CMD.GET_LED_INFO, { first_led, count }, true)
        if not resp then
            device:log("K530 OpenRGB: GET_LED_INFO failed at led " .. first_led)
            return
        end
        for i = 0, count - 1 do
            local data_idx = i * 7
            state.led_positions[first_led + i] = {
                x = resp[data_idx + 2],
                y = resp[data_idx + 3],
            }
        end
        first_led = first_led + count
    end

    -- 4) Регистрируем выход в Skydimo.
    -- TODO(СВЕРИТЬ): во втором рабочем образце (FRENZY) используется форма
    -- {id=, name=, type="matrix", size=, matrix={width=,height=,map=},
    -- capabilities={...}} — используем её как более достоверную, чем
    -- предыдущий вариант (id/x/y по отдельности). "map" — плоский массив
    -- индексов LED по строкам матрицы width x height; для K530 такой карты
    -- у нас нет отдельно, строим её из уже полученных led_positions
    -- (x,y) простым переводом в сетку MATRIX_COLS x MATRIX_ROWS
    -- (matrix_size из GET_DEVICE_INFO). Это лучшее доступное приближение,
    -- не 100%-но проверенное на реальном устройстве.
    local ok, err = pcall(function()
        local map = {}
        for i = 0, state.led_count - 1 do
            map[i + 1] = i -- без реальной раскладки строк/столбцов используем
                             -- линейный порядок как есть; при необходимости
                             -- заменить на реальную карту матрицы клавиатуры
        end
        device:add_output({
            id     = OUTPUT_ID,
            name   = "Redragon K530",
            type   = "matrix",
            size   = state.led_count,
            matrix = {
                width  = state.led_count,
                height = 1,
                map    = map,
            },
            capabilities = {
                editable = false,
                min_total_leds = state.led_count,
                max_total_leds = state.led_count,
                allowed_total_leds = { state.led_count },
            },
        })
    end)
    if not ok then
        device:log("K530 OpenRGB: add_output FAILED: " .. tostring(err))
        return
    end
    device:log("K530 OpenRGB: add_output OK")

    -- 5) Переключаем прошивку в OpenRGB Direct mode (save=0, чтобы не
    -- писать в EEPROM при каждом запуске приложения — команда 7,
    -- см. openrgb_set_mode: data = [h, s, v, mode, speed, save]).
    local mode_resp = send_command(device, CMD.SET_MODE, { 0, 0, 255, state.direct_mode_id, 0, 0 }, true)
    if not mode_resp then
        device:log("K530 OpenRGB: SET_MODE (direct) failed (no response)")
        return
    end
    if mode_resp[RAW_EPSIZE - 1] == RESP.FAILURE then
        -- resp[RAW_EPSIZE-1] соответствует data[RAW_EPSIZE-2] в 0-индексации
        -- прошивки (Lua-таблицы 1-индексированы) — см. openrgb_set_mode:
        -- raw_hid_buffer[RAW_EPSIZE - 2] = OPENRGB_SUCCESS/FAILURE
        device:log("K530 OpenRGB: SET_MODE returned FAILURE")
        return
    end

    state.initialized = true
    device:log(string.format("K530 OpenRGB: initialized — %d LEDs, direct_mode_id=%d", state.led_count, state.direct_mode_id))
end

-- on_tick — периодическая отправка текущих цветов светодиодов.
-- Источник цветов ПОДТВЕРЖДЁН по рабочему плагину controller.drgb_hid:
--   device:output_led_count(output_id) — сколько физических LED в этом output
--   device:get_rgb_bytes(output_id)    — готовая строка RGB-байт для output
-- ИСПРАВЛЕНО: раньше здесь был цикл `for output_id = 0, state.led_count-1`,
-- трактующий КАЖДЫЙ индекс LED как отдельный output_id — но в on_init
-- регистрируется РОВНО ОДИН output с id=OUTPUT_ID ("leds"), а не 63
-- отдельных. Из-за этого device:output_led_count(0), device:output_led_count(1)
-- и т.д. обращались к несуществующим outputs и ничего не возвращали —
-- поэтому подсветка не менялась. Теперь запрашиваем ВСЕ цвета ОДНИМ
-- вызовом по единственному зарегистрированному id.
-- Сигнатура (dt) без device — device глобальный, как в обоих образцах.
function plugin.on_tick(dt)
    if not state.initialized then
        return
    end

    local n = device:output_led_count(OUTPUT_ID)
    if not n or n <= 0 then
        return
    end

    local bytes = device:get_rgb_bytes(OUTPUT_ID)
    if not bytes then
        return
    end

    -- bytes — строка длиной n*3 байт, по 3 (R,G,B) на LED, в порядке,
    -- заданном полем "map" при регистрации output (см. on_init) — у нас
    -- map линейный (map[i+1]=i), так что k-тый триплет соответствует
    -- LED с индексом k в терминах прошивки (0-based).
    local count_available = math.min(n, state.led_count)

    local i = 0
    while i < count_available do
        local chunk = math.min(MAX_LEDS_PER_SET_PACKET, count_available - i)
        local extra = { chunk }
        for j = 0, chunk - 1 do
            local led_idx = i + j
            local r = string.byte(bytes, led_idx * 3 + 1) or 0
            local g = string.byte(bytes, led_idx * 3 + 2) or 0
            local b = string.byte(bytes, led_idx * 3 + 3) or 0
            table.insert(extra, led_idx)
            table.insert(extra, r)
            table.insert(extra, g)
            table.insert(extra, b)
        end
        send_command(device, CMD.DIRECT_MODE_SET_LEDS, extra, false)
        i = i + chunk
    end
end

-- on_shutdown — гасим подсветку при закрытии/отключении плагина.
function plugin.on_shutdown()
    if not state.initialized then
        return
    end

    local i = 0
    while i < state.led_count do
        local count = math.min(MAX_LEDS_PER_SET_PACKET, state.led_count - i)
        local extra = { count }
        for j = 0, count - 1 do
            local led_idx = i + j
            table.insert(extra, led_idx)
            table.insert(extra, 0)
            table.insert(extra, 0)
            table.insert(extra, 0)
        end
        send_command(device, CMD.DIRECT_MODE_SET_LEDS, extra, false)
        i = i + count
    end

    device:log("K530 OpenRGB: shutdown, LEDs cleared")
end

return plugin
