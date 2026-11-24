# Decisões de implementação e inspeção — 24/11/2026

## Inspeção anterior à implementação

Workspace: `/run/media/jpereiratrindade/labeco10T/dev/cpp/prumo`. O repositório já existia, sem commits, todos os arquivos documentais iniciais não rastreados. Foram preservados a instrução fundadora e o documento original. Nenhum AGENTS.md aplicável foi encontrado nos ancestrais consultados. Não houve reset, clean, alteração de irmão ou cópia de configurações/segredos.

O diretório pai contém sistemas C++ e sistemas heterogêneos. Foram examinados os manifests e recursos abaixo, em leitura:

| Referência real | Evidência e decisão |
|---|---|
| `../MORFO/CMakeLists.txt` | Projeto `morfo` 0.2.0, C++26 obrigatório, extensões desabilitadas, SQLite, OpenSSL, Threads, warnings e CTest. Usar o mesmo padrão de build, sem dependências de imagem. |
| `../MORFO/web/style.css` | Tokens `--dark:#071e38`, `--navy:#0c2844`, `--ink:#18334b`, `--muted:#64788a`, `--line:#dce7ef`, `--blue:#1a7dc4`, `--sky:#2ea8e8`, `--teal:#087f8c`, `--green:#14805e`, `--bg:#edf2f8`, `--surface:#fff`. Reproduzir família cromática, superfícies, raios, foco dourado e barra lateral. |
| `../MORFO/web/index.html` | Barra lateral, cabeçalho, hero, cartões; SVG linear com pontas e junções arredondadas; ícone acompanha texto. PRUMO cria seis SVGs originais com essa gramática. |
| `../MORFO/docs/sister-principles/README.md` | Princípios de desenvolvimento não são indicadores de disponibilidade. PRUMO separa o lema do status de conexão. |
| `../MORFO/.git` | Repositório real MORFO, origin `github.com:jpereiratrindade/MORFO.git`; não é apenas uma pasta visual de referência. |
| `../MORFO/src/main.cpp` | HTTP via Boost.Asio; JSON próprio. |
| `../NOESE/CMakeLists.txt`, `../NOESE/src/Server.cpp`, `../NOESE/third_party/` | C++26 sem fallback, cpp-httplib 0.28.0 e nlohmann/json 3.12.0. Adotadas essas bibliotecas já disponíveis, preservando licenças e hashes em `third_party/`. |
| `../METIS/CMakeLists.txt` | CMake 3.30, núcleo C++26 e CTest; confirma convenções locais de núcleo compilado e verificação. |

A pilha de fontes do MORFO começa com Plus Jakarta Sans/Inter e termina em system-ui. PRUMO usa a parte nativa system-ui/Segoe UI: sem CDN ou fonte baixada. Não se afirma identidade tipográfica pixel a pixel. MORFO usa base clara e navegação escura; não se inventou seletor de temas ausente nesta entrega. O símbolo PRUMO usa uma referência horizontal e um prumo suspenso, em contraste semântico próprio dentro da gramática observada.

## Módulos e persistência

- `domain/model.cpp`: tipos permitidos, estados, limites, datas, IDs/UTC e hash.
- `storage/store.cpp`: conexão SQLite, prepared statements, foreign_keys, WAL, busy_timeout e migração inicial transacional.
- `application/service.cpp`: comandos, validação contextual por projeto, revisões, contagens, relações, importação e derivação de Agora.
- `http/main.cpp`: adaptador HTTP local, rotas v1, validação de origem/Host, recursos estáticos, erros e configuração.
- `include/prumo/core.hpp`: contratos concretos desses módulos.
- `web/`: produto nativo sem build JS.

`entities` guarda envelope estável com ID aleatório de 128 bits (ou chave estável da fonte), projeto, tipo, revisão, criação/atualização UTC e exclusão lógica. O documento JSON validado guarda atributos específicos, permitindo preservar metadados desconhecidos pela forma visual. `links`, `acceptances`, `events` e `imports` têm tabelas próprias e referências SQL. Metas são entidades de agrupamento; compromissos têm quantidade/unidade/critérios; APs são atividades com referência Nexo e não uma integração ativa.

Escritas usam savepoints transacionais, inclusive para uma importação atômica com múltiplos comandos. A conexão é serializada por mutex; o banco usa WAL e espera limitada para contenção entre processos. Todas as entradas operacionais usam prepared statements. A migração 001 é uma transação e só roda em user_version 0; versões futuras são recusadas. Não há downgrade automático.

Uma edição exige revisão corrente e motivo, preservando o envelope e payload anterior no evento. Eventos são append-only na aplicação e protegidos por triggers de UPDATE/DELETE; acesso administrativo direto ao arquivo continua fora dessa garantia. Arquivamento preserva a trajetória. Um projeto com filhos ativos não pode ser arquivado. Compromisso com aceite ativo exige revogação antes de arquivar ou reduzir a quantidade abaixo do aceito.

## Aceites e dependências

Aceite é um comando dedicado: requer compromisso, critério não vazio, revisor declarado, fundamento, quantidade positiva, chave de entrega e evidência referenciada no mesmo projeto. Valida tudo na mesma transação, limita o total pactuado quando conhecido e mantém unicidade `(commitment_id, delivery_key)` inclusive após revogação. Reavaliação usa nova chave versionada. Revogar registra fundamento, ator e evento na mesma transação.

Contagens são somas de aceites ativos por compromisso, nunca de visitas ou evidências. E06/E07/E08 são independentes. O evento de aceite preserva o critério vigente. Estado declarado de execução/entrega e soma aceita são dimensões distintas; não há aceite por visita de página ou anexação de referência.

Relação atividade–compromisso é muitos-para-muitos. Vínculos da importação são propostas, confirmáveis ou rejeitáveis com ator/evento. Dependências de precedência usam busca recursiva antes da inserção; são recusadas relações reflexivas, ciclos e referências entre projetos. Condições de gate pertencem ao projeto e exigem fundamento na verificação; não liberam execução automaticamente. Não há motor de agendamento.

## Importação e versões

A cópia original e SHA256SUMS são preservados. O script de derivação gera manifesto determinístico; o backend confere o hash do original antes da carga. O ledger de imports identifica fonte/hash e impede duplicação ou sobrescrita operacional. Mudança de hash é conflito para revisão, não atualização silenciosa. Uma futura versão precisa de procedimento explícito de reconciliação; renomear uma fonte não é mecanismo de atualização suportado. Marcos internos textuais são preservados como janelas propostas; não se inventam dias exatos para meses/janelas da fonte.

## Limites de segurança e operação

Bind fixo 127.0.0.1, porta 8786 configurável, sem SO_REUSEPORT. Validação de Host para o endpoint escolhido, Origin quando presente, recusa de Sec-Fetch-Site cross-site e cabeçalho obrigatório de escrita impedem submissão simples de outras origens no navegador. Sem CORS. CSP com recursos locais, sem código inline; limite de payload 128 KiB; JSON inválido e conflitos têm erros apropriados. Uso direto da API por processo local autorizado continua possível.

Não há autenticação, autorização por projeto, criptografia do banco, verificação de conteúdo dos documentos ou DLP de texto livre. O diretório de dados recebe permissões 0700; exemplo de serviço usa UMask 0077. Quem controla a conta local/arquivo controla todos os projetos. Classificações de acervo são metadados. Não habilitar publicação remota sem tratar essas lacunas. O push autorizado do código não instala nem expõe o serviço.
