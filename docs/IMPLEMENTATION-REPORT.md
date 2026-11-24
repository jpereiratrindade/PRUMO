# Relatório de implementação — PRUMO 0.1.0

Data: 24/11/2026, America/Sao_Paulo. Instrução executada: PRUMO-PATCH-001 v0.1.0. Implementação local utilizável, com limites de primeira versão explicitados abaixo. Código autorizado para commit e envio SSH ao repositório GitHub PRUMO pelo usuário durante a execução; isso não publica o serviço web.

## Resultado operacional

Backend C++26 com interface nativa e persistência SQLite. Uma instância local foi iniciada em `http://127.0.0.1:8786`, com banco fora do checkout em `~/.local/share/prumo/prumo.sqlite3`. A base operacional contém somente a carga inaugural de planejamento; aceites e acontecimentos usados na demonstração foram criados em bases temporárias separadas, não contaminando o projeto real. O processo iniciado nesta sessão depende da permanência do ambiente; para execução duradoura use o comando do README ou instale manualmente a unidade de exemplo.

Há CRUD por criação, leitura, edição versionada e arquivamento de projetos, compromissos, atividades e marcos. Também é possível criar metas de agrupamento, condições, acontecimentos, decisões, impedimentos e referências de evidência. Edições preservam os valores anteriores em eventos. Atores são explicitamente locais e declarados; não representam identidade autenticada.

A interface oferece seleção de projeto, pesquisa, criação e edição por formulários, feedback de erro, diálogos nativos e layout responsivo. As cinco seções são funcionais: Agora deriva razões dos registros; Metas apresenta compromissos, atividades, marcos, vínculos e aceites; Campo apresenta território e preparação; Registro reúne acontecimentos e trajetória; Acervo reúne fontes e referências. Não há gráficos de progresso fabricados.

## Origem visual e tecnológica

A inspeção antecedeu a implementação e está registrada em [ARCHITECTURE.md](ARCHITECTURE.md). Foram lidos MORFO, NOESE e manifests do METIS no diretório pai. A identidade deriva de tokens e layout reais do MORFO; sistema de fontes nativo e SVGs próprios evitam dependências externas. Os seis ícones são prumo, orientação atual, metas, campo, registro e acervo, com títulos acessíveis e texto nos controles.

Build efetivamente executado: GCC 16.2.1, CMake 4.3.0, flag `-std=c++26`, SQLite 3.51.2, OpenSSL 3.5.9, cpp-httplib 0.28.0 e nlohmann/json 3.12.0. C++26 é o modo solicitado ao compilador, sem afirmar suporte integral ao padrão futuro. [build.json](evidence/build.json) preserva comandos reais e versões. Dependências HTTP/JSON foram copiadas do NOESE em leitura, com licenças e hashes locais; nenhum irmão foi editado.

## Importação inaugural demonstrada

Fonte original preservada em `seed/sources/`, sem alteração, com SHA-256:

`bcb9390652451439a9ef7f636e7a9cb52b9a6941c73adf8adba8f2ec211c87b1`

| Conteúdo | Resultado |
|---|---|
| Projeto | 30879320260001-006183, vigência 30/07/2026–30/03/2028, R$ 334.250,00 |
| Metas | Exatamente 3 |
| Tipos de entrega | Exatamente 13, E01–E13, com quantidades/unidades/critérios da matriz |
| APs Nexo | Exatamente 21, ordens 22–42, sem integração automática |
| Responsabilidades | Leandro Bochi da Silva Volk e três colaboradores preservados conforme fonte |
| Territórios | Hulha Negra, Candiota e Aceguá |
| Marcos | 13 registros por produto; janelas internas das APs também preservadas em metadados |
| Primeiros 30 dias | 7 prioridades P0/P1, saídas e coordenação |
| Gate de campo | 5 condições inicialmente não verificadas |
| Vínculos AP–produto | 15 propostas por afinidade temática, explicitamente não confirmadas |
| Histórico/painel/piloto | 100 questionários sem contagem automática; 60 e 20 preservados como propostas |
| Execução inicial | Desconhecida; nenhum aceite importado |

O manifesto gera 64 entidades. O texto integral mantém também a matriz de riscos, checkpoints, curva cumulativa proposta, condições de capacidade e governança; essas tabelas complementares não viraram automaticamente tarefas ou execução. Não foram incluídos dados individuais de famílias.

Importação executada duas vezes na mesma base de teste: a segunda não criou entidades duplicadas nem substituiu uma alteração operacional em E06. Hash adulterado e importação com erro tardio foram recusados; a última demonstrou rollback de todas as entidades/eventos da tentativa. Bancos de versão futura são recusados sem downgrade.

## Capacidades verificadas

[Teste de integração](../tests/integration.py), executado por `ctest --preset local`, passou. O teste percorre múltiplos cenários HTTP e consulta SQLite; o resultado está em [ctest.txt](evidence/ctest.txt). Foram exercitados:

- Criação e edição de projeto independente, compromisso, atividade e marco; acontecimento, decisão e evidência; arquivamento e preservação da trajetória.
- Rejeição de aceite sem critério, sem evidência, com evidência inválida, chave repetida ou quantidade acima do pactuado; aceite fundamentado e revogação com atualização do total.
- E06 com 1 aceite de teste, E07 e E08 com nenhum: contagens independentes e sem equivalência implícita entre família, entrevista e georreferência. Isso ocorreu apenas na base temporária.
- Condição não verificada e impedimento visíveis em Agora, retirada após verificação/resolução fundamentada e atribuída; tentativa sem fundamento recusada.
- Revisão concorrente obsoleta recusada; entradas inválidas, data impossível, payload excessivo, Host e origem externos recusados com erro.
- Relação muitos-para-muitos, precedência, recusa de ciclo e relação entre projetos diferentes.
- Migração inicial, triggers de eventos imutáveis, integridade e foreign_key_check; backup SQLite com processo ativo, restauração em diretório novo e reinício recuperando registros.
- Porta ocupada recusada. O teste revelou SO_REUSEPORT no padrão da biblioteca; a configuração foi corrigida para não compartilhar uma porta de outro serviço.

## Demonstração em navegador real

[tests/browser.cjs](../tests/browser.cjs) abriu Chromium real, executou os formulários e reiniciou o processo C++ durante o fluxo. Foram demonstrados projeto independente, compromisso, atividade, acontecimento, evidência referencial, aceite parcial fundamentado, condição e impedimento resolvidos, pesquisa e recuperação após reinício. O navegador não registrou erros JavaScript.

Houve captura e inspeção visual da interface desktop e móvel. Em 390×844 o documento não apresentou transbordamento horizontal; em 1440×1000 foi verificado o painel desktop. Escape e foco por Tab foram exercitados. Essa é uma avaliação focal dos fluxos implementados, não um estudo com usuários ou auditoria completa de acessibilidade/leitores de tela.

Evidências preservadas:

- [Agora COPTIL inaugural](evidence/01-coptil-agora.png)
- [Aceite parcial no projeto independente](evidence/02-aceite-parcial.png)
- [Registro recuperado depois de reiniciar](evidence/03-registro-apos-reinicio.png)
- [Layout móvel](evidence/04-mobile.png)
- [Resultado do navegador e versão efetiva](evidence/browser-result.json)

Playwright/Chromium já existentes no ambiente foram usados somente na verificação. A aplicação não depende deles. Para repetir, configure `PRUMO_PLAYWRIGHT_MODULE` e `PRUMO_CHROMIUM` conforme o comentário no script e execute `node tests/browser.cjs build/prumo`; isso usa base temporária e regenera as capturas.

## Arquivos entregues

- Raiz: CMakeLists, presets, README, CHANGELOG e regras de ignore.
- `src/{domain,storage,application,http}` e `include/prumo`: monólito modular concreto.
- `migrations/001.sql`: versão inicial, índices, relações e proteção do histórico.
- `web/{css,js,icons}` e index: interface e seis ícones originais.
- `seed/manifest.json`, fonte original/hash preservados e `scripts/prepare_seed.py`.
- `scripts/backup.py`, `scripts/restore.py`, `deploy/prumo.service.example` não instalado.
- Testes de integração e navegador, documentação da API, arquitetura, este relatório e capturas/logs reais.
- `third_party/`: HTTP/JSON com licenças e hashes.

## Limitações e próximos passos

1. **Uso local confiável.** Autenticação, autorização por projeto, sessões, criptografia de dados e revisão de segurança precedem qualquer exposição remota. Classificação de acervo é metadado, sem ACL de arquivos. Não foi realizado pentest.
2. **Evidências referenciais.** O sistema guarda origem, referência, classificação e data, mas não ingere nem verifica arquivos externos. Aceite é decisão do revisor declarado. A prevenção de duplicação depende da chave de entrega estável fornecida; não há detecção semântica de um mesmo lote renomeado.
3. **Evolução do modelo.** O envelope JSON validado conserva metadados e mantém o núcleo pequeno. Não há formulários especializados para todos os campos formais, editor gráfico de cronograma, motor de agendamento, agregação por município ou reconciliação de novas versões da fonte. Janelas mensais são texto, preservadas sem dias inventados. Checkpoints e curvas complementares permanecem consultáveis no original.
4. **Escala inicial.** Listas e histórico sem paginação, conexão de banco serializada, busca local por projeto. Antes de volumes maiores, medir uso e adicionar paginação/índices e formulários especializados.
5. **Dados e operação.** Sem coleta individual/offline, sincronização, finanças, bolsas completas, integração automática Nexo, MCP, LLM ou georreferenciamento individual. Proteção de dados pessoais em texto livre depende do operador; não há DLP. Backup não copia documentos externos. Unidade systemd é exemplo e não foi instalada/habilitada.
6. **Próxima pactuação.** Revisar os 15 vínculos sugeridos; verificar os cinco itens do gate com documentos e responsáveis; confirmar situação administrativa, capacidade de equipe e datas internas. Registrar fatos/aceites somente quando sustentados, preservando incerteza e versões.

Não se declara execução do TED, homologação institucional do plano ou conclusão de obrigações. Nenhuma configuração compartilhada, HAProxy ou serviço irmão foi alterado. O sistema está utilizável no escopo demonstrado e preparado para evolução incremental.

Sempre pronto. Sempre incompleto.
