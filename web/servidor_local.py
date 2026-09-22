#!/usr/bin/env python3
"""Servidor local para probar la página web del kit Sensei.

Web Bluetooth solo funciona en un "contexto seguro": HTTPS o localhost.
Este script sirve esta carpeta por HTTP en localhost, que Chrome y Edge
aceptan sin necesitar certificado — no hace falta nada más para probar
el flujo completo (BLE -> WiFi -> WebSocket) sin la app Android.

Uso:
    python servidor_local.py [puerto]

Luego abre la URL que imprime en Chrome o Edge (no funciona en Safari
ni Firefox — ver docs/proyecto.md, sección "Sobre la página web").
"""
import http.server
import socketserver
import sys
import webbrowser
from pathlib import Path

PUERTO = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
CARPETA = Path(__file__).resolve().parent


class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(CARPETA), **kwargs)


def main():
    with socketserver.TCPServer(("localhost", PUERTO), Handler) as httpd:
        url = f"http://localhost:{PUERTO}"
        print("=========================================")
        print("  Sensei — servidor web local")
        print("=========================================")
        print(f"  Sirviendo: {CARPETA}")
        print(f"  Abrir en Chrome o Edge: {url}")
        print("  Ctrl+C para detener.")
        print("=========================================")
        try:
            webbrowser.open(url)
        except Exception:
            pass  # sin entorno gráfico: solo falta abrir la URL a mano
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServidor detenido.")


if __name__ == "__main__":
    main()
