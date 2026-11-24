# API local v1

Base: `http://127.0.0.1:8786`. Recursos locais estão no mesmo servidor. GET não modifica estado. Escritas exigem `Content-Type: application/json` e `X-Prumo-Local: 1`; quando Origin está presente, deve ser exatamente a origem local. Não há autenticação; `actor`/`reviewer` são declarações locais.

| Método | Rota | Operação |
|---|---|---|
| GET | `/health` | Versão, esquema e limite de identidade |
| GET | `/api/v1/entities?project=ID` | Lista registros ativos; sem filtro, todos os projetos |
| GET | `/api/v1/entities/ID` | Envelope, revisão e payload |
| POST | `/api/v1/entities` | Cria entidade (201) |
| PUT | `/api/v1/entities/ID` | Edita payload integral; exige revisão e motivo |
| DELETE | `/api/v1/entities/ID` | Arquiva com revisão, ator e motivo; não apaga eventos |
| GET | `/api/v1/now?project=ID` | Pendências com razões, totais, relações e aceites |
| GET | `/api/v1/events?project=ID` | Histórico preservado com payload e datas |
| POST | `/api/v1/commitments/ID/accept` | Revisão explícita de entrega (201) |
| POST | `/api/v1/acceptances/ID/revoke` | Revogação fundamentada |
| POST | `/api/v1/links` | Propor/confirmar/rejeitar vínculo ou precedência |
| POST | `/api/v1/import` | Importa manifesto inaugural ou retorna ausência de mudança |
| GET | `/api/v1/source` | Documento original em texto, sem conversão para execução |

Criação de projeto:

```json
{"kind":"project","actor":"Operador local declarado","data":{"title":"Projeto independente"}}
```

Criação de compromisso:

```json
{"kind":"commitment","project_id":"ID_DO_PROJETO","actor":"Operador local declarado","data":{"title":"Relatório","quantity":2,"unit":"relatório","criteria":"Revisado e protocolado","state":"planned"}}
```

Tipos: `project`, `goal`, `commitment`, `activity`, `milestone`, `condition`, `occurrence`, `evidence`, `decision`, `impediment`. Título é necessário; o restante pode começar incompleto. `id`, `external_key` são usados pela importação; clientes normais deixam o backend gerar IDs. Não mova entidade entre projetos/tipos. Projetos têm `project_id: null` por serem raízes.

Edição usa o mesmo envelope com `revision` corrente, `reason` e `data` integral (campos omitidos do payload são removidos; o formulário preserva metadados extras). Condição/impedimento verificados exigem `resolution`. Campos de data usam `AAAA-MM-DD`; data do fato usa `AAAA-MM-DDTHH:MM:SSZ`. Hora de registro é do servidor em UTC.

Estados declarados: `unknown`, `unverified`, `draft`, `planned`, `executed`, `delivered`, `open`, `resolved`, `satisfied`, `unsatisfied`. Condições só permitem `unverified`, `satisfied`, `unsatisfied`. `accepted` não é um estado gravável diretamente.

Aceite:

```json
{"reviewer":"Revisor local declarado","delivery_key":"dossie-lote-01-v1","quantity":1,"evidence_ids":["ID_EVIDENCIA"],"rationale":"Critérios conferidos na revisão documentada"}
```

Evidência é entidade com `reference` não vazia; `origin`, `access` (`public`, `internal`, `restricted`) e `fact_at` contextualizam o registro. A API permite múltiplas evidências por revisão; o formulário oferece uma por operação. Não há upload, navegação de arquivo externo ou validação material automática.

Revogação: `{"actor":"Revisor local","reason":"Revisão posterior"}`. A chave do lote permanece ocupada para evitar recontagem da mesma revisão; use versão nova se necessário.

Relação:

```json
{"source_id":"ID_ATIVIDADE","target_id":"ID_COMPROMISSO","type":"activity_commitment","state":"proposed","actor":"Operador local","reason":"Vínculo para revisão"}
```

Tipos `activity_commitment` (atividade → compromisso) e `precedence` (origem precede destino); estados `proposed`, `confirmed`, `rejected`. Mesma tripla atualiza estado e gera novo evento. Propostas também são consideradas na prevenção de ciclos, conservadoramente.

Erros retornam `{"error":"mensagem"}`: 400 JSON/campo obrigatório/tipo inválido; 403 origem/Host/cabeçalho de escrita; 404 ausente/arquivado; 409 revisão obsoleta, chave duplicada, ciclo, limite ou conflito de importação; 413 payload >128 KiB; 415 Content-Type; 422 validação contextual; 500 falha interna. Algumas violações SQLite de integridade retornam 422. Não há paginação nesta versão: volume inicial é pequeno e ela deverá anteceder cargas extensas.
