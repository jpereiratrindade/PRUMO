#!/usr/bin/env python3
"""Deriva o manifesto revisável da fonte preservada. Não grava dados operacionais."""
import pathlib, hashlib, json, re
root=pathlib.Path(__file__).resolve().parents[1]
p=next((root/'seed/sources').glob('*.md'))
s=p.read_text(); project='30879320260001-006183'; entities=[]; links=[]
def add(id,kind,title,**data):
    data={'title':title,'state':'unknown','source':p.name,'source_version':'0.1.0',**data}
    e={'id':id,'kind':kind,'data':data,'external_key':'coptil-v1:'+id}
    if kind!='project': e['project_id']=project
    entities.append(e)
    return id
responsible='Leandro Bochi da Silva Volk'
collaborators='José Pedro Pereira Trindade; Alberi Noronha; Emerson Capelesso'
add(project,'project','TED MAPA–Embrapa/COPTIL',formal_start='2026-07-30',formal_end='2028-03-30',budget_brl=334250.00,responsible=responsible,collaborators=collaborators,territory='Hulha Negra; Candiota; Aceguá (RS)',description='Progresso real desconhecido. Os 100 questionários históricos de 2024 não são entregas. Piloto de 20 e painel de 60 são propostas condicionadas. Fonte: proposta de pactuação, não prova de execução.')
for n,title in enumerate(['Gestão e operação','Caracterização e diagnóstico','Monitoramento e diretrizes'],1):add(f'M{n}','goal',f'Meta {n} — {title}')
for line in s.splitlines():
    cols=[x.strip() for x in line.strip('|').split('|')]
    if re.fullmatch('E[0-9]{2}',cols[0]):
        id,title,quantity,criteria,internal=cols
        n=int(re.search(r'Meta (\d)',title)[1]); q=int(quantity.split()[0])
        unit='bolsista-mês' if 'bolsista-mês' in quantity else {'E06':'família/unidade','E07':'entrevista','E08':'unidade georreferenciada','E12':'oficina'}.get(id,'produto')
        add(id,'commitment',title,goal_id=f'M{n}',quantity=q,unit=unit,criteria=criteria,internal_window=internal,due_formal='2028-03-30')
        add('marco-'+id,'milestone',id+' — '+internal,description='Marco operacional proposto, revisável; não altera vigência formal.',related_id=id,internal_window=internal,due_formal='2028-03-30',state='planned')
    if re.fullmatch(r'(2[2-9]|3[0-9]|4[0-2])',cols[0]) and len(cols)==5:
        order,title,detail,evidence,internal=cols
        add('AP'+order,'activity',title,description=detail,expected_evidence=evidence,internal_window=internal,external_ref='Nexo ordem '+order,goal_id='M'+title[1],responsible=responsible,collaborators=collaborators,formal_start='2026-07-30',formal_end='2028-03-30',origin='Minuta Plano de Trabalho_6.pdf')
    if cols[0] in ('P0','P1') and len(cols)==4:
        prio,title,output,owner=cols
        add('30d-'+str(sum(e['id'].startswith('30d-') for e in entities)+1),'activity',title,priority=prio,next_step=output,responsible=owner,due_internal='2026-12-23',state='planned',description='Prioridade proposta dos primeiros 30 dias, 24/11–23/12/2026.')
for i,title in enumerate(['Cadastro e elegibilidade definidos','Instrumento e QA aprovados','Equipe mínima treinada','Meios de deslocamento disponíveis','Fluxo de guarda e proteção dos dados aprovado'],1):
    add('gate-'+str(i),'condition',title,state='unverified',description='Gate de início de campo; exige verificação atribuída e fundamentada.')
add('fonte-integral','evidence','Plano integrado original v0.1.0',reference='/api/v1/source',origin='Documento entregue em seed/sources/',access='internal',description='Planejamento operacional; não comprova execução.',state='unverified')
# Sugestões humanas revisáveis, nunca relações confirmadas pela importação.
for ap,products in {27:['E01'],24:['E02','E03'],28:['E04'],35:['E05','E06','E07','E08'],30:['E07'],32:['E08'],34:['E09'],37:['E10'],41:['E11'],40:['E12'],42:['E13']}.items():
    for product in products: links.append({'source_id':'AP'+str(ap),'target_id':product,'type':'activity_commitment','state':'proposed','actor':'Importador local • sugestão para revisão','reason':'Proposta por afinidade temática; fonte não explicita vínculo AP–produto.'})
(root/'seed/manifest.json').write_text(json.dumps({'source':p.name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'entities':entities,'links':links},ensure_ascii=False,indent=2)+'\n')
print(f'{len(entities)} registros; 3 metas, 13 compromissos, 21 APs, 7 prioridades, 5 condições; {len(links)} propostas de vínculo')
