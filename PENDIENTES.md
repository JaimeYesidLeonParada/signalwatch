# Features pendientes

Cosas que se dejaron a propósito fuera del refactor SOLID (el refactor no cambia el
comportamiento visible). Cada una necesita su propio paso, con decisión de diseño previa.

## 1. Ajuste manual de fecha y hora por el usuario

- **Qué:** que el usuario pueda poner la hora y la fecha del reloj desde el propio dispositivo.
- **Estado hoy:** `rtc_set_time` (`main/rtc_module.c`) existe pero no tiene llamadores.
  No comprueba el resultado de `i2c_master_transmit` (variable `err` sin usar, warning de
  compilación en `rtc_module.c:86`) y no valida los argumentos.
- **Al implementarlo:**
  - Comprobar el error de escritura y avisar al usuario si falla.
  - Validar rangos de entrada (mes 1-12, día 1-31, dow 0-6, hora 0-23, min 0-59), igual
    que ya hace `rtc_get_time` al leer.
  - Definir cómo llega el dato (pantalla táctil, botón, BLE) y quién llama a `rtc_set_time`,
    manteniendo la regla de que los módulos se conectan en `app_init.c`.

## 2. Configuración manual de la batería

- **Qué:** exponer al usuario lo que haga falta de la batería (por ejemplo ver el porcentaje a
  demanda o ajustar umbrales) en lugar de tenerlo fijo en el código.
- **Estado hoy:** `battery_get_percentage` (`main/battery_module.c`) solo la usa `battery_task`
  internamente. Los valores `CHARGING_PCT_THRESHOLD` (98), `BATTERY_MAX_VOLTAGE` y el
  suavizado (0.9 / 0.1) están como constantes.
- **Al implementarlo:** decidir qué se configura de verdad antes de exponer nada; la
  heurística de "cargando" (>= 98 %) es empírica y es el candidato más claro.

## 3. Carrera sin lock de LVGL en `touch_init`

- **Qué:** `touch_init` (`main/touch_module.c`) llama `lv_indev_create` y `lv_indev_set_*`
  sin tomar `lvgl_port_lock`, aunque la tarea de LVGL ya está corriendo.
- **Estado hoy:** existe desde antes del refactor y no se ha visto fallar. En el punto 1
  del repaso final se decidió no tocarlo para no mezclarlo con un cambio de comportamiento.
- **Al implementarlo:** envolver el registro del indev con `lvgl_port_lock(-1)` /
  `lvgl_port_unlock()`, y probar el arranque varias veces.
