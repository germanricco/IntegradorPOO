import tkinter as tk
from tkinter import messagebox, simpledialog, ttk

from ApiClient import APIClient
from xmlrpc.client import Fault
from RobotVisualizer import RobotVisualizer

class RobotApp:
    """
    Esta es la clase principal de nuestra aplicación gráfica (GUI).
    Maneja la "presentación" (los botones y ventanas).
    """
    def __init__(self, root):
        self.root = root
        self.root.title("Control Robot RRR (POO)")
        
        # Creamos una instancia de APIClient
        # (El cerebro es el que maneja el estado: token, user, etc.)
        self.cliente = APIClient("http://127.0.0.1:8080")

        # Configurar la ventana principal
        self.root.geometry("1200x920")
        
        # CREAR UN CONTENEDOR PRINCIPAL (PANEDWINDOW)
        # Esto nos permite dividir la pantalla en Izquierda (Control) y Derecha (Visual)
        self.paned = ttk.PanedWindow(root, orient=tk.HORIZONTAL)
        self.paned.pack(fill="both", expand=True, padx=10, pady=10)

        # --- IZQUIERDA: Frame LOGIN + CONTROL ---
        self.frame_left = ttk.Frame(self.paned)
        self.paned.add(self.frame_left, weight=1)

        # --- 1. Frame de Login (Superior) ---
        self.frame_login = ttk.LabelFrame(self.frame_left, text="Autenticación")
        self.frame_login.pack(fill="x", padx=10, pady=5)
        
        ttk.Label(self.frame_login, text="User:").pack(side=tk.LEFT, padx=5)
        self.entry_user = ttk.Entry(self.frame_login, width=15)
        self.entry_user.pack(side=tk.LEFT, padx=5)
        
        ttk.Label(self.frame_login, text="Password:").pack(side=tk.LEFT, padx=5)
        self.entry_pass = ttk.Entry(self.frame_login, width=15, show="*")
        self.entry_pass.pack(side=tk.LEFT, padx=5)

        self.btn_login = ttk.Button(self.frame_login, text="Login", command=self.hacer_login)
        self.btn_login.pack(side=tk.LEFT, padx=5)
        
        self.btn_logout = ttk.Button(self.frame_login, text="Logout", command=self.hacer_logout, state="disabled")
        self.btn_logout.pack(side=tk.LEFT, padx=5)

        # Etiqueta para mostrar el estado del login
        self.lbl_status = ttk.Label(self.frame_login, text="Estado: Desconectado")
        self.lbl_status.pack(side=tk.RIGHT, padx=10)

        # --- 2. Frame de Control (Principal) ---
        self.frame_control = ttk.LabelFrame(self.frame_left, text="Panel de Control")
        self.frame_control.pack(fill="both", expand=True, padx=10, pady=10)
        
        ttk.Label(self.frame_control, text="Por favor, inicia sesión para habilitar los controles.").pack(pady=20)

        # --- DERECHA: VISUALIZADOR ROBOT ---
        self.frame_right = ttk.Frame(self.paned)
        self.paned.add(self.frame_right, weight=2) # Weight 2 para que sea mas grande
        
        # Instanciamos nuestro visualizador
        self.visualizer = RobotVisualizer(self.frame_right, width=350, height=300)
        self.visualizer.pack(fill="both", expand=True)

    # --- Controladores de Eventos (Callbacks) ---

    def hacer_login(self):
        """Se ejecuta al presionar el botón 'Login'."""
        user = self.entry_user.get()
        password = self.entry_pass.get()
        
        if not user or not password:
            messagebox.showwarning("Error", "Usuario y contraseña no pueden estar vacíos.")
            return

        try:
        
            r = self.cliente.login(user, password)
            
            # Actualizamos la GUI
            self.lbl_status.config(text=f"Usuario: {r['user']} ({r['privilegio']})")
            self.btn_login.config(state="disabled")
            self.btn_logout.config(state="normal")
            
            # Borramos el frame de "inicia sesión"
            for widget in self.frame_control.winfo_children():
                widget.destroy()
            
            # Botones de control
            self.crear_widgets_control()
            
        except Fault as e:
            # Si la API reporta un error del servidor
            messagebox.showerror("Error de Login", e.faultString)
        except Exception as e:
            # Si la API reporta un error local (ej. sin conexión)
            messagebox.showerror("Error", str(e))

    def hacer_logout(self):

        try:
            self.cliente.logout()
        except Exception as e:
            # No importa si falla, forzamos el logout local
            print(f"Error en logout (ignorado): {e}")
        
        # Actualizamos la GUI al estado inicial
        self.lbl_status.config(text="Estado: Desconectado")
        self.btn_login.config(state="normal")
        self.btn_logout.config(state="disabled")
        self.entry_pass.delete(0, tk.END)

        # Borramos los widgets de control
        for widget in self.frame_control.winfo_children():
            widget.destroy()
        
        ttk.Label(self.frame_control, text="Por favor, inicia sesión para habilitar los controles.").pack(pady=20)

    def crear_widgets_control(self):
        """Crea los botones de control una vez logueado, agrupados por funcionalidad."""

        # --- Grupo: Conexión (solo admin) ---
        if self.cliente.is_admin():
            frame_conexion = ttk.LabelFrame(self.frame_control, text="Conexión robot")
            frame_conexion.pack(fill="x", padx=10, pady=5)
            ttk.Button(frame_conexion, text="Conectar robot", command=self.connect).pack(fill="x", padx=5, pady=3)
            ttk.Button(frame_conexion, text="Desconectar robot", command=self.disconnect).pack(fill="x", padx=5, pady=3)

        # --- Grupo: Cuenta personal ---
        frame_cuenta = ttk.LabelFrame(self.frame_control, text="Cuenta")
        frame_cuenta.pack(fill="x", padx=10, pady=5)
        ttk.Button(frame_cuenta, text="Cambiar mi contraseña", command=self.changeMyPassword).pack(fill="x", padx=5, pady=3)

        # --- Grupo: Comandos del robot (AHORA EN 2 COLUMNAS) ---
        frame_comandos = ttk.LabelFrame(self.frame_control, text="Comandos del robot")
        frame_comandos.pack(fill="x", padx=10, pady=5)
        
        # Configurar columnas para que se expandan equitativamente
        frame_comandos.columnconfigure(0, weight=1)
        frame_comandos.columnconfigure(1, weight=1)

        botones_comandos = [
            ("Homing (G28)", self.homing),
            ("Status (M114)", self.status),
            ("Motors (M17/M18)", self.motors),
            ("Gripper (M3/M5)", self.gripper),
            ("Mode (G90/G91)", self.mode),
            ("Move (G0)", self.move),
            ("Reporte de Comandos (RAM)", self.get_report) 
        ]

        for i, (texto, funcion) in enumerate(botones_comandos):
            btn = ttk.Button(frame_comandos, text=texto, command=funcion)
            # i // 2 nos da la fila (0, 0, 1, 1...)
            # i % 2 nos da la columna (0, 1, 0, 1...)
            btn.grid(row=i // 2, column=i % 2, padx=5, pady=3, sticky="ew")

        # --- Grupo: Manejo de trayectorias (AHORA EN 2 COLUMNAS) ---
        frame_trayectorias = tk.LabelFrame(self.frame_control, text="Gestión de trayectorias")
        frame_trayectorias.pack(fill="x", padx=10, pady=5)
        
        # Configurar columnas
        frame_trayectorias.columnconfigure(0, weight=1)
        frame_trayectorias.columnconfigure(1, weight=1)

        botones_trayectoria = [
            ("Subir Archivo (Upload)", self.upload),
            ("Listar y Ejecutar Archivo (Run)", self.show_file_selection_dialog),
            ("Iniciar Grabación (Rec Start)", self.rec_start),
            ("Detener Grabación (Rec Stop)", self.rec_stop),
        ]
        
        for i, (texto, funcion) in enumerate(botones_trayectoria):
            btn = ttk.Button(frame_trayectorias, text=texto, command=funcion)
            btn.grid(row=i // 2, column=i % 2, padx=5, pady=3, sticky="ew")

        # --- Grupo: Gestión de usuarios (solo admin) ---
        if self.cliente.is_admin():
            frame_usuarios = ttk.LabelFrame(self.frame_control, text="Gestión de usuarios y Reportes")
            frame_usuarios.pack(fill="x", padx=10, pady=5)
            botones_usuarios = [
                ("Registrar usuario", self.userRegister),
                ("Lista de usuarios", self.userList),
                ("Cambiar contraseña de usuario", self.changeUserPassword),
                ("Cambiar estado de usuario", self.updateUser),
                ("Ver Log de Auditoría (CSV)", self.get_audit_log_report),
            ]
            for texto, funcion in botones_usuarios:
                ttk.Button(frame_usuarios, text=texto, command=funcion).pack(fill="x", padx=5, pady=3)

    def homing(self):
        """Se ejecuta al presionar el botón 'Homing'."""
        
        # 1. Chequeo de permisos (la GUI lo hace)
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return

        try:
            r = self.cliente.robot_home()
            if r.get('success'):
                self.visualizer.set_position(150, 0, 150)
                self.visualizer.log(r.get('data', {}).get('msg', 'Activa los motores antes de hacer homing'))
            # Alarma Visual/Auditiva (¡Opcional!)
            messagebox.showinfo("Homing", r.get('data', {}).get('msg', 'Activa los motores antes de hacer homing'))
            # (Aquí pondríamos el playsound('success.wav'))
            
        except Fault as e:
            # Alarma de Error
            messagebox.showerror("Error de Servidor", e.faultString)
            # (Aquí pondríamos el playsound('error.wav'))
        except Exception as e:
            messagebox.showerror("Error", str(e))
    
    def status(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return 
        
        try:
            r = self.cliente.robot_get_status()

            # Status_info es un diccionario que se obtiene a partir de r
            # Donde r es la respuesta del servidor
            status_info = r.get('data', {})
            # Busca la clave 'status' en el diccionario, si existe muestra su valor, sino N/A
            mensaje = f"Estado: {status_info.get('status', 'N/A')}\n"           
            self.visualizer.log(mensaje)

            messagebox.showinfo("Robot Status", mensaje)
    
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))

# ==== Metodos para el operador y admin ====

    def motors(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return
        try:
            # Consultar el estado actual de los motores
            status = self.cliente.robot_get_status()
            status_str = status.get('data', {}).get('status', '')

            # Preguntar primero si quiere encender o apagar
            enabled = messagebox.askyesno("Motores", "¿Quieres encender los motores del robot? (Sí = Encender, No = Apagar)")

            if enabled and "MOTORS ENABLED" in status_str:
                messagebox.showerror("Motores ya encendidos", "Los motores ya están encendidos.")
                return
            if not enabled and "MOTORS ENABLED" not in status_str:
                messagebox.showerror("Motores ya apagados", "Los motores ya están apagados.")
                return

            r = self.cliente.robot_set_motors(enabled)
            messagebox.showinfo("Motores", r.get('data', {}).get('msg'))
            self.visualizer.log(r.get('data').get('msg'))

        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def gripper(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return 
        
        try:
            enabled = messagebox.askyesno("Gripper", "Quieres activar el gripper del robot?")
            r = self.cliente.robot_set_gripper(enabled)

            if r.get('success'):
                self.visualizer.set_gripper(enabled)

            messagebox.showinfo("Gripper", r.get('data').get('msg', 'OK'))
            self.visualizer.log(r.get('data', {}).get('msg'))
        except Fault as e:
            # Alarma de Error
            messagebox.showerror("Error de Servidor", e.faultString)
            # (Aquí pondríamos el playsound('error.wav'))
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def mode(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return 
        
        try:
            modo = simpledialog.askstring("Modo de movimiento", "Ingrese el modo: ABS (absoluto) o REL (relativo)")
            if modo is None:
                return
            modo = modo.strip().lower() # convierto a minusculas
            if modo not in ["abs", "rel"]:
                messagebox.showerror("Error", "Modo inválido. Use 'ABS' o 'REL'.")
                return
            
            r = self.cliente.robot_set_mode(modo)
            messagebox.showinfo("Modo cambiado", r.get('data', {}).get('msg', 'OK'))
            self.visualizer.log(r.get('data').get('msg'))
        except Fault as e:
            # Alarma de Error
            messagebox.showerror("Error de Servidor", e.faultString)
            # (Aquí pondríamos el playsound('error.wav'))
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def move(self):

        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return
        
        try:
    
            x = simpledialog.askfloat("Coordenada X", "Ingrese la coordenada X [mm]:")
            if x is None:  
                return
            y = simpledialog.askfloat("Coordenada Y", "Ingrese la coordenada Y [mm]:")
            if y is None:
                return
            z = simpledialog.askfloat("Coordenada Z", "Ingrese la coordenada Z [mm]:")
            if z is None:
                return
            v = simpledialog.askfloat("Velocidad", "Ingrese la velocidad del robot [mm/s]:\n Si desea utilizar la velicdad por defecto presione ~cancel~")
            if v is None:
                v = 50
            # Llamada al cerebro
            r = self.cliente.robot_move(x, y, z, v)
            
            if r.get('success'):
                self.visualizer.set_position(x, y, z)

            messagebox.showinfo("Move Exitoso", r.get('data', {}).get('msg', 'OK'))
            self.visualizer.log(r.get('data').get('msg'))
        except Fault as e:
            # Alarma de Error
            messagebox.showerror("Error de Servidor", e.faultString)
            # (Aquí pondríamos el playsound('error.wav'))
        except Exception as e:
            messagebox.showerror("Error", str(e))

    # --- MODIFICADO (get_report) ---
    def get_report(self):
        """
        Llama al servicio robot.getReport.
        Si es Admin, pregunta por filtros opcionales.
        """
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return

        try:
            filter_user = None
            filter_error = None

            # 1. Preguntar por filtros SOLO si es Admin
            if self.cliente.is_admin():
                filter_user = simpledialog.askstring("Filtro de Reporte (Admin)", 
                                                     "Filtrar por usuario (dejar en blanco para ver todos):",
                                                     parent=self.root)
                if filter_user is None: return 
                if filter_user == "": filter_user = None

                err_choice = messagebox.askyesnocancel("Filtro de Reporte (Admin)",
                                                       "¿Filtrar por estado de error?\n\n"
                                                       " - SÍ: Ver solo ERRORES.\n"
                                                       " - NO: Ver solo ÉXITOS.\n"
                                                       " - CANCELAR: Ver ambos.",
                                                       parent=self.root)
                filter_error = err_choice
            
            # 2. Llamar a la API con los filtros
            result = self.cliente.robot_get_report(filter_user, filter_error)

            if not result.get("success"):
                 raise Exception(result.get("error", "Error desconocido del cliente"))

            # 3. Procesar la respuesta
            r = result["data"]
            if not r.get('ok'):
                 raise Exception("El servidor devolvió un error")

            total_cmds = r.get('total_comandos', 0)
            total_errs = r.get('total_errores', 0)
            entries = r.get('entries', [])

            # 4. Formatear el mensaje para mostrar
            report_title = self.cliente.user
            if self.cliente.is_admin() and filter_user:
                report_title = filter_user
            elif self.cliente.is_admin() and not filter_user:
                report_title = "TODOS LOS USUARIOS (RAM)"

            report_msg = f"--- Reporte de Comandos para '{report_title}' ---\n\n"
            report_msg += f"Mostrando {len(entries)} de {total_cmds} comandos ({total_errs} errores)\n"
            report_msg += "----------------------------------------\n"

            if not entries:
                report_msg += "\n(No hay comandos que coincidan con el filtro)"
            else:
                for entry in entries[-15:]: # Mostrar últimos 15
                    status = "ERROR" if entry.get('error') else "OK"
                    details = entry.get('details', 'N/A')
                    service = entry.get('service', 'N/A')
                    time = entry.get('timestamp', 'N/A')
                    
                    if self.cliente.is_admin() and not filter_user:
                        user = entry.get('username', '???')
                        report_msg += f"\n[{time}] [{status}] @{user} -> {service}\n"
                    else:
                        report_msg += f"\n[{time}] [{status}] {service}\n"
                        
                    if details != "N/A":
                         report_msg += f"    > {details}\n"

            self.visualizer.log(f"OK: Reporte generado ({len(entries)} comandos).")
            self.show_report_dialog("Reporte de Comandos (RAM)", report_msg)

        except Fault as e:
            self.visualizer.log(f"ERROR: {e.faultString}")
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            self.visualizer.log(f"ERROR: {str(e)}")
            messagebox.showerror("Error", str(e))
            
    def show_report_dialog(self, title, content):
        """Muestra un diálogo Toplevel con un widget de Texto (para copiar y pegar)"""
        dialog = tk.Toplevel(self.root)
        dialog.title(title)
        dialog.geometry("600x400")
        dialog.transient(self.root)
        dialog.grab_set()

        text_widget = tk.Text(dialog, wrap="word", height=20, width=70)
        text_widget.pack(padx=10, pady=10, fill="both", expand=True)
        text_widget.insert("1.0", content)
        text_widget.config(state="disabled") # Hacerlo de solo lectura

        ttk.Button(dialog, text="Cerrar", command=dialog.destroy).pack(pady=5)
    
    # --- FIN DE FUNCIÓN MODIFICADA ---

    def upload(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return
        
        try:
            filename = simpledialog.askstring("Nombre de archivo", "Ingrese el nombre para guardar el archivo en el servidor:")
            if not filename:
                return
            file_path = simpledialog.askstring("Ruta local", "Ingrese la ruta local del archivo a subir:")
            if not file_path:
                return

            r = self.cliente.robot_upload_file(filename, file_path)
            if r.get("success"):
                # Obtenemos el 'filename' de la respuesta del servidor
                final_filename = r.get('data', {}).get('filename', filename)
                msg = f"{r.get('data', {}).get('msg', 'OK')}\n\nGuardado como:\n{final_filename}"
                messagebox.showinfo("Upload Exitoso", msg)
            else:
                messagebox.showerror("Error al subir", r.get("error", "Error desconocido"))
            
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def rec_start(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return
        try:
            # --- MODIFICADO: Pedir nombre ---
            nombre_logico = simpledialog.askstring("Iniciar Grabación", "Ingrese un nombre para la trayectoria:")
            if not nombre_logico:
                return
            
            r = self.cliente.robot_rec_start(nombre_logico)
            data = r.get('data', 'N/A')
            if isinstance(data, dict):
                msg = data.get('msg', str(data))
            else:
                msg = str(data)            
            
            messagebox.showinfo("Trayectoria del robot", msg)
        
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))            

    def rec_stop(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return
        try:
            # --- MODIFICADO: Pedir nombre ---
            nombre_logico = simpledialog.askstring("Detener Grabación", "Ingrese el nombre de la trayectoria a detener:")
            if not nombre_logico:
                return
                
            r = self.cliente.robot_rec_stop(nombre_logico)
            # --- MODIFICADO: Mostrar nombre final ---
            msg = r.get('data', {}).get('msg', 'Detenido.')
            filename = r.get('data', {}).get('filename', None)
            if filename:
                msg += f"\n\nGuardado como:\n{filename}"
            messagebox.showinfo("Trayectoria del robot", msg)


        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))    

    def changeMyPassword(self):
        try:
            user = self.cliente.user
            old = simpledialog.askstring("Cambiar contraseña", "Ingrese la contraseña actual: ")
            if old is None:
                return
            new = simpledialog.askstring("Cambiar contraseña", "Ingrese la nueva contraseña: ")
            if new is None:
                return
            r = self.cliente.change_my_password(user, old, new)
            messagebox.showinfo("Cambiar contraseña", r.get('data', {}))
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))         
# ==== Metodos solo para admin ====

    def connect(self):
        try:
            r = self.cliente.robot_connect()
            messagebox.showinfo("Conexión", r.get('data', 'N/A').get('msg', 'N/A'))
        
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))    

    def disconnect(self):
        try:
            r = self.cliente.robot_disconnect()
            messagebox.showinfo("Conexión", r.get('data', 'N/A').get('msg', 'N/A'))
        
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))    

    def userRegister(self):
        try:
            username = simpledialog.askstring("Registro de usuario ","Ingresar el nombre de usuario a almacenar: ")
            if username is None:
                return
            password = simpledialog.askstring("Regsitro de usuario", "Contraseña: ")
            if password is None:
                return
            privilege = simpledialog.askstring("Registro de usuario", "Privilegio (admin, op, viewer): ) ")
            if privilege is None:
                return
            if privilege not in ["admin", "op", "viewer"]:
                messagebox.showerror("Error", "Modo inválido. Use 'admin', 'op', 'viewer'.")
                return
            r = self.cliente.register_user(username, password, privilege)
            messagebox.showinfo("Registro", r.get('data', 'N/A').get('msg', 'N/A'))

        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))    

    def userList(self):
        try:
            r = self.cliente.list_users()
            lista = r.get('users', [])

            texto = "Usuarios registrados: \n"
            for u in lista:
                texto += f"- NOMBRE: {u.get('username', 'N/A')} - PRIV: {u.get('role', 'N/A')} \n"
            
            messagebox.showinfo("Lista de usuarios", texto)

        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))    

    def changeUserPassword(self):
        try:
            username = simpledialog.askstring("Cambiar contraseña ", "Ingrese el nombre de usuario: ")
            if not username:
                return
            old = simpledialog.askstring("Cambiar contraseña ", "Ingrese la contraseña actual: ")
            if not old:
                return

            new = simpledialog.askstring("Cambiar contraseña", "Ingrese la nueva contraseña: ")
            if not new:
                return
            r = self.cliente.change_user_password(username, old, new)
            messagebox.showinfo("Cambio de contraseña", r.get('data', {}).get('msg', 'OK'))

        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))    

    def updateUser(self):
        if not self.cliente.is_admin():
            messagebox.showerror("Permiso Denegado", "Solo el admin puede realizar esta acción.")
            return
        try:
            id_str = simpledialog.askstring("Actualizar estado de usuario", "Escriba el id del usuario: ")
            if id_str is None:
                return
            try:
                user_id = int(id_str)
            except ValueError:
                messagebox.showerror("Error", "El id debe ser un número.")
                return

            active = simpledialog.askstring("Actualizar estado de usuario", "Escriba el estado (true|false): ")
            if active is None:
                return
            if active not in ["true", "false"]:
                messagebox.showerror("Error", "Modo inválido. Use 'true' o 'false'.")
                return

            # Conversión a booleano
            active_bool = (active == "true")

            r = self.cliente.update_user(user_id, active_bool)
            messagebox.showinfo("Actualizar estado de usuario", r.get('data', {}).get('msg','OK'))
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))    

    def show_file_selection_dialog(self):
        """
        Llama a listMyFiles, muestra un diálogo Toplevel con un Listbox
        y permite al usuario seleccionar un archivo para ejecutar.
        """
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return

        try:
            # 1. Llamar a la API para obtener la lista de archivos
            r_list = self.cliente.robot_list_files()
            if not r_list.get("success"):
                raise Exception(r_list.get("error", "Error desconocido al listar archivos"))
            
            files = r_list.get("files", [])
            if not files:
                messagebox.showinfo("Sin Archivos", "No se encontraron archivos de trayectoria en el servidor.")
                return

            # 2. Crear una nueva ventana (Toplevel) para el diálogo
            dialog = tk.Toplevel(self.root)
            dialog.title("Seleccionar Archivo para Ejecutar")
            dialog.geometry("450x300")
            dialog.transient(self.root) # Mantenerla encima de la app principal
            dialog.grab_set() # Hacerla modal (bloquea la ventana principal)

            ttk.Label(dialog, text="Seleccione un archivo de la lista:").pack(pady=5)

            # 3. Crear el Listbox con Scrollbar
            listbox_frame = ttk.Frame(dialog)
            listbox_frame.pack(fill="both", expand=True, padx=10, pady=5)
            
            scrollbar = ttk.Scrollbar(listbox_frame, orient=tk.VERTICAL)
            listbox = tk.Listbox(listbox_frame, yscrollcommand=scrollbar.set, exportselection=False)
            scrollbar.config(command=listbox.yview)
            
            scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
            listbox.pack(side=tk.LEFT, fill="both", expand=True)

            # Poblar la lista de archivos
            for file in files:
                listbox.insert(tk.END, file)
            
            # 4. Función interna para el botón "Ejecutar"
            def on_execute():
                try:
                    selected_indices = listbox.curselection()
                    if not selected_indices:
                        messagebox.showwarning("Sin Selección", "Por favor, seleccione un archivo.", parent=dialog)
                        return
                    
                    filename = listbox.get(selected_indices[0])
                    
                    if not messagebox.askyesno("Confirmar Ejecución", f"¿Seguro que desea ejecutar:\n\n{filename}?", parent=dialog):
                        return
                    
                    # 5. Llamar a la API para ejecutar el archivo
                    r_run = self.cliente.robot_run_file(filename)
                    if r_run.get("success"):
                        msg = r_run.get('data', {}).get('msg', 'Ejecución iniciada.')
                        
                        # --- ¡AQUÍ ESTÁ EL ARREGLO! ---
                        # Actualizamos el visualizer a la posición de homing,
                        # ya que el servidor siempre hace homing (G28) antes de empezar.
                        self.visualizer.set_position(150, 0, 150)
                        
                        # Mostrar el éxito en la ventana principal
                        messagebox.showinfo("Éxito", msg, parent=self.root) 
                        dialog.destroy() # Cerrar el diálogo
                    else:
                        raise Exception(r_run.get("error", "Error desconocido al ejecutar"))

                except Fault as e:
                    messagebox.showerror("Error de Servidor", e.faultString, parent=dialog)
                except Exception as e:
                    messagebox.showerror("Error", str(e), parent=dialog)

            # 6. Botones del diálogo
            btn_frame = ttk.Frame(dialog)
            btn_frame.pack(fill="x", pady=10, padx=10)
            
            ttk.Button(btn_frame, text="Ejecutar", command=on_execute).pack(side=tk.LEFT, expand=True, padx=5)
            ttk.Button(btn_frame, text="Cancelar", command=dialog.destroy).pack(side=tk.RIGHT, expand=True, padx=5)

        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))

    # --- NUEVA FUNCIÓN ---
    def get_audit_log_report(self):
        """
        Llama al servicio admin.getLogReport y muestra los resultados.
        """
        if not self.cliente.is_admin():
            messagebox.showerror("Permiso Denegado", "Solo el admin puede realizar esta acción.")
            return

        try:
            filter_user = None
            filter_response = None

            # 1. Preguntar por filtros
            filter_user = simpledialog.askstring("Filtro de Log (Admin)", 
                                                 "Filtrar por usuario (dejar en blanco para ver todos):",
                                                 parent=self.root)
            if filter_user is None: return 
            if filter_user == "": filter_user = None

            filter_response = simpledialog.askstring("Filtro de Log (Admin)", 
                                                     "Filtrar si la respuesta 'contiene' este texto (ej. ERROR, OK, FORBIDDEN). Dejar en blanco para ver todo:",
                                                     parent=self.root)
            if filter_response is None: return
            if filter_response == "": filter_response = None
            
            # 2. Llamar a la API con los filtros
            result = self.cliente.admin_get_log_report(filter_user, filter_response)

            if not result.get("success"):
                 raise Exception(result.get("error", "Error desconocido del cliente"))

            # 3. Procesar la respuesta
            r = result["data"]
            if not r.get('ok'):
                 raise Exception("El servidor devolvió un error")

            entries = r.get('log_entries', [])

            # 4. Formatear el mensaje para mostrar
            report_title = "TODOS LOS USUARIOS (CSV)"
            if filter_user:
                report_title = f"USUARIO: {filter_user} (CSV)"

            report_msg = f"--- Reporte de Auditoría '{report_title}' ---\n\n"
            report_msg += f"Mostrando {len(entries)} entradas\n"
            report_msg += "----------------------------------------\n"

            if not entries:
                report_msg += "\n(No hay entradas que coincidan con el filtro)"
            else:
                for entry in entries:
                    time = entry.get('timestamp', 'N/A')
                    peticion = entry.get('peticion', 'N/A')
                    user = entry.get('usuario', '???')
                    nodo = entry.get('nodo', 'N/A')
                    resp = entry.get('respuesta', 'N/A')
                    
                    report_msg += f"\n[{time}] @{user} (Nodo: {nodo})\n"
                    report_msg += f"  PETICIÓN: {peticion}\n"
                    report_msg += f"  RESPUESTA: {resp}\n"

            self.visualizer.log(f"OK: Reporte CSV generado ({len(entries)} entradas).")
            self.show_report_dialog("Reporte de Auditoría (audit.csv)", report_msg)

        except Fault as e:
            self.visualizer.log(f"ERROR: {e.faultString}")
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            self.visualizer.log(f"ERROR: {str(e)}")
            messagebox.showerror("Error", str(e))

# --- El Lanzador ---
if __name__ == "__main__":
    root = tk.Tk()
    app = RobotApp(root)
    root.mainloop()