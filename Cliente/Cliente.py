import sys
from Console_UI import ConsoleUI

def main():
    """Función principal del cliente"""

    url = "http://127.0.0.1:8080"
       
    try:
        # Crear y ejecutar la interfaz de consola
        ui = ConsoleUI(url)
        ui.run()
    except Exception as e:
        print(f"Error al iniciar el cliente: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()