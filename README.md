# PRUMO

Sempre pronto. Sempre incompleto.

Sistema local de orientação e acompanhamento de compromissos. Backend C++26, SQLite e interface web nativa. A primeira base é o plano TED MAPA–Embrapa/COPTIL; outros projetos podem ser criados sem dependência desse domínio.

## Construir e executar

Requer compilador com suporte ao modo C++26, CMake ≥3.25, SQLite3, OpenSSL Crypto, threads e Python3 para verificações. HTTP e JSON estão em `third_party/`, com licenças preservadas. Sem downloads durante o build, npm, CDN ou serviços externos.

```bash
cmake --preset local
cmake --build --preset local -j4
ctest --preset local
./build/prumo --import
./build/prumo --port 8786
```

Abra **http://127.0.0.1:8786**. A porta deve estar livre; o processo recusa compartilhamento de porta. O bind é exclusivamente local. Dados ficam em `${XDG_DATA_HOME:-$HOME/.local/share}/prumo/prumo.sqlite3`. Pode usar `--data /diretorio/fora/do/repositorio`; o programa recusa dados dentro do checkout. `--import` importa a fonte e encerra; não é necessário importar para usar projetos independentes. Reimportar preserva a operação existente. `--help` mostra opções.

O build verificado usou GCC 16.2.1, CMake 4.3.0, SQLite 3.51.2, OpenSSL 3.5.9 e **`-std=c++26`**. C++26 ainda é um padrão em desenvolvimento; a flag aceita pelo compilador não significa implementação integral de todos os recursos futuros. Nenhum rebaixamento de padrão é permitido pelo CMake.

## Trabalhar

1. Informe seu ator local no topo. É uma identificação declarada, sem autenticação.
2. Selecione um projeto ou use **+ Projeto**. Basta um título para iniciar um rascunho.
3. Em **Metas**, use **+ Registrar** e escolha meta, compromisso, atividade ou marco. Edite critérios, quantidades, responsáveis, referências e próximos passos conforme forem conhecidos.
4. Em **Campo**, acompanhe contexto territorial e condições de preparação. Uma condição satisfeita ou um impedimento resolvido exige fundamento. Não registre dados pessoais de famílias, contatos ou coordenadas.
5. Em **Registro**, registre acontecimentos e decisões. A trajetória mostra ator, data do fato, data de registro e valores anteriores. Edições e arquivamentos exigem motivo e revisão atual; conflitos recusam sobrescrita.
6. Em **Acervo**, registre uma referência de evidência e sua origem/classificação. O documento original pode ser consultado localmente. A referência não faz upload, não verifica o conteúdo e não altera aceite.
7. Use **Revisar entrega** no compromisso: critérios definidos, chave única do lote/dossiê, quantidade positiva, evidência do mesmo projeto e fundamento são necessários. A revisão vale para aquele compromisso. E06, E07 e E08 são contados separadamente. Revogações ficam no histórico. Uma chave revogada não pode ser reutilizada; uma nova revisão usa chave versionada.
8. **Agora** deriva dos registros: mostra condições não verificadas/não satisfeitas, impedimentos, próximos passos, prazos internos vencidos e entregas pendentes de revisão. Cada item informa sua razão. Sem aceite registrado não significa zero de execução confirmado. Aceite completo remove o saldo pendente do compromisso.

A pesquisa filtra o projeto/visão atual. O fuso de apresentação pode ser São Paulo ou UTC; persistência temporal é UTC. Escape fecha o diálogo e os controles usam navegação nativa por teclado. Arquivamento preserva dados e eventos; não há exclusão física pelo aplicativo.

## Dados inaugurais

`seed/sources/` preserva o original e SHA256SUMS. `seed/manifest.json` é uma derivação revisável, gerada por `python3 scripts/prepare_seed.py`; gerar manifesto não importa dados operacionais. A fonte é conferida por SHA-256 no backend. Há 3 metas, 13 tipos E01–E13, 21 APs Nexo, 13 marcos por produto, 7 prioridades dos primeiros 30 dias, 5 condições de gate não verificadas e a fonte como evidência de planejamento. As janelas internas das APs são preservadas em seus metadados, sem convertê-las em datas formais.

Vigência 30/07/2026–30/03/2028, valor R$ 334.250,00; Leandro e colaboradores são preservados conforme o original. Os 100 questionários históricos não entram na contagem, e piloto de 20/painel de 60 continuam propostas. As 15 sugestões de vínculo AP–produto aguardam revisão. A fonte integral também conserva riscos, checkpoints e hipóteses de capacidade, que não são execução comprovada.

## Backup e restauração

Use a API de backup SQLite, inclusive com WAL e serviço ativo; não copie só o arquivo `.sqlite3` com o processo em execução.

```bash
python3 scripts/backup.py "$HOME/.local/share/prumo/prumo.sqlite3" /caminho/backup/prumo-2026-11-24.sqlite3
python3 scripts/restore.py /caminho/backup/prumo-2026-11-24.sqlite3 /caminho/novo-diretorio-prumo
./build/prumo --data /caminho/novo-diretorio-prumo --port 8787
```

O destino do backup deve ser novo. A restauração exige diretório novo e confere versão, integridade e chaves estrangeiras. Preserve o banco anterior; encerre sua instância pelo terminal antes de mudar caminhos ou unidades. Não sobrescreva arquivos WAL/SHM. Um backup contém o banco, inclusive eventos, não os documentos externos referenciados. Preserve esses dossiês separadamente, além do checkout/fonte original.

## Operação e evolução

`deploy/prumo.service.example` é uma unidade de usuário para instalação manual futura. Ajuste caminhos, construa o binário e crie o diretório de dados antes de instalar; nada foi habilitado por este patch. Não há alterações em HAProxy, portas de irmãos ou configurações compartilhadas.

Somente uso local de operador confiável nesta versão. Não exponha via proxy ou túnel: autenticação, autorização por projeto e revisão de segurança são pré-requisitos para acesso remoto/multiusuário. Verificações de Host/Origin, cabeçalho de escrita, limites de payload e CSP já protegem a interface local, mas não constituem autenticação. Classificação de evidência é metadado, não controle de acesso a arquivos.

Detalhes: [API](docs/API.md), [decisões e inspeção](docs/ARCHITECTURE.md), [relatório e evidências](docs/IMPLEMENTATION-REPORT.md), [instrução fundadora](docs/PRUMO-PATCH-001-v0.1.0.md).
