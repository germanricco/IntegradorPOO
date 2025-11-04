#!/usr/bin/env python3
from xmlrpc.client import ServerProxy, Fault

class CLI:
    def __init__(self, url):
        self.api = ServerProxy(url, allow_none=True)
        self.token = None
        self.username = None
        self.role = None

    # ===== helpers =====
    def _refresh_me(self):
        if not self.token:
            self.username = self.role = None
            return
        try:
            r = self.api.__getattr__("auth.me")({"token": self.token})
            # Ajustar a tu contrato real de auth.me si difiere
            self.username = r.get("user") or r.get("username")
            self.role     = r.get("privilegio") or r.get("role")
        except Exception:
            # token inválido o server reiniciado
            self.username = self.role = self.token = None

    def _is_admin(self):
        self._refresh_me()
        return self.role == "admin"

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
        print("  methods                      # listar métodos RPC publicados")
        print("  help                         # esta ayuda")
        print("  help-rpc <metodo>            # ver help() del método en el servidor")
        print("  login <user> <pass>")
        print("  me")
        print("  logout")
        if self._is_admin():
            print("  uadd <user> <pass> [role] [active=1]   # user.register")
            print("  ulist                                 # user.list")
            print("  uupd <id> active=0|1                   # user.update (habilitar/deshabilitar)")
        print("  mychpass <old> <new>       # user.changePassword (autoservicio)")
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
                    # contrato actual del server
                    self.token    = r["token"]
                    self.username = r.get("username") or r.get("user")
                    self.role     = r.get("role") or r.get("privilegio")
                    print(f"login OK — user={self.username}, role={self.role}")

            elif cmd == "me":
                self._refresh_me()
                if self.username:
                    print({"ok": True, "user": self.username, "role": self.role})
                else:
                    print("No hay sesión activa.")

            elif cmd == "logout":
                if not self.token:
                    print("No hay sesión activa.")
                else:
                    print(self.api.__getattr__("auth.logout")({"token": self.token}))
                    self.token = self.username = self.role = None

            elif cmd == "uadd":
                if not self._is_admin():
                    print("No sos admin.")
                elif len(args) < 2:
                    print("uso: uadd <user> <pass> [role] [active=1]")
                else:
                    user = args[0]
                    pw   = args[1]
                    role = args[2] if len(args) >= 3 else "operator"
                    active = True
                    if len(args) >= 4:
                        kv = args[3]
                        if kv.startswith("active="):
                            active = (kv.split("=",1)[1] == "1")
                    r = self.api.__getattr__("user.register")({
                        "user": user, "pass": pw, "role": role, "active": active
                    })  # el server ignora 'token' hoy, pero podrías agregarlo si lo validás
                    print(r)

            elif cmd == "ulist":
                if not self._is_admin():
                    print("No sos admin.")
                else:
                    r = self.api.__getattr__("user.list")({})
                    users = r["users"]
                    rows = [[u["id"], u["username"], u["role"], "sí" if u["active"] else "no"] for u in users]
                    self._print_table(rows, headers=["id", "username", "role", "active"])

            elif cmd == "uupd":
                if not self._is_admin():
                    print("No sos admin.")
                elif len(args) < 2 or not args[1].startswith("active="):
                    print("uso: uupd <id> active=0|1")
                else:
                    uid = int(args[0])
                    active = (args[1].split("=",1)[1] == "1")
                    print(self.api.__getattr__("user.update")({"id": uid, "active": active}))

            elif cmd == "mychpass":
                if len(args) != 2:
                    print("uso: mychpass <old> <new>")
                else:
                    self._refresh_me()
                    if not self.username:
                        print("Primero hacé login.")
                    else:
                        old, newp = args
                        print(self.api.__getattr__("user.changePassword")({
                            "user": self.username, "old": old, "new": newp
                        }))

            elif cmd == "quit":
                return False

            else:
                print("Comando desconocido. Escribí 'help'.")

        except Fault as e:
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

