#!/usr/bin/env python3
"""Backup consistente, inclusive com o serviço em execução; destino novo obrigatório."""
import argparse, pathlib, sqlite3, os
p=argparse.ArgumentParser();p.add_argument('source');p.add_argument('destination');a=p.parse_args()
src=pathlib.Path(a.source).resolve();dst=pathlib.Path(a.destination).resolve()
if not src.is_file():p.error('Banco de origem inexistente')
if dst.exists():p.error('Destino já existe; escolha um novo nome')
dst.parent.mkdir(parents=True,exist_ok=True)
fd=os.open(dst,os.O_CREAT|os.O_EXCL|os.O_WRONLY,0o600);os.close(fd)
try:
    with sqlite3.connect(src.as_uri()+'?mode=ro',uri=True) as origin, sqlite3.connect(dst) as target:
        origin.backup(target)
        assert target.execute('PRAGMA integrity_check').fetchone()[0]=='ok'
        assert not target.execute('PRAGMA foreign_key_check').fetchall()
    print(dst)
except BaseException:
    dst.unlink(missing_ok=True)
    raise
