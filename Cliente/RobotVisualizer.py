import tkinter as tk
import math
from tkinter import scrolledtext
import datetime

class RobotVisualizer(tk.Frame):
    # --- PALETA DE COLORES (Estilo Industrial) ---
    COLOR_BG = "#2E2E2E"       
    COLOR_GRID = "#404040"     
    COLOR_BASE = "#505050"     
    COLOR_LINK = "#D4D4D4"     
    COLOR_JOINT = "#FF8C00"    
    COLOR_GRIPPER = "#32CD32"  
    COLOR_GRIPPER_OFF = "#FF0000" 
    COLOR_TEXT = "#FFFFFF"    

    # Colores para la consola
    COLOR_CONSOLE_BG = "#1E1E1E"
    COLOR_CONSOLE_FG = "#FFB700"

    def __init__(self, parent, width=300, height=300):
        super().__init__(parent, bd=2, relief="groove", bg=self.COLOR_BG)
        self.width = width
        self.height = height
        
        # --- Geometría (mm) ---
        self.L1 = 140  
        self.L2 = 160  
        self.L3 = 160  
        
        # Escala y Origen
        self.scale = 0.4
        self.origin_x = width // 2
        self.origin_y = height - 50 

        # Estado
        self.curr_x = 150
        self.curr_y = 0
        self.curr_z = 150
        self.gripper_open = False

        # --- Vistas ---
        # Frame contenedor para los gráficos (para separarlos de la consola)
        # --- 1. Vistas Gráficas (Top & Side) ---
        # (Mantenemos la estructura anterior pero usando pack con side=TOP)
        
        # Frame contenedor para los gráficos (para separarlos de la consola)
        self.frame_graphics = tk.Frame(self, bg=self.COLOR_BG)
        self.frame_graphics.pack(side="top", fill="both", expand=True)

        # Vista Superior
        tk.Label(self.frame_graphics, text="Vista Superior (XY)", 
                 bg=self.COLOR_BG, fg=self.COLOR_TEXT, font=("Arial", 8, "bold")).pack(pady=(5,0))
        self.canvas_top = tk.Canvas(self.frame_graphics, width=width, height=width, 
                                    bg=self.COLOR_BG, highlightthickness=0)
        self.canvas_top.pack(pady=2)

        # Vista Lateral
        tk.Label(self.frame_graphics, text="Vista Lateral (Z)", 
                 bg=self.COLOR_BG, fg=self.COLOR_TEXT, font=("Arial", 8, "bold")).pack(pady=(5,0))
        self.canvas_side = tk.Canvas(self.frame_graphics, width=width, height=height, 
                                     bg=self.COLOR_BG, highlightthickness=0)
        self.canvas_side.pack(pady=2)

        # --- 2. Mini Consola (Log) ---
        tk.Label(self, text="Historial de Comandos", 
                 bg=self.COLOR_BG, fg=self.COLOR_TEXT, font=("Arial", 8)).pack(side="top", anchor="w", padx=5)
        
        self.console = scrolledtext.ScrolledText(self, height=8, bg=self.COLOR_CONSOLE_BG, 
                                                 fg=self.COLOR_CONSOLE_FG, font=("Consolas", 8),
                                                 state='disabled') # Disabled para que el usuario no escriba
        self.console.pack(side="bottom", fill="x", padx=5, pady=5)

        # Mensaje inicial
        self.log("Sistema de visualización iniciado.")
        self.draw_robot()
    
    def log(self, message):
        """Agrega un mensaje a la mini consola con timestamp"""
        timestamp = datetime.datetime.now().strftime("%H:%M:%S")
        full_msg = f"[{timestamp}] {message}\n"
        
        self.console.config(state='normal') # Habilitar escritura
        self.console.insert(tk.END, full_msg)
        self.console.see(tk.END) # Auto-scroll al final
        self.console.config(state='disabled') # Bloquear escritura

    def set_position(self, x, y, z):
        self.curr_x = x
        self.curr_y = y
        self.curr_z = z
        self.draw_robot()

    def set_gripper(self, is_open):
        self.gripper_open = is_open
        self.draw_robot()

    def _to_screen(self, val_h, val_v, canvas_type="side"):
        if canvas_type == "side":
            screen_x = self.origin_x + (val_h * self.scale)
            screen_y = self.origin_y - (val_v * self.scale)
        else:
            screen_x = self.origin_x + (val_h * self.scale)
            screen_y = (self.height // 2) - (val_v * self.scale)
        return screen_x, screen_y

    def _draw_grid(self, canvas, width, height):
        """Dibuja una cuadrícula sutil para referencia"""
        step = 50
        for i in range(0, width, step):
            canvas.create_line(i, 0, i, height, fill=self.COLOR_GRID, dash=(2, 4))
        for i in range(0, height, step):
            canvas.create_line(0, i, width, i, fill=self.COLOR_GRID, dash=(2, 4))

    def solve_ik_2d(self, target_r, target_z):
        """
        Cinemática Inversa Mejorada: Mantiene la rigidez de los brazos
        incluso si el objetivo está fuera de alcance.
        """
        # Altura relativa desde el hombro (la base L1 es fija)
        z_rel = target_z - self.L1
        
        # Distancia directa (hipotenusa) desde el hombro al objetivo
        D = math.sqrt(target_r**2 + z_rel**2)
        
        # Radio máximo del brazo (suma de los dos eslabones móviles)
        max_reach = self.L2 + self.L3
        
        # --- CASO 1: FUERA DE ALCANCE (Aquí arreglamos el "brazo elástico") ---
        if D > max_reach or D == 0:
            # El robot no llega. Lo dibujamos totalmente estirado hacia el objetivo.
            # Calculamos el vector unitario y multiplicamos por el largo del L2
            ratio = self.L2 / D if D > 0 else 0
            
            # El codo queda en la dirección del objetivo, a distancia L2
            elbow_r = target_r * ratio
            elbow_z = self.L1 + (z_rel * ratio)
            return elbow_r, elbow_z

        # --- CASO 2: DENTRO DE ALCANCE (Matemática normal) ---
        try:
            # Teorema del coseno para hallar el ángulo interno del codo
            alpha = math.acos((self.L2**2 + D**2 - self.L3**2) / (2 * self.L2 * D))
            
            # Ángulo de elevación del objetivo
            theta_base = math.atan2(z_rel, target_r)
            
            # Ángulo total del primer brazo
            theta_1 = theta_base + alpha 
            
            # Convertimos coordenadas polares a cartesianas para el codo
            elbow_r = self.L2 * math.cos(theta_1)
            elbow_z = self.L1 + (self.L2 * math.sin(theta_1))
            
            return elbow_r, elbow_z
            
        except ValueError:
            # Si algo falla matemáticamente (ej. punto muy pegado al cuerpo), 
            # mantenemos el brazo vertical.
            return 0, self.L1 + self.L2

    def draw_robot(self):
        # Limpiar
        self.canvas_top.delete("all")
        self.canvas_side.delete("all")
        
        # Dibujar Grillas de fondo
        self._draw_grid(self.canvas_top, self.width, self.width)
        self._draw_grid(self.canvas_side, self.width, self.height)

        # ==========================================
        # VISTA SUPERIOR (TOP)
        # ==========================================
        cx, cy = self._to_screen(0, 0, "top")
        tx, ty = self._to_screen(self.curr_x, self.curr_y, "top")

        # 1. Base Rotatoria (Círculo grande con detalles)
        r_base = 20
        self.canvas_top.create_oval(cx-r_base, cy-r_base, cx+r_base, cy+r_base, 
                                    fill=self.COLOR_BASE, outline=self.COLOR_LINK, width=2)
        # Tornillos base
        self.canvas_top.create_oval(cx-3, cy-3, cx+3, cy+3, fill=self.COLOR_JOINT)

        # 2. Brazo (Rectángulo grueso en lugar de línea)
        self.canvas_top.create_line(cx, cy, tx, ty, width=12, capstyle=tk.ROUND, fill=self.COLOR_LINK)
        self.canvas_top.create_line(cx, cy, tx, ty, width=4, capstyle=tk.ROUND, fill="#F0F0F0") # Brillo central

        # 3. Gripper (Visto desde arriba)
        grip_col = self.COLOR_GRIPPER if self.gripper_open else self.COLOR_GRIPPER_OFF
        self.canvas_top.create_rectangle(tx-8, ty-8, tx+8, ty+8, fill=grip_col, outline="white")

        # Texto Info
        self.canvas_top.create_text(10, 10, anchor="nw", fill=self.COLOR_TEXT,
                                    text=f"X: {self.curr_x:.1f} mm\nY: {self.curr_y:.1f} mm")


        # ==========================================
        # VISTA LATERAL (SIDE)
        # ==========================================
        R = math.sqrt(self.curr_x**2 + self.curr_y**2)
        elbow_r, elbow_z = self.solve_ik_2d(R, self.curr_z)

        base_x, base_y = self._to_screen(0, 0, "side")
        shoulder_x, shoulder_y = self._to_screen(0, self.L1, "side")
        elbow_x, elbow_y = self._to_screen(elbow_r, elbow_z, "side")
        wrist_x, wrist_y = self._to_screen(R, self.curr_z, "side")

        # 1. Pedestal (Base trapezoidal)
        # Coordenadas: (Izquierda abajo, Izquierda arriba, Derecha arriba, Derecha abajo)
        pedestal_coords = [
            base_x - 30, base_y,           # Suelo izq
            base_x - 15, shoulder_y + 10,  # Hombro izq
            base_x + 15, shoulder_y + 10,  # Hombro der
            base_x + 30, base_y            # Suelo der
        ]
        self.canvas_side.create_polygon(pedestal_coords, fill=self.COLOR_BASE, outline="#666", width=2)
        
        # Línea de suelo
        self.canvas_side.create_line(0, base_y, self.width, base_y, fill="#666", width=2)

        # 2. Eslabones (Brazos) - Usamos lineas muy gruesas con bordes redondeados
        # Brazo 1
        self.canvas_side.create_line(shoulder_x, shoulder_y, elbow_x, elbow_y, 
                                     width=14, capstyle=tk.ROUND, fill=self.COLOR_LINK)
        # Brazo 2
        self.canvas_side.create_line(elbow_x, elbow_y, wrist_x, wrist_y, 
                                     width=10, capstyle=tk.ROUND, fill=self.COLOR_LINK)

        # 3. Articulaciones (Joints) - Círculos naranjas con "tornillo"
        def draw_joint(x, y, radius=8):
            self.canvas_side.create_oval(x-radius, y-radius, x+radius, y+radius, 
                                         fill=self.COLOR_JOINT, outline="black", width=1)
            # Tornillo central
            self.canvas_side.create_oval(x-2, y-2, x+2, y+2, fill="#333")

        draw_joint(shoulder_x, shoulder_y, 10) # Hombro
        draw_joint(elbow_x, elbow_y, 8)        # Codo
        draw_joint(wrist_x, wrist_y, 6)        # Muñeca

        # 4. Gripper (Pinza)
        self._draw_gripper_side(wrist_x, wrist_y)

        # Texto Info
        self.canvas_side.create_text(10, 10, anchor="nw", fill=self.COLOR_TEXT,
                                     text=f"Radio: {R:.1f} mm\nAltura Z: {self.curr_z:.1f} mm")

    def _draw_gripper_side(self, x, y):
        """Dibuja una pinza simple que se abre o cierra"""
        color = self.COLOR_GRIPPER if self.gripper_open else self.COLOR_GRIPPER_OFF
        
        # Tamaño de apertura
        gap = 10 if self.gripper_open else 2
        length = 15
        
        # Dedo superior
        self.canvas_side.create_polygon([
            x, y - 2,
            x + length, y - 2 - gap,
            x + length, y - 6 - gap,
            x, y - 6
        ], fill=color, outline="black")

        # Dedo inferior
        self.canvas_side.create_polygon([
            x, y + 2,
            x + length, y + 2 + gap,
            x + length, y + 6 + gap,
            x, y + 6
        ], fill=color, outline="black")