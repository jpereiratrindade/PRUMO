#!/usr/bin/env python3
"""Restaura em diretório NOVO, sem sobrescrever bases ou serviços existentes."""
import argparse,pathlib,subprocess,sys,sqlite3
p=argparse.ArgumentParser();p.add_argument('backup');p.add_argument('new_data_directory');a=p.parse_args()
source=pathlib.Path(a.backup).resolve();dest=pathlib.Path(a.new_data_directory).resolve()
if dest.exists():p.error('Use um diretório novo; a restauração não sobrescreve dados')
with sqlite3.connect(source.as_uri()+'?mode=ro',uri=True) as db:
    if db.execute('PRAGMA integrity_check').fetchone()[0]!='ok' or db.execute('PRAGMA foreign_key_check').fetchall():p.error('Backup inválido')
    if db.execute('PRAGMA user_version').fetchone()[0]!=1:p.error('Versão incompatível')
dest.mkdir(parents=True,mode=0o700)
subprocess.run([sys.executable,str(pathlib.Path(__file__).with_name('backup.py')),str(source),str(dest/'prumo.sqlite3')],check=True)
