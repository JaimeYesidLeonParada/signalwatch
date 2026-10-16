\# Smartwatch ESP32-C6 (Waveshare 1.69") para personas con discapacidad auditiva

ESP-IDF v6.0. Todo el código en main/. CMakeLists.txt usa lista explícita de SRCS

(al crear o mover un .c hay que agregarlo ahí).



\## Reglas de trabajo

\- Un paso a la vez, cambios pequeños. No avanzar hasta que yo confirme que corre en el reloj.

\- Si hay una decisión de diseño, preguntarme antes de implementar.

\- Un refactor NO cambia el comportamiento visible.

\- Verificar con `idf.py build` después de cada paso. Un commit por paso.

\- Solo ui\_task dibuja en pantalla (LVGL). Todo llega a la UI por la cola de ui.c.

\- Texto de pantalla sin tildes ni "¡" (la fuente no las tiene).



\## Módulos

capture\_task, classify\_task, ui\_task, clock\_task, battery\_task, vibrator\_task.

ai\_module (captura y clasificación), ei\_wrapper (solo corre el modelo),

alert\_module (ventana estadística), vibrator\_module (morse), battery\_module,

rtc\_module (PCF85063), ui (LVGL, cola FIFO).

