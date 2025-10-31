#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import csv, sqlite3, hashlib, sys, os, argparse

# Usa el MISMO SALT que el servidor (init_auth_layer("...","<SALT>"))
SALT = os.environ.get("AUTH_SALT", "cambia_este_salt")

# ---------- Utilidades ----------

def sha256_hash(plain: str) -> str:
    return hashlib.sha256((SALT + plain).encode()).hexdigest()

def sniff_csv(path):
    # Intenta detectar separador y si hay header
    with open(path, "rb") as fb:
        raw = fb.read(4096)
    text = raw.decode("utf-8-sig", errors="replace")
    sniffer = csv.Sniffer()
    dialect = sniffer.sniff(text, delimiters=",;|\t")
    has_header = sniffer.has_header(text)
    return dialect, has_header

def norm(s):
    return (s or "").strip()

def parse_active(v, default_active: int) -> int:
    if v is None:
        return int(default_active)
    vv = norm(v).lower()
    return 1 if vv in ("1","true","t","yes","y","si","sí") else 0

def is_sha256_hex(s: str) -> bool:
    ss = norm(s)
    return len(ss) == 64 and all(c in "0123456789abcdef" for c in ss.lower())

def map_role(r: str, default_role: str) -> str:
    r = norm(r).lower()
    aliases = {
        "op": "operator",
        "operador": "operator",
        "operario": "operator",
        "adm": "admin",
        "administrador": "admin",
        "lector": "viewer",
        "lectura": "viewer",
        "read": "viewer",
        "viewer": "viewer",
        "operator": "operator",
        "admin": "admin",
    }
    mapped = aliases.get(r, r if r else default_role)
    if mapped not in ("admin", "operator", "viewer"):
        mapped = default_role
    return mapped

# ---------- Main ----------

def main():
    ap = argparse.ArgumentParser(description="Migrar usuarios desde CSV a SQLite (flexible).")
    ap.add_argument("--csv", default="users.csv", help="ruta al CSV (por defecto users.csv)")
    ap.add_argument("--db",  default="data/db/poo.db", help="ruta a la DB SQLite")

    # Columnas por nombre (si hay header)
    ap.add_argument("--user-col",   help="nombre de columna para usuario (p.ej. username)")
    ap.add_argument("--pass-col",   help="nombre de columna para contraseña o hash (p.ej. pass_hash)")
    ap.add_argument("--role-col",   help="nombre de columna para rol (p.ej. privilegio) [opcional]")
    ap.add_argument("--active-col", help="nombre de columna para activo (p.ej. habilitado) [opcional]")

    # CSV sin encabezado (índices 0-based)
    ap.add_argument("--no-header", action="store_true", help="indica que el CSV no tiene encabezado")
    ap.add_argument("--user-idx", type=int, help="índice de usuario si no hay header")
    ap.add_argument("--pass-idx", type=int, help="índice de contraseña si no hay header")
    ap.add_argument("--role-idx", type=int, help="índice de rol si no hay header")
    ap.add_argument("--active-idx", type=int, help="índice de activo si no hay header")

    ap.add_argument("--role-default", default="operator", help="rol por defecto si falta (operator/admin/viewer)")
    ap.add_argument("--active-default", type=int, default=1, help="1 o 0 si falta")

    args = ap.parse_args()

    if not os.path.exists(args.csv):
        print(f"ERROR: no existe {args.csv}")
        sys.exit(1)
    if not os.path.exists(args.db):
        print(f"ERROR: no existe {args.db}")
        sys.exit(1)

    # Detectar formato CSV
    dialect, guessed_header = sniff_csv(args.csv)
    has_header = False if args.no_header else guessed_header

    with open(args.csv, newline="", encoding="utf-8-sig") as f:
        rdr = csv.reader(f, dialect)
        headers = None
        headers_l = []
        if has_header:
            headers = next(rdr)
            headers_l = [norm(h).lower() for h in headers]

        def find_col(candidates):
            for c in candidates:
                if c in headers_l:
                    return headers_l.index(c)
            return None

        # Resolver posiciones de columnas
        if has_header:
            u_idx = headers_l.index(args.user_col.lower()) if args.user_col else find_col(["user","username","usuario","nombre_usuario"])
            p_idx = headers_l.index(args.pass_col.lower()) if args.pass_col else find_col(["pass","password","clave","password_hash","pass_hash"])
            r_idx = headers_l.index(args.role_col.lower()) if args.role_col else find_col(["role","rol","privilegio"])
            a_idx = headers_l.index(args.active_col.lower()) if args.active_col else find_col(["active","is_active","habilitado","activo"])
        else:
            u_idx = args.user_idx
            p_idx = args.pass_idx
            r_idx = args.role_idx
            a_idx = args.active_idx

        if u_idx is None or p_idx is None:
            print("ERROR: no pude detectar columnas de usuario y contraseña.")
            if has_header:
                print("Encabezados detectados:", headers_l)
                print("Usá --user-col y --pass-col para indicar los nombres correctos.")
            else:
                print("Usá --no-header y --user-idx/--pass-idx para indicar las posiciones.")
            sys.exit(2)

        con = sqlite3.connect(args.db)
        cur = con.cursor()

        migrated, skipped = 0, 0
        for row in rdr:
            if not row:
                continue

            # Extraer valores
            user = norm(row[u_idx]) if u_idx is not None and u_idx < len(row) else ""
            rawp = norm(row[p_idx]) if p_idx is not None and p_idx < len(row) else ""
            role = args.role_default
            if r_idx is not None and r_idx < len(row):
                role = norm(row[r_idx]) or role
            active = args.active_default
            if a_idx is not None and a_idx < len(row):
                active = parse_active(row[a_idx], args.active_default)

            if not user:
                skipped += 1
                continue

            # Evitar duplicados
            cur.execute("SELECT id FROM users WHERE username=?", (user,))
            if cur.fetchone():
                skipped += 1
                continue

            # Mapear rol a valores válidos
            role = map_role(role, args.role_default)

            # ¿hash ya calculado o plain?
            if is_sha256_hex(rawp):
                h = rawp
            else:
                h = sha256_hash(rawp)

            try:
                cur.execute(
                    "INSERT INTO users(username,password_hash,role,is_active) VALUES (?,?,?,?)",
                    (user, h, role, int(active))
                )
                migrated += 1
            except sqlite3.IntegrityError as e:
                # Cualquier constraint (CHECK role, UNIQUE username, etc.)
                print(f"[SKIP] {user}: {e}")
                skipped += 1

        con.commit()
        con.close()

    print(f"OK. Migrados: {migrated}, omitidos: {skipped}")
    print('Tip: verificá con: sqlite3 data/db/poo.db "SELECT id,username,role,is_active FROM users ORDER BY id;"')

if __name__ == "__main__":
    main()

