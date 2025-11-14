import tkinter as tk
from tkinter import messagebox, simpledialog, ttk

from ApiClient import APIClient
from xmlrpc.client import Fault

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
        self.root.geometry("1080x920")
        
        # --- 1. Frame de Login (Superior) ---
        self.frame_login = ttk.LabelFrame(root, text="Autenticación")
        self.frame_login.pack(fill="x", padx=10, pady=5)
        
        ttk.Label(self.frame_login, text="User:").pack(side=tk.LEFT, padx=5)
        self.entry_user = ttk.Entry(self.frame_login, width=15)
        self.entry_user.pack(side=tk.LEFT, padx=5)
        
        ttk.Label(self.frame_login, text="Pass:").pack(side=tk.LEFT, padx=5)
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
        self.frame_control = ttk.LabelFrame(root, text="Panel de Control")
        self.frame_control.pack(fill="both", expand=True, padx=10, pady=10)
        
        # (Aquí pondremos los botones de Homing, Move, etc.)
        # Por ahora, un simple saludo:
        ttk.Label(self.frame_control, text="Por favor, inicia sesión para habilitar los controles.").pack(pady=20)


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
        """Crea los botones de control una vez logueado."""
        # Frame interno para la grilla de botones
        botones_frame = ttk.Frame(self.frame_control)
        botones_frame.pack(expand=True, fill="both", padx=10, pady=10)

        # Lista de botones comunes (texto, función)
        botones = [
            ("Homing (G28)", self.homing),
            ("Status (M114)", self.status),
            ("Motors (M17/M18)", self.motors),
            ("Gripper (M3/M5)", self.gripper),
            ("Mode ", self.mode),
            ("Move (G0)", self.move),
            ("Upload ", self.upload),
            ("Run ", self.run),
            ("Inicializar grabado de trayectoria", self.rec_start),
            ("Finalizar grabado de trayectoria", self.rec_stop),
            ("Cambiar mi contraseña", self.changeMyPassword),
        ]

        # Si es admin, agrega los botones de admin
        if self.cliente.is_admin():
            botones += [
                ("Conectar robot", self.connect),
                ("Desconectar robot", self.disconnect),
                ("Registrar usuario", self.userRegister),
                ("Lista de usuarios", self.userList),
                ("Cambiar contraseña de usuario", self.changeUserPassword),
                ("Cambiar estado de usuario", self.updateUser),
            ]

        # Distribuir los botones en 2 columnas
        for i, (texto, funcion) in enumerate(botones):
            fila = i // 2
            columna = i % 2
            btn = ttk.Button(botones_frame, text=texto, command=funcion)
            btn.grid(row=fila, column=columna, padx=10, pady=8, sticky="ew")

        # Hacer que las columnas se expandan
        botones_frame.columnconfigure(0, weight=1)
        botones_frame.columnconfigure(1, weight=1)

    def homing(self):
        """Se ejecuta al presionar el botón 'Homing'."""
        
        # 1. Chequeo de permisos (la GUI lo hace)
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return

        try:
          
            r = self.cliente.robot_home()
            
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
            # Esta linea de abajo creo que la vamos a tener que sacar
            mensaje += f"Posicion: {status_info.get('position', 'N/A')}\n"
           
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
            enabled = messagebox.askyesno("Encender motores", "Quieres encender los motores del robot?")

            r = self.cliente.robot_set_motors(enabled)
            messagebox.showinfo("Motores", r.get('data', {}).get('msg', 'OK'))

        except Fault as e:
            # Alarma de Error
            messagebox.showerror("Error de Servidor", e.faultString)
            # (Aquí pondríamos el playsound('error.wav'))
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def gripper(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return 
        
        try:
            enabled = messagebox.askyesno("Gripper", "Quieres activar el gripper del robot?")

            r = self.cliente.robot_set_gripper(enabled)
            messagebox.showinfo("Gripper", r.get('data', {}).get('msg', 'OK'))
        
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
            x = simpledialog.askfloat("Coordenada X", "Ingrese la coordenada X:")
            if x is None:  
                return
            y = simpledialog.askfloat("Coordenada Y", "Ingrese la coordenada Y:")
            if y is None:
                return
            z = simpledialog.askfloat("Coordenada Z", "Ingrese la coordenada Z:")
            if z is None:
                return
            
            # Llamada al cerebro
            r = self.cliente.robot_move(x, y, z)
            messagebox.showinfo("Move Exitoso", r.get('data', {}).get('msg', 'OK'))
            
        except Fault as e:
            # Alarma de Error
            messagebox.showerror("Error de Servidor", e.faultString)
            # (Aquí pondríamos el playsound('error.wav'))
        except Exception as e:
            messagebox.showerror("Error", str(e))

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
                messagebox.showinfo("Upload exitoso", r.get('data', {}).get('msg', 'Archivo subido correctamente'))
            else:
                messagebox.showerror("Error al subir", r.get("error", "Error desconocido"))
            
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def run(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return
        
        try:
            filename = simpledialog.askstring("Ejecutar archivo", "Ingrese el nombre del archivo gcode a ejecutar:")
            if not filename:
                return

            r = self.cliente.robot_run_file(filename)
            messagebox.showinfo("Ejecución de archivo", r.get('data', 'N/A').get('msg', 'OK'))

        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def rec_start(self):
        if not self.cliente.has_operator_privileges():
            messagebox.showerror("Permiso Denegado", "Se requiere ser Operador o Admin.")
            return
        try:
            r = self.cliente.robot_rec_start()
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
            r = self.cliente.robot_rec_stop()
            messagebox.showinfo("Trayectoria del robot", r.get('data', 'N/A').get('msg', 'N/A'))

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
        # if not self.cliente.is_admin():
        #     messagebox.showerror("Error, no posees el privilegio para realizar esta operación")
        #     return 
        try:
            r = self.cliente.robot_connect()
            messagebox.showinfo("Conexión", r.get('data', 'N/A').get('msg', 'N/A'))
        
        except Fault as e:
            messagebox.showerror("Error de Servidor", e.faultString)
        except Exception as e:
            messagebox.showerror("Error", str(e))    

    def disconnect(self):
        # if not self.cliente.is_admin():
        #     messagebox.showerror("Error, no posees el privilegio para realizar esta operación")
        #     return
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
            privilege = simpledialog.askstring("Registro de usuario", "Privilegio (admin, operador, viewer): ) ")
            if privilege is None:
                return
            if privilege not in ["admin", "operador", "viewer"]:
                messagebox.showerror("Error", "Modo inválido. Use 'admin', 'operador', 'viewer'.")
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

# --- El Lanzador ---
if __name__ == "__main__":
    root = tk.Tk()
    app = RobotApp(root)
    root.mainloop()