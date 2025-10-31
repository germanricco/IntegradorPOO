#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Resetear la tabla users (borrar todo) y sembrar 10 usuarios controlados
Usage:
  export AUTH_SALT="cambia_este_salt"
  python3 tools/reset_and_seed_users.py
"""

import sqlite3
import hashlib
import os
import sys

DB_PATH = "data/db/poo.db"
SALT = os.environ.get("AUTH_SALT", "cambia_este_salt")

USERS = [
    (1, "admin",    "admin",     "admin",    1),
    (2, "pepe",     "pepe1234",  "operator", 1),
    (3, "maria",    "maria1234", "operator", 1),
    (4, "juan",     "juan1234",  "operator", 0),
    (5, "luis",     "luis1234",  "viewer",   1),
    (6, "carla",    "carla1234", "operator", 1),
    (7, "ana",      "ana1234",   "viewer",   0),
    (8, "pablo",    "pablo1234", "operator", 1),
    (9, "laura",    "laura1234", "viewer",   1),
    (10,"santiago", "santiago1234","operator",1),
]

def sha256_hash(plain):
    return hashlib.sha256((SALT + plain).encode()).hexdigest()

def main():
    if not os.path.exists(DB_PATH):
        print(f"ERROR: No existe la DB en {DB_PATH}. Crea la DB (ej: sqlite3 {DB_PATH} < db/schema.sql) y vuelve a intentar.")
        sys.exit(2)

    con = sqlite3.connect(DB_PATH)
    cur = con.cursor()

    # 1) Borrar todos los datos de users e resetear sqlite_sequence
    print("[*] Borrando tabla users y reseteando secuencia...")
    cur.execute("DELETE FROM users;")
    cur.execute("DELETE FROM sqlite_sequence WHERE name='users';")
    con.commit()

    # 2) Insertar los usuarios con id fijo y password hasheadas
    print("[*] Insertando usuarios seed...")
    for uid, username, plain_pass, role, active in USERS:
        phash = sha256_hash(plain_pass)
        try:
            cur.execute(
                "INSERT INTO users(id, username, password_hash, role, is_active) VALUES (?, ?, ?, ?, ?)",
                (uid, username, phash, role, active)
            )
        except sqlite3.IntegrityError as e:
            print(f"[WARN] No se pudo insertar {username} (id {uid}): {e}")

    con.commit()

    # 3) Ajustar sqlite_sequence para que el próximo id sea MAX(id)
    cur.execute("SELECT MAX(id) FROM users;")
    mx = cur.fetchone()[0] or 0
    cur.execute("DELETE FROM sqlite_sequence WHERE name='users';")
    if mx > 0:
        cur.execute("INSERT INTO sqlite_sequence(name, seq) VALUES('users', ?)", (mx,))
    con.commit()

    con.close()
    print(f"[OK] Seed completado. Usuarios insertados: {len(USERS)}. Next id = {mx+1}")

if __name__ == "__main__":
    main()

