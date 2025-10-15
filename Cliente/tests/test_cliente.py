import xmlrpc.client

def main():
    host = "localhost"
    port = 8080  # Ajustar
    server_url = f"http://{host}:{port}"

    print(f"Conectando a {server_url}...")

    try:
        # Crear proxy para el servidor
        with xmlrpc.client.ServerProxy(server_url) as proxy:
            # Llamar al método de prueba
            respuesta = proxy.ServicioPrueba()
            
            print(f"✅ Respuesta del servidor: {respuesta}")
            
    except xmlrpc.client.Fault as error:
        print(f"❌ Error en la llamada remota:")
        print(f"   Código: {error.faultCode}")
        print(f"   Mensaje: {error.faultString}")
    
    except xmlrpc.client.ProtocolError as error:
        print(f"❌ Error de protocolo:")
        print(f"   URL: {error.url}")
        print(f"   Código HTTP: {error.errcode}")
        print(f"   Mensaje: {error.errmsg}")
    
    except ConnectionRefusedError:
        print(f"❌ No se pudo conectar al servidor en {server_url}")
        print("   Verifica que:")
        print("   1. El servidor esté ejecutándose")
        print("   2. El puerto sea correcto")
        print("   3. No haya problemas de firewall")
    
    except Exception as e:
        print(f"❌ Error inesperado: {e}")

if __name__ == "__main__":
    main()