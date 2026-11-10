#ifndef BUTTON_MODULE_H
#define BUTTON_MODULE_H

// Se llama (desde la tarea del boton) cada vez que se pulsa el boton.
typedef void (*button_cb_t)(void);

// Configura el pin del botón y lanza la tarea de lectura. on_press puede ser NULL.
void button_init(button_cb_t on_press);

#endif // BUTTON_MODULE_H