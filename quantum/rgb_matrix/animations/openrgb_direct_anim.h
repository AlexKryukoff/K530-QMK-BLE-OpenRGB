#ifdef OPENRGB_ENABLE
RGB_MATRIX_EFFECT(OPENRGB_DIRECT)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

bool OPENRGB_DIRECT(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

#        ifndef OPENRGB_DIRECT_MODE_UNBUFFERED
    for (uint8_t i = led_min; i < led_max; i++) {
        // K530: LED 61/62 — статусные индикаторы у тумблера (host-линк/батарея).
        // Их красит ТОЛЬКО rgb_matrix_indicators_advanced_user в keymap.c.
        // Пропускаем их здесь, чтобы direct-эффект не перебивал индикатор каждый
        // кадр (иначе асинхронный PWM-скан ловит промежуточный кадр -> мерцание).
        if (i == 61 || i == 62) continue;
#            ifdef OPENRGB_DIRECT_MODE_USE_UNIVERSAL_BRIGHTNESS
        float brightness = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
        rgb_matrix_set_color(i, brightness * g_openrgb_direct_mode_colors[i].r, brightness * g_openrgb_direct_mode_colors[i].g, brightness * g_openrgb_direct_mode_colors[i].b);
#            else
        rgb_matrix_set_color(i, g_openrgb_direct_mode_colors[i].r, g_openrgb_direct_mode_colors[i].g, g_openrgb_direct_mode_colors[i].b);
#            endif
    }
#        endif

    return led_max < DRIVER_LED_TOTAL;
}
#    endif
#endif
