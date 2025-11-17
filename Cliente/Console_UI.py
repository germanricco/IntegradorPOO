#!/usr/bin/env python3
from ApiClient import APIClient

class ConsoleUI:
    """Interfaz de consola para el cliente del sistema de robot"""   
    def __init__(self, api_url):
        self.client = APIClient(api_url)
        
    def print_help(self):       
        self.client.refresh_session()
        print("==================================================================================================================")
        print("                                             COMANDOS BÁSICOS:")
        print("==================================================================================================================")   
        print("METODOS: <methods>                                                       # listar métodos RPC del servidor")
        print("HELP: <help>                                                             # comando de ayuda (filtrada por rol)")
        print("HELP RPC: <help-rpc> <metodo>                                            # ver help() del método en el servidor")
        print("LOGIN: <login> <user> <pass>                                             # loggeo de usuario ")
        print("==================================================================================================================")  
        if self.client.priv == "admin":
            print("==================================================================================================================")
            print("                                CONEXION ADMINISTRADOR - COMANDOS DISPONIBLES:")
            print("==================================================================================================================")   
            print("REGISTRO DE USUARIO: <uadd> <user> <pass> <priv>                     # user.register  (priv=admin|op|viewer)")
            print("LISTAR USUARIOS: <ulist>                                             # user.list")
            print("ACTUALIZAR ESTADO DE USUARIO: <uupd> <id> [hab=false|true]           # user.update")
            print("CAMBIAR CONTRASEÑA DE USUARIO: <uchpass> <user> <oldpass> <newpass>  # user.changePassword (modo admin)")
            print("CAMBIAR MI CONTRASEÑA: <mychpass> <old> <new>                        # user.changePassword (autoservicio)")
            print("==================================================================================================================")             
            print("CONECTAR ROBOT: <connect>                                                       ")
            print("DESCONECTAR ROBOT: <disconnect>                                                 ")                  
            print("HOME: <home>                                                         # (G28)    ")
            print("ESTADO DEL ROBOT: <status>                                           # (M114)   ")
            print("MOTORES: <motors> <on|off>                                           # (M17/M18)")
            print("GRIPPER: <gripper> <on|off>                                          # (M3/M5)  ")
            print("MODO DE COORDENADAS: <mode> <abs|rel>                                # (G90/G91)")
            print("MOVER ROBOT: <move> <x> <y> <z> <vel>                                # (G1)     ")
            print("SUBIR ARCHIVO: <upload> <local_file>                                            ")
            print("EJECUTAR: <run> <remote_file>                                                   ")
            print("LISTAR ARCHIVOS: <list> (o <ls>)                                     # Lista sus archivos (admin ve todos)")
            print("INICIO GRABADO DE TRAYECTORIA: <rec-start> <file>                               ")
            print("FIN GRABADO DE TRAYECTORIA: <rec-stop> <file>                                   ")
            print("REPORTE (RAM): <report> [user=<user>] [error=true|false]             # Reporte en vivo (CommandHistory)")
            print("REPORTE (CSV): <logreport> [user=<user>] [response=<texto>]          # Reporte persistente (audit.csv)")
            print("======================================================================================================================")   

        elif self.client.priv == "op":      
            print("======================================================================================================================")
            print("                                     CONEXION OPERADOR - COMANDOS DISPONIBLES:")
            print("======================================================================================================================")    
            print("HOME: <home>                                                         # (G28)    ")
            print("ESTADO DEL ROBOT: <status>                                           # (M114)   ")
            print("MOTORES: <motors> <on|off>                                           # (M17/M18)")
            print("GRIPPER: <gripper> <on|off>                                          # (M3/M5)  ")
            print("MODO DE COORDENADAS: <mode> <abs|rel>                                # (G90/G91)")
            print("MOVER ROBOT: <move> <x> <y> <z> <vel>                                # (G1)     ")
            print("SUBIR ARCHIVO: <upload> <local_file>                                            ")
            print("EJECUTAR: run <remote_file>                                                     ")
            print("LISTAR ARCHIVOS: <list> (o <ls>)                                     # Lista sus archivos de trayectoria")
            print("INICIO GRABADO DE TRAYECTORIA: <rec-start> <file>                               ")
            print("FIN GRABADO DE TRAYECTORIA: <rec-stop> <file>                                   ")
            print("OBTENER REPORTE DE COMANDOS: <report>                                           ")
            print("======================================================================================================================")  
        print("CERRAR PROGRAMA: <quit>       # Cerrar el programa "  )
        print("==================================================================================================================")  

    def print_table(self, rows, headers):
        "Tabla para mostrar los usuarios cuando se ejecuta ulist"
        if not rows:
            print("(sin datos)")
            return
        widths = [len(h) for h in headers]
        for r in rows:
            for i, cell in enumerate(r):
                widths[i] = max(widths[i], len(str(cell)))
        line = " | ".join(h.ljust(widths[i]) for i, h in enumerate(headers))
        sep  = "-+-".join("-"*widths[i] for i,_ in enumerate(headers))
        print(line); print(sep)
        for r in rows:
            print(" | ".join(str(r[i]).ljust(widths[i]) for i in range(len(headers))))

    def handle_command(self, line: str) -> bool:
        """Procesa un comando de la consola. Retorna False si debe salir."""
        try:
            parts = line.strip().split()
            if not parts:
                return True
            cmd, *args = parts

            # ===== Comandos Generales =====
            if cmd == "help":
                self.print_help()

            elif cmd == "methods":
                result = self.client.get_available_methods()
                if result["success"]:
                    print(result["methods"])
                else:
                    print(f"Error: {result['error']}")

            elif cmd == "help-rpc":
                if not args:
                    print("uso: help-rpc <metodo>")
                else:
                    result = self.client.get_method_help(args[0])
                    if result["success"]:
                        print(result["help"])
                    else:
                        print(f"Error: {result['error']}")

            # ===== Autenticación =====
            elif cmd == "login":
                if len(args) != 2:
                    print("uso: login <user> <pass>")
                else:
                    result = self.client.login(args[0], args[1])
                    if result["success"]:
                        print(f"login OK — user={result['user']}, priv={result['privilegio']}")
                    else:
                        print(f"Error: {result['error']}")

            elif cmd == "me":
                result = self.client.get_current_user()
                if result["success"]:
                    print({"ok": True, "user": result["user"], "privilegio": result["privilegio"]})
                else:
                    print(result["error"])

            elif cmd == "logout":
                result = self.client.logout()
                if result["success"]:
                    print(result["message"])
                else:
                    print(result["error"])

            # ===== Gestión de Usuarios =====
            elif cmd == "uadd":
                if not self.client.is_admin():
                    print("No sos admin.")
                elif len(args) != 3:
                    print("uso: uadd <user> <pass> <priv>")
                else:
                    result = self.client.register_user(args[0], args[1], args[2])
                    if result["success"]:
                        print(result["data"])
                    else:
                        print(f"Error: {result['error']}")

            elif cmd == "ulist":
                if not self.client.is_admin():
                    print("No sos admin.")
                else:
                    result = self.client.list_users()
                    if result["success"]:
                        users = result["users"]
                        rows = [[u["id"], u["username"], u["role"], "sí" if u["active"] else "no"] for u in users]
                        self.print_table(rows, headers=["id", "usuario", "rol", "habilitado"])
                    else:
                        print(f"Error: {result['error']}")

            elif cmd == "uupd":
                if not self.client.is_admin():
                    print("No sos admin.")
                elif len(args) != 2 or not args[1].startswith("hab="):
                    print("uso: uupd <id> hab=true|false")
                else:
                    try:
                        user_id = int(args[0])
                    except ValueError:
                        print("El id debe ser un número.")
                        return True
                    hab_str = args[1].split("=", 1)[1].lower()
                    if hab_str not in ("true", "false"):
                        print("El valor de hab debe ser true o false.")
                        return True
                    active = (hab_str == "true")
                    result = self.client.update_user(user_id, active)
                    if result["success"]:
                        print(result["data"])
                    else:
                        print(f"Error: {result['error']}")

            elif cmd == "uchpass":
                if not self.client.is_admin():
                    print("No sos admin.")
                elif len(args) != 3:
                    print("uso: uchpass <user> <oldpass> <newpass>")
                else:
                    result = self.client.change_user_password(args[0], args[1], args[2])
                    if result["success"]:
                        print(result["data"])
                    else:
                        print(f"Error: {result['error']}")

            elif cmd == "mychpass":
                if len(args) != 2:
                    print("uso: mychpass <old> <new>")
                else:
                    username = self.client.user
                    old = args[0]
                    new = args[1]
                    result = self.client.change_my_password(username, old, new)
                    if result["success"]:
                        print(result["data"])
                    else:
                        print(f"Error: {result['error']}")

            # ===== Control de Robot =====
            elif cmd == "connect":
                if not self.client.is_admin():
                    print("Error: Permiso denegado (se requiere 'admin')")
                else:
                    print("Enviando orden de conexión al robot...")
                    result = self.client.robot_connect()
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")

            elif cmd == "disconnect":
                if not self.client.is_admin():
                    print("Error: Permiso denegado (se requiere 'admin')")
                else:
                    print("Enviando orden de desconexión del robot...")
                    result = self.client.robot_disconnect()
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")

            elif cmd == "home":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                else:
                    print("Enviando comando de Homing (G28)...")
                    result = self.client.robot_home()
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")

            elif cmd == "status":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                else:
                    print("Pidiendo estado del robot (M114)...")
                    result = self.client.robot_get_status()
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")

            elif cmd in ["motors", "gripper"]:
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1 or args[0] not in ["on", "off"]:
                    print(f"Uso: {cmd} <on|off>")
                else:
                    enabled = (args[0] == "on")
                    print(f"Enviando comando ({cmd} {args[0]})...")
                    
                    if cmd == "motors":
                        result = self.client.robot_set_motors(enabled)
                    else:
                        result = self.client.robot_set_gripper(enabled)
                    
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")

            elif cmd == "mode":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1 or args[0] not in ["abs", "rel"]:
                    print("Uso: mode <abs|rel>")
                else:
                    gcode = "G90" if args[0] == "abs" else "G91"
                    print(f"Cambiando a modo {args[0]} ({gcode})...")
                    result = self.client.robot_set_mode(args[0])
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")

            elif cmd == "move":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) < 3 or len(args) > 4:
                    print("Uso: move <x> <y> <z> [vel]")
                else:
                    try:
                        x, y, z = float(args[0]), float(args[1]), float(args[2])
                        velocidad = float(args[3]) if len(args) == 4 else 50.0
                        
                        print(f"Moviendo a (X:{x}, Y:{y}, Z:{z}) a Vel:{velocidad}...")
                        result = self.client.robot_move(x, y, z, velocidad)
                        if result["success"]:
                            print("Respuesta del servidor:", result["data"])
                        else:
                            print(f"Error del servidor: {result['error']}")
                    except ValueError:
                        print("Error: Las coordenadas y la velocidad deben ser números.")

            elif cmd == "upload":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1:
                    print("Uso: upload <ruta_al_archivo_local.gcode>")
                else:
                    local_path = args[0]
                    print(f"Subiendo {local_path} al servidor...")
                    result = self.client.robot_upload_file(local_path, local_path)
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error: {result['error']}")

            elif cmd == "run":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1:
                    print("Uso: run <nombre_archivo_remoto.gcode>")
                else:
                    remote_file = args[0]
                    print(f"Solicitando ejecución de {remote_file} en el servidor...")
                    result = self.client.robot_run_file(remote_file)
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")

            elif cmd == "list" or cmd == "ls":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 0:
                    print("Uso: list (o ls)")
                else:
                    print("Solicitando lista de archivos del servidor...")
                    result = self.client.robot_list_files() # Llama al nuevo método de la API
                    
                    if result["success"]:
                        files = result["files"]
                        if not files:
                            print("(No se encontraron archivos en el servidor)")
                        else:
                            print("Archivos disponibles:")
                            # Imprime cada archivo en una nueva línea
                            for f in files:
                                print(f"  - {f}")
                    else:
                        print(f"Error del servidor: {result['error']}")

            # ==== Grabado de trayectorias ====
            elif cmd == "rec-start":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1:
                    print("Uso: rec-start <nombre_logico>")
                else:
                    nombre_logico = args[0]
                    print(f"Iniciando grabación de movimientos con nombre '{nombre_logico}'...")
                    result = self.client.robot_rec_start(nombre_logico)
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")
                
            elif cmd == "rec-stop":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1:
                    print("Uso: rec-stop <nombre_logico>")
                else:
                    nombre_logico = args[0]
                    print(f"Finalizando grabación con nombre '{nombre_logico}'...")
                    result = self.client.robot_rec_stop(nombre_logico)
                    if result["success"]:
                        print("Respuesta del servidor:", result["data"])
                    else:
                        print(f"Error del servidor: {result['error']}")
            
            # --- MODIFICADO ---
            elif cmd == "report":
                if not self.client.has_operator_privileges():
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                    return True
                
                # --- LÓGICA DE PARSEO DE FILTROS ---
                filter_user = None
                filter_error = None
                
                if self.client.is_admin():
                    try:
                        for arg in args:
                            if arg.startswith("user="):
                                filter_user = arg.split("=", 1)[1]
                            elif arg.startswith("error="):
                                val = arg.split("=", 1)[1].lower()
                                if val == "true":
                                    filter_error = True
                                elif val == "false":
                                    filter_error = False
                                else:
                                    raise ValueError(f"Valor inválido para 'error': {val}")
                        
                        log_msg = "Solicitando reporte de comandos..."
                        if filter_user: log_msg += f" [Filtro Usuario: {filter_user}]"
                        if filter_error is not None: log_msg += f" [Filtro Error: {filter_error}]"
                        print(log_msg)

                    except Exception as e:
                        print(f"Error en argumentos de filtro: {e}")
                        print("Uso: report [user=<nombre>] [error=true|false]")
                        return True
                else:
                    if len(args) != 0:
                        print("Uso: report (los operadores no pueden usar filtros)")
                        return True
                    print("Solicitando reporte de comandos...")
                # --- FIN LÓGICA DE PARSEO ---

                try:
                    # 1. Llamar al método de ApiClient con los filtros
                    result = self.client.robot_get_report(filter_user, filter_error)

                    if not result["success"]:
                         raise Exception(result["error"])

                    # 2. Obtener los datos (el resto es igual que antes)
                    r = result["data"] 
                    total_cmds = r.get('total_comandos', 0)
                    total_errs = r.get('total_errores', 0)
                    entries = r.get('entries', [])

                    # 3. Imprimir el reporte
                    print("==========================================================")
                    user_report = filter_user if filter_user else self.client.user
                    print(f"--- Reporte de Comandos para '{user_report}' ---")
                    print(f"Mostrando {len(entries)} de {total_cmds} comandos ({total_errs} errores)")
                    print("----------------------------------------------------------")

                    if not entries:
                        print("(No hay comandos que coincidan con el filtro)")
                    else:
                        for entry in entries:
                            status = "ERROR" if entry.get('error') else "OK   "
                            details = entry.get('details', 'N/A')
                            service = entry.get('service', 'N/A')
                            time = entry.get('timestamp', 'N/A')
                            
                            if self.client.is_admin() and not filter_user:
                                user = entry.get('username', '???')
                                print(f"[{time}] [{status}] @{user} -> {service}")
                            else:
                                print(f"[{time}] [{status}] {service}")

                            if details != "N/A":
                                 print(f"    > {details}")
                    
                    print("==========================================================")
                                            
                except Exception as e:
                    print(f"Error: {e}")

            # --- NUEVO COMANDO ---
            elif cmd == "logreport":
                if not self.client.is_admin():
                    print("Error: Permiso denegado (solo admin)")
                    return True

                # --- LÓGICA DE PARSEO DE FILTROS ---
                filter_user = None
                filter_response = None
                
                try:
                    for arg in args:
                        if arg.startswith("user="):
                            filter_user = arg.split("=", 1)[1]
                        elif arg.startswith("response="):
                            filter_response = arg.split("=", 1)[1]
                    
                    log_msg = "Solicitando reporte de auditoría (CSV)..."
                    if filter_user: log_msg += f" [Filtro Usuario: {filter_user}]"
                    if filter_response: log_msg += f" [Filtro Respuesta: {filter_response}]"
                    print(log_msg)

                except Exception as e:
                    print(f"Error en argumentos de filtro: {e}")
                    print("Uso: logreport [user=<nombre>] [response=<texto>]")
                    return True

                try:
                    # 1. Llamar al nuevo método de ApiClient
                    result = self.client.admin_get_log_report(filter_user, filter_response)

                    if not result["success"]:
                         raise Exception(result["error"])

                    # 2. Obtener los datos
                    r = result["data"] 
                    entries = r.get('log_entries', [])

                    # 3. Imprimir el reporte CSV
                    print("==========================================================")
                    print(f"--- Reporte de Auditoría (audit.csv) ---")
                    print(f"Mostrando {len(entries)} entradas")
                    print("----------------------------------------------------------")

                    if not entries:
                        print("(No hay entradas que coincidan con el filtro)")
                    else:
                        # Imprimir cabecera
                        headers = ["timestamp", "peticion", "usuario", "nodo", "respuesta"]
                        # Pre-calcular anchos de columna
                        rows = []
                        for entry in entries:
                            rows.append([
                                entry.get('timestamp', 'N/A'),
                                entry.get('peticion', 'N/A'),
                                entry.get('usuario', 'N/A'),
                                entry.get('nodo', 'N/A'),
                                entry.get('respuesta', 'N/A')
                            ])
                        self.print_table(rows, headers)
                    
                    print("==========================================================")
                                            
                except Exception as e:
                    print(f"Error: {e}")

            elif cmd == "quit":
                return False

            else:
                print("Comando desconocido. Escribí 'help'.")

        except Exception as e:
            print("Error:", e)
        return True

    def run(self):
        """Ejecuta el bucle principal de la consola"""
        print("CLI RPC. Escribí 'help'. Ctrl+C para salir.")
        try:
            while True:
                if not self.handle_command(input("> ")):
                    break
        except KeyboardInterrupt:
            print("\nConexión interrumpida, cerrando el cliente ...")