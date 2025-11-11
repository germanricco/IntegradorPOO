lfrom xmlrpc.server import SimpleXMLRPCServer
import base64 # Necesario para el mock de upload

# --- Nuestro Mock Server COMPLETO ---
# Finge ser el "cerebro" C++ y responde a todos
# los métodos que usa cli.py.

# --- Mocks auth.* ---
def mock_auth_login(params):
    user = params["user"]
    print(f"[Mock Server] Recibido 'auth.login' para: {user}")
    return {
        "ok": True,
        "token": "TOKEN_FALSO_12345", 
        "user": user,
        "privilegio": "admin"
    }

def mock_auth_me(params):
    token = params["token"]
    print(f"[Mock Server] Recibido 'auth.me' con token: {token}")
    if token != "TOKEN_FALSO_12345": raise Exception("AUTH_INVALID: token")
    return {"ok": True, "user": "jorge", "privilegio": "admin"}

def mock_auth_logout(params):
    token = params["token"]
    print(f"[Mock Server] Recibido 'auth.logout' con token: {token}")
    if token != "TOKEN_FALSO_12345": raise Exception("AUTH_INVALID: token")
    return {"ok": True}

# --- Mocks user.* ---
def mock_user_register(params):
    print(f"[Mock Server] Recibido 'user.register' para: {params['user']}")
    return {"ok": True, "id": 123}

def mock_user_list(params):
    print(f"[Mock Server] Recibido 'user.list'")
    # Devolvemos una lista de usuarios falsos
    return {
        "ok": True,
        "users": [
            {"id": 1, "user": "jorge", "privilegio": "admin", "habilitado": True},
            {"id": 2, "user": "pepe", "privilegio": "op", "habilitado": True},
            {"id": 3, "user": "ana", "privilegio": "viewer", "habilitado": False},
        ]
    }

def mock_user_update(params):
    print(f"[Mock Server] Recibido 'user.update' para: {params['user']}")
    return {"ok": True}

def mock_user_change_password(params):
    print(f"[Mock Server] Recibido 'user.changePassword'")
    return {"ok": True}

# --- Mocks robot.* (Nuestros nuevos métodos) ---

# 1. Gestión de Conexión y Estado
def mock_robot_connect(params):
    print(f"[Mock Server] Recibido 'robot.connect'")
    return {"ok": True, "msg": "Arduino conectado (Mock)"}

def mock_robot_disconnect(params):
    print(f"[Mock Server] Recibido 'robot.disconnect'")
    return {"ok": True, "msg": "Arduino desconectado (Mock)"}

def mock_robot_get_status(params):
    print(f"[Mock Server] Recibido 'robot.getStatus'")
    return {"ok": True, "status": "M114: X:10.0 Y:50.0 Z:80.0 (Mock)"}

# 2. Movimiento Manual
def mock_robot_set_motors(params):
    estado = "ON" if params["estado"] else "OFF"
    print(f"[Mock Server] Recibido 'robot.setMotors' -> {estado}")
    return {"ok": True, "msg": f"Motores {estado} (Mock)"}

def mock_robot_homing(params):
    print(f"[Mock Server] Recibido 'robot.homing'")
    return {"ok": True, "msg": "Homing completado (Mock)"}

def mock_robot_set_gripper(params):
    estado = "ON" if params["estado"] else "OFF"
    print(f"[Mock Server] Recibido 'robot.setGripper' -> {estado}")
    return {"ok": True, "msg": f"Gripper {estado} (Mock)"}

def mock_robot_set_mode(params):
    print(f"[Mock Server] Recibido 'robot.setMode' -> {params['mode']}")
    return {"ok": True, "msg": f"Modo seteado a {params['mode']} (Mock)"}

def mock_robot_move(params):
    print(f"[Mock Server] Recibido 'robot.move' -> X:{params['x']}, Y:{params['y']}, Z:{params['z']}, F:{params['vel']}")
    return {"ok": True, "msg": "Movimiento completado (Mock)"}

# 3. Modo Automático
def mock_robot_upload_file(params):
    nombre = params['nombre']
    data = base64.b64decode(params['data_b64']).decode('utf-8')
    print(f"[Mock Server] Recibido 'robot.uploadFile' -> nombre:{nombre}")
    print(f"--- Contenido del archivo (primeras 50 chars) ---")
    print(data[:50] + "...")
    print(f"------------------------------------------------")
    return {"ok": True, "msg": f"Archivo '{nombre}' subido (Mock)"}

def mock_robot_run_file(params):
    print(f"[Mock Server] Recibido 'robot.runFile' -> {params['nombre']}")
    return {"ok": True, "msg": f"Ejecutando '{params['nombre']}' (Mock)"}


# --- Configuración del Servidor ---
if __name__ == "__main__":
    puerto = 8080
    servidor = SimpleXMLRPCServer(("localhost", puerto))
    servidor.register_introspection_functions()

    print(f"Mock Server (Python) COMPLETO escuchando en el puerto {puerto}...")
    
    # Registramos TODOS los métodos
    
    # auth.*
    servidor.register_function(mock_auth_login, "auth.login")
    servidor.register_function(mock_auth_me, "auth.me")
    servidor.register_function(mock_auth_logout, "auth.logout")
    
    # user.*
    servidor.register_function(mock_user_register, "user.register")
    servidor.register_function(mock_user_list, "user.list")
    servidor.register_function(mock_user_update, "user.update")
    servidor.register_function(mock_user_change_password, "user.changePassword")
    
    # robot.*
    servidor.register_function(mock_robot_connect, "robot.connect")
    servidor.register_function(mock_robot_disconnect, "robot.disconnect")
    servidor.register_function(mock_robot_get_status, "robot.getStatus")
    servidor.register_function(mock_robot_set_motors, "robot.setMotors")
    servidor.register_function(mock_robot_homing, "robot.homing")
    servidor.register_function(mock_robot_set_gripper, "robot.setGripper")
    servidor.register_function(mock_robot_set_mode, "robot.setMode")
    servidor.register_function(mock_robot_move, "robot.move")
    servidor.register_function(mock_robot_upload_file, "robot.uploadFile")
    servidor.register_function(mock_robot_run_file, "robot.runFile")
    
    try:
        servidor.serve_forever()
    except KeyboardInterrupt:
        print("\nCerrando Mock Server...")