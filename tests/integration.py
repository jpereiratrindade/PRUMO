#!/usr/bin/env python3
import urllib.request,urllib.error,json,tempfile,subprocess,time,socket,pathlib,sqlite3,sys,unittest
EXE=str(pathlib.Path(sys.argv[1]).resolve());ROOT=pathlib.Path(__file__).resolve().parents[1]
class Integration(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.temp=tempfile.TemporaryDirectory(prefix='prumo-test-');cls.data=pathlib.Path(cls.temp.name)/'data'
  with socket.socket() as s:s.bind(('127.0.0.1',0));cls.port=s.getsockname()[1]
  cls.start()
 @classmethod
 def start(cls):
  cls.proc=subprocess.Popen([EXE,'--data',str(cls.data),'--port',str(cls.port)],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  for _ in range(100):
   try:cls.request('/health',prefix='');return
   except (OSError,urllib.error.URLError):time.sleep(.05)
  raise RuntimeError(cls.proc.communicate(timeout=1))
 @classmethod
 def tearDownClass(cls):
  cls.proc.terminate();cls.proc.communicate(timeout=5);cls.temp.cleanup()
 @classmethod
 def request(cls,path,method='GET',b=None,expected=200,prefix='/api/v1',headers=None):
  h={'Content-Type':'application/json','X-Prumo-Local':'1',**(headers or {})}
  req=urllib.request.Request(f'http://127.0.0.1:{cls.port}'+prefix+path,headers=h,method=method,data=json.dumps(b).encode() if b is not None else None)
  try:
   with urllib.request.urlopen(req,timeout=5) as r:status=r.status;result=json.loads(r.read())
  except urllib.error.HTTPError as e:status=e.code;result=json.loads(e.read());e.close()
  assert status==expected,(path,status,expected,result)
  return result
 def create(self,kind,title,p=None,**kw):
  return self.request('/entities','POST',{'kind':kind,'data':{'title':title,**kw},'actor':'Teste local','project_id':p or ''},201)
 def edit(self,e,**kw):
  return self.request('/entities/'+e['id'],'PUT',{'kind':e['kind'],'data':{**e['data'],**kw},'project_id':e['project_id'] or '', 'revision':e['revision'],'actor':'Revisor local','reason':'Verificação de teste'})
 def test_full_workflow(self):
  # Importação da fonte e reimportação preservando edição operacional.
  imp=self.request('/import','POST',{});self.assertTrue(imp['imported'])
  records=self.request('/entities');p='30879320260001-006183';cop=[e for e in records if e['project_id']==p]
  self.assertEqual(len([e for e in cop if e['kind']=='goal']),3)
  self.assertEqual(len([e for e in cop if e['kind']=='commitment']),13)
  self.assertEqual(len([e for e in cop if e['kind']=='activity' and e['id'].startswith('AP')]),21)
  self.assertEqual(len([e for e in cop if e['kind']=='condition' and e['data']['state']=='unverified']),5)
  self.assertTrue(all(e['data']['state']=='unknown' for e in cop if e['kind']=='commitment'))
  e06=next(e for e in cop if e['id']=='E06');self.edit(e06,next_step='Conferir cadastro na reunião')
  again=self.request('/import','POST',{});self.assertFalse(again['imported']);self.assertEqual(len(self.request('/entities')),len(records))
  self.assertEqual(self.request('/entities/E06')['data']['next_step'],'Conferir cadastro na reunião')
  now=self.request('/now?project='+p);self.assertTrue(all(x['state']=='proposed' for x in now['links']))
  self.assertTrue(all(x['reviews']==0 for x in now['acceptance_totals'].values()))
  # CRUD de domínio independente e trilha de antes/depois.
  project=self.create('project','Projeto independente');pid=project['id'];project=self.edit(project,description='Edição do projeto funciona')
  c=self.create('commitment','Produto verificável',pid,quantity=10,unit='produto')
  activity=self.create('activity','Preparar produto',pid,state='planned')
  milestone=self.create('milestone','Revisão interna',pid,due_internal='2026-12-01',due_formal='2028-03-30')
  occurrence=self.create('occurrence','Reunião de preparação',pid,fact_at='2026-11-24T12:00:00Z')
  self.create('decision','Revisar instrumento antes de campo',pid)
  evidence=self.create('evidence','Ata referenciada',pid,reference='dossie-local/ata-v1.pdf',access='internal',origin='Ata de revisão')
  cid=c['id'];b={'reviewer':'Revisor declarado','delivery_key':'lote-A','quantity':2,'evidence_ids':[evidence['id']],'rationale':'Conferência documentada'}
  self.request('/commitments/'+cid+'/accept','POST',b,422)
  c=self.edit(c,criteria='Documento revisado e protocolado')
  self.request('/commitments/'+cid+'/accept','POST',{**b,'evidence_ids':[]},422)
  self.request('/commitments/'+cid+'/accept','POST',{**b,'evidence_ids':['fonte-integral']},422)
  accepted=self.request('/commitments/'+cid+'/accept','POST',b,201)
  self.request('/entities/'+cid,'PUT',{'kind':'commitment','project_id':pid,'revision':c['revision'],'actor':'Teste','reason':'Redução inconsistente','data':{**c['data'],'quantity':1}},409)
  self.request('/commitments/'+cid+'/accept','POST',b,409)
  self.request('/commitments/'+cid+'/accept','POST',{**b,'delivery_key':'lote-B','quantity':20},422)
  self.request('/entities/'+cid,'DELETE',{'actor':'Teste','reason':'não deve apagar aceite','revision':c['revision']},409)
  self.request('/acceptances/'+accepted['id']+'/revoke','POST',{'actor':'Revisor','reason':'Nova análise documental'})
  self.assertEqual(self.request('/now?project='+pid)['acceptance_totals'][cid]['quantity'],0)
  # Independência E06/E07/E08: aceitar 1 família não aceita entrevista/georreferência.
  ev=self.create('evidence','Dossiê agregado sem pessoas',p,reference='dossie/agregado')
  self.request('/commitments/E06/accept','POST',{**b,'quantity':1,'evidence_ids':[ev['id']]},201)
  totals=self.request('/now?project='+p)['acceptance_totals'];self.assertEqual([totals[k]['quantity'] for k in ['E06','E07','E08']],[1,0,0])
  # Condição e impedimento entram em Agora; resolução fundamentada os retira.
  condition=self.create('condition','Equipe preparada',pid,state='unverified')
  block=self.create('impediment','Veículo pendente',pid,state='open')
  ids=lambda:[x['entity']['id'] for x in self.request('/now?project='+pid)['items']]
  self.assertIn(condition['id'],ids());self.assertIn(block['id'],ids())
  self.request('/entities/'+condition['id'],'PUT',{'kind':'condition','data':{**condition['data'],'state':'satisfied'},'project_id':pid,'revision':1,'actor':'Teste','reason':'mudança sem fundamento'},422)
  self.edit(condition,state='satisfied',resolution='Treinamento conferido na ata')
  self.edit(block,state='resolved',resolution='Disponibilidade conferida na agenda')
  self.assertNotIn(condition['id'],ids());self.assertNotIn(block['id'],ids())
  # Versão concorrente, rollback, validação e origem HTTP.
  self.edit(activity,description='Nova revisão')
  self.request('/entities/'+activity['id'],'PUT',{'kind':'activity','data':activity['data'],'project_id':pid,'revision':1,'actor':'Outro','reason':'Edição obsoleta'},409)
  for bad in [{'title':''},{'title':'x','quantity':-1},{'title':'x','state':'accepted'},{'title':'x','coordinates':[1,2]}]:self.request('/entities','POST',{'kind':'commitment','data':bad,'project_id':pid,'actor':'Teste'},422)
  self.request('/entities','POST',{'kind':'activity'},400)
  self.request('/import','POST',{},403,headers={'Origin':'https://external.invalid'})
  self.request('/import','POST',{},403,headers={'X-Prumo-Local':''})
  # M:N e precedência sem ciclos ou vínculos entre projetos.
  base={'source_id':activity['id'],'target_id':cid,'type':'activity_commitment','actor':'Teste','state':'confirmed'}
  self.request('/links','POST',base,201)
  self.request('/links','POST',{**base,'target_id':'E06'},422)
  self.request('/links','POST',{**base,'type':'precedence','target_id':milestone['id']},201)
  self.request('/links','POST',{**base,'type':'precedence','source_id':milestone['id'],'target_id':activity['id']},409)
  self.request('/entities/'+milestone['id'],'DELETE',{'actor':'Teste','reason':'Marco replanejado','revision':1})
  self.request('/entities/'+milestone['id'],expected=404)
  history=self.request('/events?project='+pid);self.assertTrue(any(e['type']=='edited' and 'before' in e['payload'] for e in history));self.assertTrue(any(e['type']=='acceptance_revoked' for e in history))
  # Backup com WAL ativo, integridade, restauração em diretório novo e reinício.
  backup=pathlib.Path(self.temp.name)/'snapshot.sqlite3';restored=pathlib.Path(self.temp.name)/'restored'
  subprocess.run([sys.executable,str(ROOT/'scripts/backup.py'),str(self.data/'prumo.sqlite3'),str(backup)],check=True,capture_output=True)
  subprocess.run([sys.executable,str(ROOT/'scripts/restore.py'),str(backup),str(restored)],check=True,capture_output=True)
  with sqlite3.connect(backup) as db:
   self.assertEqual(db.execute('PRAGMA user_version').fetchone()[0],1)
   self.assertEqual(db.execute('PRAGMA integrity_check').fetchone()[0],'ok');self.assertEqual(db.execute('PRAGMA foreign_key_check').fetchall(),[])
   with self.assertRaises(sqlite3.IntegrityError):db.execute("UPDATE events SET actor='alteração'")
  self.proc.terminate();self.proc.communicate(timeout=5);type(self).data=restored;type(self).start()
  self.assertEqual(self.request('/entities/'+occurrence['id'])['data']['title'],'Reunião de preparação')
  self.assertEqual(self.request('/entities/'+cid)['data']['criteria'],'Documento revisado e protocolado')
  # Limites e rollback: entrada inválida não deixa registro; aceites não somem por edição.
  before=len(self.request('/entities'))
  self.request('/entities','POST',{'kind':'condition','data':{'title':'Verificação sem fundamento','state':'satisfied'},'project_id':pid,'actor':'Teste'},422)
  self.request('/entities','POST',{'kind':'milestone','data':{'title':'Data impossível','due_internal':'2026-02-30'},'project_id':pid,'actor':'Teste'},422)
  self.assertEqual(before,len(self.request('/entities')))
  self.request('/entities','POST',{'kind':'activity','data':{'title':'x','description':'x'*140000},'project_id':pid,'actor':'Teste'},413)
  self.request('/health',prefix='',headers={'Host':'external.invalid'},expected=403)
  # Uma falha tardia de importação desfaz toda a carga.
  manifest=json.loads((ROOT/'seed/manifest.json').read_text());manifest['entities'].append(manifest['entities'][-1])
  broken=pathlib.Path(self.temp.name)/'broken-seed.json';broken.write_text(json.dumps(manifest))
  atomic_data=pathlib.Path(self.temp.name)/'atomic'
  result=subprocess.run([EXE,'--data',str(atomic_data),'--import','--seed',str(broken)],capture_output=True)
  self.assertNotEqual(result.returncode,0)
  with sqlite3.connect(atomic_data/'prumo.sqlite3') as db:
   self.assertEqual(db.execute('SELECT COUNT(*) FROM entities').fetchone()[0],0)
   self.assertEqual(db.execute('SELECT COUNT(*) FROM events').fetchone()[0],0)
  # Bancos de versões futuras são recusados sem downgrade.
  future=pathlib.Path(self.temp.name)/'future';future.mkdir()
  with sqlite3.connect(future/'prumo.sqlite3') as db:db.execute('PRAGMA user_version=99')
  result=subprocess.run([EXE,'--data',str(future),'--import'],capture_output=True);self.assertNotEqual(result.returncode,0)
  # Porta ocupada não derruba serviço; manifesto adulterado não entra.
  port_conflict=subprocess.run([EXE,'--data',str(pathlib.Path(self.temp.name)/'other'),'--port',str(self.port)],capture_output=True,timeout=10)
  self.assertNotEqual(port_conflict.returncode,0);self.assertIn('Porta indisponível',port_conflict.stderr.decode())
  manifest=json.loads((ROOT/'seed/manifest.json').read_text());manifest['sha256']='0'*64;bad=pathlib.Path(self.temp.name)/'bad.json';bad.write_text(json.dumps(manifest))
  result=subprocess.run([EXE,'--data',str(restored),'--import','--seed',str(bad)],capture_output=True);self.assertNotEqual(result.returncode,0)
  print('Demonstrado: importação idempotente, CRUD, revisão, conflitos, ciclos, independência de aceites, Agora, histórico, backup/restauração e reinício.')
if __name__=='__main__':unittest.main(argv=[sys.argv[0]],verbosity=2)
