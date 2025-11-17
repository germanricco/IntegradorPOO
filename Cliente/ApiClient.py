from xmlrpc.client import ServerProxy, Fault
import base64

class APIClient:
    """Cliente para la API RPC del sistema de robot"""
    
    def __init__(self, url):
        self.api = ServerProxy(url, allow_none=True)
        self.token = None
        self.user = None
        self.priv = None

    # ===== Gestión de Sesión =====
    def refresh_session(self):
        """Verifica constantemente que el token esté vigente"""
        if not self.token:
            self.user = self.priv = None
            return
        try:
            r = self.api.__getattr__("auth.me")({"token": self.token})
            self.user = r["user"]
            self.priv = r["privilegio"]
        except Exception:
            # token inválido o server reiniciado
            self.user = self.priv = self.token = None

    def is_admin(self):
        """Verifica si el usuario actual es admin"""
        self.refresh_session()
        return self.priv == "admin"

    def has_operator_privileges(self):
        """Verifica si el usuario tiene privilegios de operador o admin"""
        self.refresh_session()
        return self.priv in ["admin", "op"]

    # ===== Autenticación =====
    def login(self, username, password):
        """Inicia sesión en el sistema"""
        try:
            r = self.api.__getattr__("auth.login")({"user": username, "pass": password})
            self.token = r["token"]
            self.user = r["user"]
            self.priv = r["privilegio"]
            return {"success": True, "user": self.user, "privilegio": self.priv}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def logout(self):
        """Cierra la sesión actual"""
        if not self.token:
            return {"success": False, "error": "No hay sesión activa"}
        try:
            result = self.api.__getattr__("auth.logout")({"token": self.token})
            self.token = self.user = self.priv = None
            return {"success": True, "message": result}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def get_current_user(self):
        """Obtiene información del usuario actual"""
        self.refresh_session()
        if self.user:
            return {"success": True, "user": self.user, "privilegio": self.priv}
        return {"success": False, "error": "No hay sesión activa"}

    # ===== Gestión de Usuarios (Admin) =====
    def register_user(self, username, password, privilege):
        """Registra un nuevo usuario (requiere admin)"""
        try:
            r = self.api.__getattr__("user.register")({
                "token": self.token, "user": username, "pass": password, "privilegio": privilege
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def list_users(self):
        """Lista todos los usuarios (requiere admin)"""
        try:
            r = self.api.__getattr__("user.list")({"token": self.token})
            return {"success": True, "users": r["users"]}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def update_user(self, user_id, active):
        """Actualiza la habilitacion de un usuario (requiere admin)"""
        try:
            payload = {"token": self.token, "id": user_id, "active": active}
                      
            r = self.api.__getattr__("user.update")(payload)
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def change_user_password(self, username, old_password, new_password):
        """Cambia la contraseña de un usuario (modo admin)"""
        try:
            r = self.api.__getattr__("user.changePassword")({
                "token": self.token, "user": username, "old": old_password , "new": new_password
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def change_my_password(self, username, old_password, new_password):
        """Cambia la propia contraseña"""
        try:
            r = self.api.__getattr__("user.changePassword")({
                "token": self.token, "user": username, "old": old_password, "new": new_password
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    # ===== Control de Robot =====
    def robot_connect(self):
        """Conecta al robot (requiere admin)"""
        try:
            r = self.api.__getattr__("robot.connect")({"token": self.token})
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_disconnect(self):
        """Desconecta del robot (requiere admin)"""
        try:
            r = self.api.__getattr__("robot.disconnect")({"token": self.token})
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_rec_start(self, nombreLogico):
        """Inicia la grabación de movimientos"""
        try:
            r = self.api.__getattr__("robot.iniciarGrabacion")({
                "token": self.token, "nombre": nombreLogico
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_rec_stop(self, nombreLogico):
        """Detiene la grabación de movimientos"""
        try:
            r = self.api.__getattr__("robot.finalizarGrabacion")({
                "token": self.token, "nombre": nombreLogico
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_home(self):
        """Envía el robot a posición home (G28)"""
        try:
            r = self.api.__getattr__("robot.homing")({"token": self.token})
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_get_status(self):
        """Obtiene el estado actual del robot (M114)"""
        try:
            r = self.api.__getattr__("robot.getStatus")({"token": self.token})
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_set_motors(self, enabled):
        """Activa/desactiva motores (M17/M18)"""
        try:
            r = self.api.__getattr__("robot.setMotors")({
                "token": self.token, "estado": enabled
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_set_gripper(self, enabled):
        """Activa/desactiva gripper (M3/M5)"""
        try:
            r = self.api.__getattr__("robot.setGripper")({
                "token": self.token, "estado": enabled
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_set_mode(self, mode):
        """Cambia modo de coordenadas (G90/G91)"""
        try:
            r = self.api.__getattr__("robot.setMode")({
                "token": self.token, "mode": mode
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_move(self, x, y, z, velocidad):
        """Mueve el robot a coordenadas específicas (G1)"""
        try:
            r = self.api.__getattr__("robot.move")({
                "token": self.token, "x": x, "y": y, "z": z, "velocidad": velocidad
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_upload_file(self, filename, file_path):
        """Sube un archivo gcode al servidor (como texto plano)"""
        try:
            # 1. Abrir en modo texto ('r'), no binario ('rb')
            with open(file_path, "r", encoding="utf-8") as f:
                file_content = f.read() # Leer como un string simple
           
            # 2. Enviar el contenido (texto) en el parámetro 'contenido'
            r = self.api.__getattr__("robot.uploadFile")({
                "token": self.token,
                "nombre": filename,
                "contenido": file_content
            })
            return {"success": True, "data": r}
        except FileNotFoundError:
            return {"success": False, "error": f"Archivo no encontrado: {file_path}"}
        except Fault as e:
            return {"success": False, "error": e.faultString}
        
    def robot_run_file(self, filename):
        """Ejecuta un archivo gcode en el servidor"""
        try:
            r = self.api.__getattr__("robot.runFile")({
                "token": self.token, "nombre": filename
            })
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}

    def robot_list_files(self):
        """Obtiene la lista de archivos de trayectoria del usuario."""
        try:
            # Llama al nuevo método RPC que creamos en el servidor
            r = self.api.__getattr__("robot.listMyFiles")({
                "token": self.token
            })
            # El servidor C++ devuelve {ok: bool, files: array}
            return {"success": True, "files": r["files"]}
        except Fault as e:
            return {"success": False, "error": e.faultString}
    
    def robot_get_report(self):
        """Obtiene el historial de comandos del usuario."""
        try:
            # Llama al método RPC que implementamos en el servidor
            r = self.api.__getattr__("robot.getReport")({
                "token": self.token
            })
            # El servidor C++ devuelve {ok: bool, total_comandos: int, ...}
            # Lo envolvemos en el formato estándar de nuestro cliente
            return {"success": True, "data": r}
        except Fault as e:
            return {"success": False, "error": e.faultString}
        
    # ===== Utilidades =====
    def get_available_methods(self):
        """Obtiene lista de métodos RPC disponibles"""
        try:
            return {"success": True, "methods": self.api.system.listMethods()}
        except Exception as e:
            return {"success": False, "error": str(e)}

    def get_method_help(self, method_name):
        """Obtiene ayuda de un método específico"""
        try:
            return {"success": True, "help": self.api.system.methodHelp(method_name)}
        except Exception as e:
            return {"success": False, "error": str(e)}