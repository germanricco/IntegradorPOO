#!/usr/bin/env python3
from xmlrpc.client import ServerProxy, Fault

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

