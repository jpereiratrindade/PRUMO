#include "prumo/core.hpp"
#include <set>
#include <regex>
namespace prumo {
namespace {
struct Tx { Store& s;bool done=false; explicit Tx(Store& x):s(x){s.exec("SAVEPOINT operation");}void commit(){s.exec("RELEASE operation");done=true;}~Tx(){if(!done){try{s.exec("ROLLBACK TO operation; RELEASE operation;");}catch(...){}}}};
void require(bool b,const std::string& m,int code=422){if(!b)throw Error(code,m);}
void actor_check(const std::string& a){require(a.find_first_not_of(" \t\r\n")!=std::string::npos&&a.size()<=200,"Identifique o ator local (sem autenticação)");}
json unpack(json r){auto d=json::parse(r["data"].get<std::string>());r.erase("data");r["data"]=d;return r;}
}
json Service::get(const std::string& id){auto rows=store.query("SELECT * FROM entities WHERE id=? AND deleted=0",{id});require(!rows.empty(),"Registro não encontrado",404);return unpack(rows[0]);}
json Service::list(const std::string& p){std::lock_guard lock(store.mutex);auto rows=p.empty()?store.query("SELECT * FROM entities WHERE deleted=0 ORDER BY created_at,id"):store.query("SELECT * FROM entities WHERE deleted=0 AND (project_id=? OR id=?) ORDER BY created_at,id",{p,p});for(auto& r:rows)r=unpack(r);return rows;}
void Service::event(const std::string& p,const std::string& id,const std::string& a,const std::string& t,const json& payload,const std::string& fact){store.run("INSERT INTO events VALUES(?,?,?,?,?,?,?,?)",{uid(),p,id,a,fact.empty()?now():fact,now(),t,payload.dump()});}
json Service::save(json input,const std::string& actor,const std::string& reason,bool importing){
 std::lock_guard lock(store.mutex);actor_check(actor);Tx tx(store);
 auto kind=input.at("kind").get<std::string>();auto d=input.at("data");validate(kind,d);
 if((kind=="condition"&&d.value("state","unverified")!="unverified")||(kind=="impediment"&&d.value("state","")=="resolved"))require(!d.value("resolution","").empty(),"Verificação/resolução exige fundamento");
 std::string id=input.value("id",uid()),project=input.value("project_id","");
 require(std::regex_match(id,std::regex("[A-Za-z0-9_-]{1,128}")),"Identificador inválido");
 auto old=store.query("SELECT * FROM entities WHERE id=?",{id});bool create=old.empty();
 if(kind=="project"){project=id;}else{auto p=get(project);require(p["kind"]=="project","Projeto inválido");}
 if(d.contains("goal_id")&&d["goal_id"]!=""){auto g=get(d["goal_id"]);require(g["kind"]=="goal"&&g["project_id"]==project,"Meta de outro projeto ou inválida");}
 if(!create){auto prev=unpack(old[0]);require(!prev["deleted"].get<int>(),"Registro arquivado",409);require(prev["kind"]==kind&&(kind=="project"||prev["project_id"]==project),"Tipo e projeto são imutáveis");require(input.contains("revision")&&input["revision"]==prev["revision"],"Conflito de edição: recarregue o registro",409);require(!reason.empty(),"Explique o motivo da edição");
 if((kind=="condition"||kind=="impediment")&&prev["data"].value("state","")!=d.value("state","") )require(!d.value("resolution","").empty(),"Mudança de condição/impedimento exige fundamento da verificação");
 if(kind=="commitment"&&d.contains("quantity")&&!d["quantity"].is_null()){auto accepted=store.query("SELECT COALESCE(SUM(quantity),0) q FROM acceptances WHERE commitment_id=? AND revoked_at IS NULL",{id})[0]["q"].get<double>();require(d["quantity"].get<double>()>=accepted,"Quantidade pactuada inferior aos aceites ativos; revise ou revogue antes",409);}
 store.run("UPDATE entities SET data=?,revision=revision+1,updated_at=? WHERE id=?",{d.dump(),now(),id});event(project,id,actor,"edited",{{"before",prev},{"after",d},{"reason",reason}});
 }else{
 if(!d.contains("state"))d["state"]=kind=="condition"?"unverified":"unknown";
 store.run("INSERT INTO entities(id,project_id,kind,data,created_at,updated_at,external_key) VALUES(?,?,?,?,?,?,?)",{id,kind=="project"?json(nullptr):json(project),kind,d.dump(),now(),now(),input.value("external_key",json(nullptr))});
 // Project roots have no parent; events reference their own identifier.
 event(project,id,actor,importing?"imported":"created",{{"after",d}},d.value("fact_at",""));
 }
 auto result=get(id);tx.commit();return result;
}
json Service::remove(const std::string& id,int rev,const std::string& actor,const std::string& reason){
 std::lock_guard lock(store.mutex);actor_check(actor);require(!reason.empty(),"Motivo do arquivamento obrigatório");Tx tx(store);auto e=get(id);require(e["revision"]==rev,"Conflito de edição",409);
 require(store.query("SELECT id FROM acceptances WHERE commitment_id=? AND revoked_at IS NULL",{id}).empty(),"Revogue os aceites antes de arquivar",409);
 require(store.query("SELECT id FROM entities WHERE deleted=0 AND project_id=?",{id}).empty(),"Arquive os registros do projeto primeiro",409);
 store.run("UPDATE entities SET deleted=1,revision=revision+1,updated_at=? WHERE id=?",{now(),id});event(e["kind"]=="project"?id:e["project_id"].get<std::string>(),id,actor,"archived",{{"before",e},{"reason",reason}});tx.commit();return {{"archived",id}};
}
json Service::accept(const std::string& id,const json& b){
 std::lock_guard lock(store.mutex);Tx tx(store);auto c=get(id);auto actor=b.at("reviewer").get<std::string>();actor_check(actor);
 require(c["kind"]=="commitment","Aceite exige compromisso");require(!c["data"].value("criteria","").empty(),"Defina critérios de aceite no compromisso");
 auto rationale=b.at("rationale").get<std::string>(),key=b.at("delivery_key").get<std::string>();require(!rationale.empty()&&!key.empty(),"Fundamento e chave única da entrega obrigatórios");
 require(b.at("quantity").is_number(),"Quantidade numérica obrigatória");auto q=b["quantity"].get<double>();require(q>0,"Quantidade deve ser positiva");
 auto ev=b.at("evidence_ids");require(ev.is_array()&&!ev.empty(),"Aceite exige evidência referenciada");
 for(auto& eid:ev){auto e=get(eid.get<std::string>());require(e["kind"]=="evidence"&&e["project_id"]==c["project_id"]&&!e["data"].value("reference","").empty(),"Evidência inválida, sem referência ou de outro projeto");}
 auto total=store.query("SELECT COALESCE(SUM(quantity),0) total FROM acceptances WHERE commitment_id=? AND revoked_at IS NULL",{id})[0]["total"].get<double>();
 if(c["data"].contains("quantity")&&!c["data"]["quantity"].is_null())require(total+q<=c["data"]["quantity"].get<double>(),"Aceite ultrapassa quantidade pactuada");
 auto aid=uid();store.run("INSERT INTO acceptances(id,project_id,commitment_id,delivery_key,quantity,reviewer,rationale,evidence,accepted_at) VALUES(?,?,?,?,?,?,?,?,?)",{aid,c["project_id"],id,key,q,actor,rationale,ev.dump(),now()});
 event(c["project_id"],id,actor,"accepted",{{"acceptance_id",aid},{"criteria_snapshot",c["data"]["criteria"]},{"review",b}});tx.commit();return {{"id",aid},{"accepted_quantity",total+q}};
}
json Service::revoke(const std::string& id,const json& b){
 std::lock_guard lock(store.mutex);Tx tx(store);actor_check(b.at("actor"));auto reason=b.at("reason").get<std::string>();require(!reason.empty(),"Fundamento da revogação obrigatório");auto rows=store.query("SELECT * FROM acceptances WHERE id=? AND revoked_at IS NULL",{id});require(!rows.empty(),"Aceite ativo não encontrado",404);auto a=rows[0];store.run("UPDATE acceptances SET revoked_at=?,revocation_reason=? WHERE id=?",{now(),reason,id});event(a["project_id"],a["commitment_id"],b["actor"],"acceptance_revoked",{{"acceptance_id",id},{"reason",reason}});tx.commit();return {{"revoked",id}};
}
json Service::link(const json& b){
 std::lock_guard lock(store.mutex);Tx tx(store);actor_check(b.at("actor"));auto s=get(b.at("source_id")),t=get(b.at("target_id"));require(s["project_id"]==t["project_id"]&&!s["project_id"].is_null(),"Relação exige registros do mesmo projeto");
 std::string type=b.at("type"),state=b.value("state","proposed");require(type=="activity_commitment"||type=="precedence","Tipo de relação inválido");require(state=="proposed"||state=="confirmed"||state=="rejected","Estado de relação inválido");
 if(type=="activity_commitment")require(s["kind"]=="activity"&&t["kind"]=="commitment","Vincule atividade a compromisso");
 if(type=="precedence"&&state!="rejected"){
 auto cycles=store.query("WITH RECURSIVE reach(id) AS (SELECT target_id FROM links WHERE source_id=? AND type='precedence' AND state!='rejected' UNION SELECT l.target_id FROM links l JOIN reach r ON l.source_id=r.id WHERE l.type='precedence' AND l.state!='rejected') SELECT id FROM reach WHERE id=?",{t["id"],s["id"]});require(s["id"]!=t["id"]&&cycles.empty(),"Ciclo de precedência recusado",409);
 }
 store.run("INSERT INTO links VALUES(?,?,?,?,?,?) ON CONFLICT(source_id,target_id,type) DO UPDATE SET state=excluded.state",{uid(),s["project_id"],s["id"],t["id"],type,state});event(s["project_id"],s["id"],b["actor"],"link_review",b);tx.commit();return {{"linked",true}};
}
json Service::events(const std::string& project){std::lock_guard lock(store.mutex);auto rows=store.query("SELECT * FROM events WHERE project_id=? ORDER BY recorded_at,rowid",{project});for(auto& r:rows)r["payload"]=json::parse(r["payload"].get<std::string>());return rows;}
json Service::dashboard(const std::string& p){
 std::lock_guard lock(store.mutex);get(p);auto entities=list(p);json items=json::array(),totals=json::object();
 for(auto& e:entities){std::string k=e["kind"],state=e["data"].value("state","unknown"),id=e["id"];std::string reason;
 if(k=="condition"&&state!="satisfied")reason=state=="unverified"?"Condição ainda não verificada":"Condição não satisfeita";
 else if(k=="impediment"&&state!="resolved")reason="Impedimento sem resolução registrada";
 else if(k=="activity"||k=="milestone"||k=="commitment"){
 if(state=="unknown")reason="Progresso real desconhecido; verificar situação";
 else if(state=="draft"||state=="planned"||state=="unverified")reason="Próximo passo pendente de registro de execução";
 if(e["data"].contains("due_internal")&&e["data"]["due_internal"].is_string()&&e["data"]["due_internal"]!=""&&e["data"]["due_internal"].get<std::string>()<now().substr(0,10)&&state!="delivered")reason+="; prazo interno vencido (não é prazo formal)";
 }
 if(k=="commitment"){auto accepted=store.query("SELECT COALESCE(SUM(quantity),0) q,COUNT(*) n FROM acceptances WHERE commitment_id=? AND revoked_at IS NULL",{id})[0];if(accepted["n"].get<int>()>0)reason="Revisar saldo do compromisso: "+accepted["q"].dump()+" com aceite ativo; conclusão não presumida";else if(state=="executed"||state=="delivered")reason="Entrega declarada pendente de revisão e aceite";if(e["data"].contains("quantity")&&e["data"]["quantity"].is_number()&&e["data"]["quantity"].get<double>()>0&&accepted["q"].get<double>()>=e["data"]["quantity"].get<double>())reason.clear();}
 if(!reason.empty())items.push_back({{"entity",e},{"reason",reason}});
 if(k=="commitment"){auto sum=store.query("SELECT COALESCE(SUM(quantity),0) quantity,COUNT(*) reviews FROM acceptances WHERE commitment_id=? AND revoked_at IS NULL",{id})[0];totals[id]=sum;}
 }
 return {{"items",items},{"acceptance_totals",totals},{"links",store.query("SELECT * FROM links WHERE project_id=?",{p})},{"acceptances",store.query("SELECT * FROM acceptances WHERE project_id=? ORDER BY accepted_at",{p})}};
}
json Service::import_seed(const std::string& path){
 std::lock_guard lock(store.mutex);auto seed=json::parse(read(path));std::string source=seed.at("source"),digest=hash(read(std::string(PRUMO_ROOT)+"/seed/sources/"+source));require(digest==seed.at("sha256").get<std::string>(),"Fonte diverge do hash da importação; revisão de versão obrigatória",409);Tx tx(store);
 auto previous=store.query("SELECT * FROM imports WHERE source=?",{source});if(!previous.empty()){require(previous[0]["hash"]==digest,"Conflito de versão da fonte",409);tx.commit();return {{"imported",false},{"reason","Fonte já importada; alterações operacionais preservadas"}};}
 for(auto e:seed.at("entities"))save(e,"Importador local • fonte v0.1.0","Carga inaugural",true);
 for(auto l:seed.at("links"))link(l);
 store.run("INSERT INTO imports VALUES(?,?,?)",{source,digest,now()});tx.commit();return {{"imported",true},{"entities",seed["entities"].size()},{"sha256",digest}};
}
}
