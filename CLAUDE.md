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

\- Los módulos no se conocen entre sí: se conectan en app\_init.c (callbacks o parámetros).

Un módulo nuevo no incluye ui.h ni otros módulos de lógica (salvo alert\_dispatcher, que es el pegamento).



\## Módulos

Tareas: capture\_task, classify\_task, ui\_task, clock\_task, battery\_task, vibrator\_task.

Arranque: app\_init.c es el composition root. Es el único que conoce todos los
módulos y los conecta (callbacks y handles por parámetro). El orden de init está
comentado paso a paso ahí.

\- ai\_module: captura y clasificación. Avisa por callbacks (ai\_listener\_t), no conoce la UI.

\- ei\_wrapper: solo corre el modelo.

\- alert\_module: ventana estadística.

\- alert\_catalog: datos por clase (label, morse, glifo, textos). Agregar una clase = una fila.

\- alert\_dispatcher: traduce los eventos de ai\_module a UI y vibración.

\- vibrator\_module (morse), battery\_module, rtc\_module (PCF85063): entregan datos por
  callback, no incluyen ui.

\- display\_hw: hardware de pantalla (SPI, ST7789) y LVGL. i2c\_bus: bus I2C compartido.

\- ui: cola FIFO y ui\_task, que es el único que dibuja. Los widgets están en ui\_clock,
  ui\_battery, ui\_vu, ui\_notification y ui\_alerts.

\- bsp\_es8311 (audio), touch\_module, button\_module.
