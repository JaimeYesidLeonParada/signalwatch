#!/usr/bin/env python3
"""Convierte el volcado de audio del reloj (lineas "AD:...") en archivos WAV.

El firmware (main/audio_diag.c, con AUDIO_DIAG_DUMP en 1) imprime por el puerto serie el audio
que capta el microfono, en base64. Guarda la salida del monitor en un archivo de texto y:

    python tools/audiodump_to_wav.py monitor.log [carpeta_de_salida]

Crea un WAV mono de 16 bits por cada ventana completa (audiodump-<id>-<clase>.wav) y muestra su
pico, su nivel medio y cuantas muestras estan saturadas. Solo usa la libreria estandar.
"""
import base64
import math
import re
import struct
import sys
import wave
from pathlib import Path

CLIP_LEVEL = 32700

BEGIN = re.compile(r"AD:BEGIN (\d+) (\d+) (\d+) (\S+) ([\d.]+)")
END = re.compile(r"AD:END (\d+)")
DATA = re.compile(r"AD:([A-Za-z0-9+/=]+)\s*$")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    log = Path(sys.argv[1])
    out_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else log.parent
    out_dir.mkdir(parents=True, exist_ok=True)

    current = None   # (id, rate, samples, label, conf, bytearray)
    written = 0
    for line in log.read_text(errors="replace").splitlines():
        m = BEGIN.search(line)
        if m:
            current = (int(m[1]), int(m[2]), int(m[3]), m[4], float(m[5]), bytearray())
            continue
        if current is None:
            continue
        m = END.search(line)
        if m and int(m[1]) == current[0]:
            wid, rate, samples, label, conf, data = current
            current = None
            if len(data) != samples * 2:
                print(f"ventana {wid}: incompleta ({len(data)} de {samples * 2} bytes), se omite")
                continue
            pcm = struct.unpack(f"<{samples}h", bytes(data))
            peak = max(abs(v) for v in pcm)
            rms = int(math.sqrt(sum(v * v for v in pcm) / samples))
            clipped = sum(1 for v in pcm if abs(v) >= CLIP_LEVEL)
            path = out_dir / f"audiodump-{wid}-{label}.wav"
            with wave.open(str(path), "wb") as w:
                w.setnchannels(1)
                w.setsampwidth(2)
                w.setframerate(rate)
                w.writeframes(bytes(data))
            print(f"{path.name}: modelo dijo {label} {conf:.2f} | pico={peak} rms={rms} saturadas={clipped}")
            written += 1
            continue
        m = DATA.search(line)
        if m:
            try:
                current[5].extend(base64.b64decode(m[1]))
            except ValueError:
                pass   # linea cortada o mezclada con otro texto: la ventana saldra incompleta
    print(f"{written} ventana(s) guardada(s) en {out_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
