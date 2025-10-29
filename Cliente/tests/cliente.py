#!/usr/bin/env python3
from xmlrpc.client import ServerProxy, Fault
import base64

class CLI:
    def __init__(self, url):
        self.api = ServerProxy(url, allow_none=True)
        self.token = None
        self.user = None
        self.priv = None

    # ===== helpers =====
    def _refresh_me(self):
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

    def _is_admin(self):
        self._refresh_me()
        return self.priv == "admin"

    def _print_table(self, rows, headers):
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

    def _help(self):
        self._refresh_me()
        print("Comandos disponibles:")
        print("  methods                      # listar métodos RPC publicados por el servidor")
        print("  help                         # esta ayuda (filtrada por rol)")
        print("  help-rpc <metodo>            # ver help() del método en el servidor")
        print("  login <user> <pass>")
        print("  me")
        print("  logout")
        if self._is_admin():
            print("  uadd <user> <pass> <priv>    # user.register  (priv=admin|op|viewer)")
            print("  ulist                        # user.list")
            print("  uupd <user> [name=NEW] [priv=admin|op|viewer] [hab=0|1]")
            print("                               # user.update")
            print("  uchpass <user> <newpass>     # user.changePassword (modo admin)")
        print("  mychpass <old> <new>         # user.changePassword (autoservicio)")
        # --- Comandos de Robot ---
        if self.priv == "admin":
            print("  connect                      # [ROBOT] Conecta el servidor al robot")
            print("  disconnect                   # [ROBOT] Desconecta el servidor del robot")
        
        if self.priv == "admin" or self.priv == "op":
            print("  home                         # [ROBOT] Mover a posicion Origen (G28)")
            print("  status                       # [ROBOT] Pide estado actual (M114)")

            print("  motors <on|off>              # [ROBOT] Activa/Desactiva motores (M17/M18)")
            print("  gripper <on|off>             # [ROBOT] Activa/Desactiva efector (M3/M5)")
            print("  mode <abs|rel>               # [ROBOT] Setea modo coordenadas (G90/G91)")
            print("  move <x> <y> <z> [vel]       # [ROBOT] Mueve a (X,Y,Z) [vel] (G1)")

            print("  upload <local_file>        # [ROBOT] Sube un .gcode al servidor")
            print("  run <remote_file>          # [ROBOT] Ejecuta un .gcode en el servidor")

        print("  quit")

    # ===== dispatcher =====
    def do(self, line: str) -> bool:
        try:
            parts = line.strip().split()
            if not parts:
                return True
            cmd, *args = parts

            if cmd == "help":
                self._help()

            elif cmd == "methods":
                print(self.api.system.listMethods())

            elif cmd == "help-rpc":
                if not args:
                    print("uso: help-rpc <metodo>")
                else:
                    print(self.api.system.methodHelp(args[0]))

            elif cmd == "login":
                if len(args) != 2:
                    print("uso: login <user> <pass>")
                else:
                    u, p = args
                    r = self.api.__getattr__("auth.login")({"user": u, "pass": p})
                    self.token = r["token"]; self.user = r["user"]; self.priv = r["privilegio"]
                    print(f"login OK — user={self.user}, priv={self.priv}")

            elif cmd == "me":
                self._refresh_me()
                if self.user:
                    print({"ok": True, "user": self.user, "privilegio": self.priv})
                else:
                    print("No hay sesión activa.")

            elif cmd == "logout":
                if not self.token:
                    print("No hay sesión activa.")
                else:
                    print(self.api.__getattr__("auth.logout")({"token": self.token}))
                    self.token = self.user = self.priv = None

            elif cmd == "uadd":
                if not self._is_admin():
                    print("No sos admin.")
                elif len(args) != 3:
                    print("uso: uadd <user> <pass> <priv>")
                else:
                    u, p, prv = args
                    r = self.api.__getattr__("user.register")({
                        "token": self.token, "user": u, "pass": p, "privilegio": prv
                    })
                    print(r)

            elif cmd == "ulist":
                if not self._is_admin():
                    print("No sos admin.")
                else:
                    r = self.api.__getattr__("user.list")({"token": self.token})
                    users = r["users"]
                    rows = [[u["id"], u["user"], u["privilegio"], "sí" if u["habilitado"] else "no"] for u in users]
                    self._print_table(rows, headers=["id", "usuario", "rol", "habilitado"])

            elif cmd == "uupd":
                if not self._is_admin():
                    print("No sos admin.")
                elif not args:
                    print("uso: uupd <user> [name=NEW] [priv=admin|op|viewer] [hab=0|1]")
                else:
                    payload = {"token": self.token, "user": args[0]}
                    for kv in args[1:]:
                        if "=" not in kv: 
                            print("parámetro inválido:", kv); return True
                        k, v = kv.split("=", 1)
                        if k == "name": payload["newUser"] = v
                        elif k == "priv": payload["newPrivilegio"] = v
                        elif k == "hab":  payload["habilitado"] = (v == "1")
                        else:
                            print("parámetro desconocido:", k); return True
                    print(self.api.__getattr__("user.update")(payload))

            elif cmd == "uchpass":
                if not self._is_admin():
                    print("No sos admin.")
                elif len(args) != 2:
                    print("uso: uchpass <user> <newpass>")
                else:
                    u, newp = args
                    print(self.api.__getattr__("user.changePassword")({
                        "token": self.token, "user": u, "newPass": newp
                    }))

            elif cmd == "mychpass":
                if len(args) != 2:
                    print("uso: mychpass <old> <new>")
                else:
                    old, newp = args
                    print(self.api.__getattr__("user.changePassword")({
                        "token": self.token, "oldPass": old, "newPass": newp
                    }))

            # ==================================
            # ===== CASOS DE USO DE ROBOT =====
            # ==================================
            
            elif cmd == "home":
                self._refresh_me() # 1. Refrescar la sesión (el patrón de siempre)

                # 2. Verificar Permisos (nuestro "Cliente Inteligente")
                if self.priv == "viewer" or not self.priv:
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                else:
                    try:
                        # 3. Llamar al Servicio (el "Patrón Mágico")
                        print("Enviando comando de Homing (G28)...")
                        r = self.api.__getattr__("robot.homing")({
                            "token": self.token
                        })
                        print("Respuesta del servidor:", r)
                    
                    # 4. Manejo de Errores (opcional pero recomendado)
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "connect":
                self._refresh_me()
                
                # 1. Verificar Permisos (Admin)
                if not self._is_admin():
                    print("Error: Permiso denegado (se requiere 'admin')")
                else:
                    try:
                        # 2. Llamar al Servicio
                        print("Enviando orden de conexión al robot...")
                        r = self.api.__getattr__("robot.connect")({
                            "token": self.token
                        })
                        print("Respuesta del servidor:", r)
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "disconnect":
                self._refresh_me()
                
                # 1. Verificar Permisos (Admin)
                if not self._is_admin():
                    print("Error: Permiso denegado (se requiere 'admin')")
                else:
                    try:
                        # 2. Llamar al Servicio
                        print("Enviando orden de desconexión del robot...")
                        r = self.api.__getattr__("robot.disconnect")({
                            "token": self.token
                        })
                        print("Respuesta del servidor:", r)
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "status":
                self._refresh_me()
                
                # 1. Verificar Permisos (Op o Admin)
                if self.priv == "viewer" or not self.priv:
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                else:
                    try:
                        # 2. Llamar al Servicio
                        print("Pidiendo estado del robot (M114)...")
                        r = self.api.__getattr__("robot.getStatus")({
                            "token": self.token
                        })
                        print("Respuesta del servidor:", r)
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "motors" or cmd == "gripper":
                self._refresh_me()
                if self.priv == "viewer" or not self.priv:
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1 or (args[0] != "on" and args[0] != "off"):
                    print(f"Uso: {cmd} <on|off>")
                else:
                    try:
                        # 1. Preparar parámetros
                        servicio_rpc = "robot.setMotors" if cmd == "motors" else "robot.setGripper"
                        estado_bool = (args[0] == "on")
                        comando_gcode = f"({cmd} {args[0]})"
                        
                        # 2. Llamar al Servicio
                        print(f"Enviando comando {comando_gcode}...")
                        r = self.api.__getattr__(servicio_rpc)({
                            "token": self.token,
                            "estado": estado_bool  # Enviamos un booleano
                        })
                        print("Respuesta del servidor:", r)
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "mode":
                self._refresh_me()
                if self.priv == "viewer" or not self.priv:
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1 or (args[0] != "abs" and args[0] != "rel"):
                    print("Uso: mode <abs|rel>")
                else:
                    try:
                        # 1. Preparar parámetros
                        modo_str = args[0]
                        comando_gcode = "G90" if modo_str == "abs" else "G91"
                        
                        # 2. Llamar al Servicio
                        print(f"Cambiando a modo {modo_str} ({comando_gcode})...")
                        r = self.api.__getattr__("robot.setMode")({
                            "token": self.token,
                            "mode": modo_str # Enviamos el string "abs" o "rel"
                        })
                        print("Respuesta del servidor:", r)
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "move":
                self._refresh_me()
                if self.priv == "viewer" or not self.priv:
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) < 3 or len(args) > 4:
                    print("Uso: move <x> <y> <z> [vel]")
                else:
                    try:
                        # 1. Preparar parámetros
                        # Usamos float() para convertir los strings a números
                        params_rpc = {
                            "token": self.token,
                            "x": float(args[0]),
                            "y": float(args[1]),
                            "z": float(args[2]),
                            "vel": float(args[3]) if len(args) == 4 else 150.0 # Velocidad default
                        }
                        
                        # 2. Llamar al Servicio
                        print(f"Moviendo a (X:{params_rpc['x']}, Y:{params_rpc['y']}, Z:{params_rpc['z']}) a Vel:{params_rpc['vel']}...")
                        r = self.api.__getattr__("robot.move")(params_rpc)
                        print("Respuesta del servidor:", r)
                    except ValueError:
                        print("Error: Las coordenadas y la velocidad deben ser números.")
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "upload":
                self._refresh_me()
                if self.priv == "viewer" or not self.priv:
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1:
                    print("Uso: upload <ruta_al_archivo_local.gcode>")
                else:
                    try:
                        # 1. Leer el archivo local
                        local_path = args[0]
                        with open(local_path, "rb") as f: # Abrir en modo 'read binary'
                            file_content = f.read()
                        
                        # 2. Codificar en Base64
                        content_b64 = base64.b64encode(file_content).decode('utf-8')
                        
                        # 3. Llamar al Servicio
                        print(f"Subiendo {local_path} al servidor...")
                        r = self.api.__getattr__("robot.uploadFile")({
                            "token": self.token,
                            "nombre": local_path,  # O un nombre de archivo limpio
                            "data_b64": content_b64
                        })
                        print("Respuesta del servidor:", r)
                    except FileNotFoundError:
                        print(f"Error: No se encontró el archivo local '{local_path}'")
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "run":
                self._refresh_me()
                if self.priv == "viewer" or not self.priv:
                    print("Error: Permiso denegado (se requiere 'op' o 'admin')")
                elif len(args) != 1:
                    print("Uso: run <nombre_archivo_remoto.gcode>")
                else:
                    try:
                        # 1. Preparar parámetros
                        remote_file = args[0]
                        
                        # 2. Llamar al Servicio
                        print(f"Solicitando ejecución de {remote_file} en el servidor...")
                        r = self.api.__getattr__("robot.runFile")({
                            "token": self.token,
                            "nombre": remote_file
                        })
                        print("Respuesta del servidor:", r)
                    except Fault as e:
                        print(f"Error del servidor: {e.faultString}")
                    except Exception as e:
                        print(f"Error inesperado: {e}")

            elif cmd == "quit":
                return False

            else:
                print("Comando desconocido. Escribí 'help'.")

        except Fault as e:
            # Nunca cortamos el CLI: mostramos el fault y seguimos
            print("Fault:", e)
        except Exception as e:
            print("Error:", e)
        return True


if __name__ == "__main__":
    cli = CLI("http://127.0.0.1:8080")
    print("CLI RPC. Escribí 'help'. Ctrl+C para salir.")
    while True:
        if not cli.do(input("> ")):
            break

