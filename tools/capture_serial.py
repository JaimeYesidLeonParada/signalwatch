#!/usr/bin/env python3
"""Guarda en un archivo todo lo que el reloj escribe por el puerto serie.

El monitor de la terminal solo recuerda las ultimas lineas, y el volcado de audio
(tools/audiodump_to_wav.py) son cientos de lineas por ventana: se perderia. Este programa las
guarda todas en un archivo.

    python tools/capture_serial.py --list              # muestra los puertos disponibles
    python tools/capture_serial.py COM7 monitor.log    # guarda lo que llega; Ctrl+C para parar

Antes de usarlo hay que CERRAR el monitor de ESP-IDF: dos programas no pueden abrir el mismo
puerto a la vez. Necesita pyserial, que ya viene en el entorno de ESP-IDF.
"""
import sys

import serial
import serial.tools.list_ports

BAUD = 115200


def main():
    if len(sys.argv) >= 2 and sys.argv[1] == "--list":
        for p in serial.tools.list_ports.comports():
            print(f"{p.device}: {p.description}")
        return 0
    if len(sys.argv) < 3:
        print(__doc__)
        return 1

    port, out_path = sys.argv[1], sys.argv[2]
    ser = serial.Serial()
    ser.port = port
    ser.baudrate = BAUD
    ser.timeout = 1
    # Sin tocar DTR/RTS: con algunos adaptadores, abrirlos reinicia el reloj.
    ser.dtr = False
    ser.rts = False
    try:
        ser.open()
    except serial.SerialException as e:
        print(f"No se pudo abrir {port}: {e}")
        print("Cierra el monitor de ESP-IDF (u otro programa que use el puerto) y prueba otra vez.")
        return 1

    lines = dump_lines = 0
    print(f"Guardando {port} en {out_path}. Ctrl+C para parar.")
    try:
        with open(out_path, "w", encoding="utf-8", newline="\n") as f:
            while True:
                raw = ser.readline()
                if not raw:
                    continue
                text = raw.decode("utf-8", errors="replace").rstrip("\r\n")
                f.write(text + "\n")
                f.flush()
                lines += 1
                if text.startswith("AD:"):
                    dump_lines += 1
                    if text.startswith("AD:BEGIN") or text.startswith("AD:END"):
                        print(text)
                else:
                    print(text)
    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
    print(f"\n{lines} lineas guardadas ({dump_lines} de audio) en {out_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
