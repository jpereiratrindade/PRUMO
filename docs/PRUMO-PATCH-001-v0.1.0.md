---
identificador: PRUMO-PATCH-001
versao: 0.1.0
data: 2026-11-24
natureza: instrucao de implementacao fundadora
destino: /run/media/jpereiratrindade/labeco10T/dev/cpp/prumo
estado: proposta para execucao local
principio: "Sempre pronto. Sempre incompleto."
---
# PRUMO — criação do sistema

## 1. Missão
Criar um sistema operacional de orientação e acompanhamento de compromissos, começando pelo TED MAPA–Embrapa/COPTIL, sem acoplar o domínio ao TED. Não substituir NEXO nem converter planejamento em execução presumida.

## 2. Instrução ao agente de implementação
Antes de escrever, inspecionar o diretório pai, o MORFO (identificar o repositório real, tokens CSS, layout, tipografia, temas e convenções SVG) e o padrão de build dos projetos C++ existentes. Registrar caminhos, versões e decisões. Não alterar nenhum repositório irmão. Se prumo existir, preservar conteúdo, verificar git status e propor alterações incrementais; não executar reset, clean ou sobrescrita. Não copiar segredos. Se MORFO não estiver acessível, usar estilo provisório explicitamente identificado; não declarar fidelidade visual. Criar ícones SVG originais, acessíveis e consistentes com a gramática visual constatada.

## 3. Escopo funcional inicial
A primeira entrega deve permitir trabalhar, não apenas navegar em mockups. Implementar CRUD de projetos, compromissos, atividades e marcos; registro de acontecimentos, evidências referenciais, decisões e impedimentos; critérios de aceite e revisão explícita de entregas. Uma edição deve preservar a trajetória. Não exigir preenchimento integral para registrar um rascunho. Diferenciar desconhecido, não verificado, planejado, executado, entregue e aceito. Nenhuma mudança de estado de aceite por simples upload ou visita.

Interface: Agora (pendências, próximos passos, impedimentos e condições não verificadas), Metas (compromissos e atividades), Campo (contexto territorial e preparação; coleta individual fica para evolução), Registro (acontecimentos e decisões), Acervo (fontes e evidências). Seleção de projeto, pesquisa, criação e edição reais, estados vazios honestos, feedback de erro, navegação por teclado e layout responsivo. Sem gráficos fictícios ou percentuais inventados. A visão Agora deve derivar dos dados persistidos, com razão visível para cada item.

## 4. Arquitetura
C++26, CMake, monólito modular; verificar suporte efetivo do compilador local e registrar flag usada (não rebaixar silenciosamente). Preferir biblioteca HTTP e JSON já adotada nos projetos irmãos, mediante inspeção; SQLite com foreign_keys, WAL, migrations transacionais, prepared statements, transações para escrita e trilha de eventos. HTML/CSS/JS nativos, SVG próprios; sem CDN, npm obrigatório, LLM ou microsserviços. Servir recursos locais; bind 127.0.0.1 por padrão, porta configurável e verificada antes de uso. API JSON versionada /api/v1, /health, códigos de erro adequados, validação de entrada e limites de payload. Não habilitar acesso remoto antes de autenticação, autorização por projeto, proteção CSRF quando aplicável e revisão de segurança. Não registrar contatos, coordenadas individuais ou dados pessoais de famílias nesta versão.

Estrutura sugerida: CMakeLists.txt, CMakePresets.json, src/{domain,application,storage,http}, include/prumo, web/{css,js,icons}, migrations, seed, tests, docs, deploy, scripts, README.md, CHANGELOG.md, .gitignore. Manter dados operacionais fora do repositório. Criar unidade systemd de exemplo sem habilitar automaticamente; documentar instalação, backup consistente SQLite e restauração. Não modificar HAProxy ou serviços existentes neste patch.

## 5. Modelo mínimo
Identificadores estáveis; timestamps UTC; apresentação local configurável; referência de origem e versão. Project; Commitment (quantidade, unidade, fonte, critério de aceite); Activity (referência externa Nexo opcional); relação activity_commitment muitos-para-muitos; Milestone (prazo interno distinto do formal); Condition/Dependency (estado não verificado, satisfeito, não satisfeito); Event (ator, data do fato e do registro, tipo, entidade, payload); Evidence (referência, origem, classificação de acesso, data); Decision; Impediment; Acceptance (revisor, fundamento, quantidade, evidências, data). Atores inicialmente locais explicitamente identificados, sem alegar identidade autenticada se não houver autenticação. Eventos append-only na aplicação; correções por novo evento. Contagem de aceites não duplicada, revogação registrada e transacional. Não permitir ciclos nas dependências de precedência. Separar registro de estado e evidência de execução.

## 6. Dados inaugurais
Usar o arquivo original `PLANO-INTEGRADO-EXECUCAO-TED-MAPA-COPTIL-2026-001-v0.1.0.md` entregue junto deste patch. Preservar cópia e hash, importar idempotentemente com chaves externas estáveis. Projeto 30879320260001-006183, vigência 30/07/2026 a 30/03/2028, R$ 334.250,00, três metas, 13 tipos de produto E01–E13, 21 APs Nexo ordens 22–42, municípios Hulha Negra, Candiota e Aceguá. Importar descrições, quantidades, unidades, critérios de aceite, marcos internos e prioridades dos primeiros 30 dias. Não inventar relações AP–produto onde a fonte não as explicita: registrar propostas de vinculação para revisão. Preservar responsabilidade formal de Leandro e colaboradores conforme fonte. Os 100 questionários históricos não contam como entrega; painel de 60 e piloto de 20 são propostas. Importar cinco condições do gate de campo como não verificadas. Progresso real desconhecido inicialmente, não zero confirmado. Não importar dados familiares. Reimportação não deve sobrescrever alterações operacionais nem criar duplicatas; conflito de versão exige revisão.

## 7. Critérios de demonstração e aceite do patch
- Build local e execução reproduzíveis; registrar compilador, flags, dependências e comandos reais.
- Abrir interface no navegador; criar projeto independente, compromisso e atividade, registrar acontecimento e evidência; reiniciar serviço e recuperar dados.
- Demonstrar projeto COPTIL com exatamente 3 metas, 13 tipos de entrega e 21 APs, sem execução fabricada; importar duas vezes e verificar ausência de duplicatas.
- Demonstrar que aceite exige critérios e evidência e que contagens de família, entrevista e georreferência são independentes.
- Demonstrar impedimento e condição não verificada na tela Agora; resolver com registro atribuído e verificar atualização.
- Exercitar validação, persistência, migrations, backup/restauração, conflitos e tratamento de erros. Testes automatizados focalizados nas invariantes; complementar com demonstração real no navegador, não usar testes como substituto da avaliação de usabilidade.
- Entregar relatório com arquivos criados, decisões, limitações, evidências reais e próximos passos. Se algo não foi demonstrado, dizer expressamente. Não declarar pronto o que só existe em documentação.

## 8. Limites e evolução
Não implementar coleta offline, sincronização, folha financeira, integração automática Nexo, MCP, LLM, georreferenciamento individual ou gestão completa de bolsas neste patch. Preparar interfaces sem abstrações vazias. Não enviar dados externos. A versão deve ser útil localmente e permanecer extensível. Publicação remota e uso multiusuário dependem de autorização e segurança verificadas. Registrar alterações de escopo no CHANGELOG. Não criar commit remoto nem publicar sem solicitação.
