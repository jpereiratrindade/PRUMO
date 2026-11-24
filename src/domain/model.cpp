#include "prumo/core.hpp"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <regex>
#include <set>
namespace prumo {
std::string now(){auto t=std::time(nullptr);std::tm tm{};gmtime_r(&t,&tm);std::ostringstream s;s<<std::put_time(&tm,"%Y-%m-%dT%H:%M:%SZ");return s.str();}
std::string uid(){unsigned char b[16];if(RAND_bytes(b,16)!=1)throw Error(500,"Falha de identificador");std::ostringstream s;for(auto c:b)s<<std::hex<<std::setw(2)<<std::setfill('0')<<int(c);return s.str();}
std::string hash(const std::string& s){unsigned char b[SHA256_DIGEST_LENGTH];SHA256(reinterpret_cast<const unsigned char*>(s.data()),s.size(),b);std::ostringstream o;for(auto c:b)o<<std::hex<<std::setw(2)<<std::setfill('0')<<int(c);return o.str();}
std::string read(const std::string& p){std::ifstream f(p);if(!f)throw Error(500,"Arquivo não encontrado: "+p);return {std::istreambuf_iterator<char>(f),{}};}
void validate(const std::string& kind,const json& d){
 static const std::set<std::string> kinds={"project","goal","commitment","activity","milestone","condition","occurrence","evidence","decision","impediment"};
 if(!kinds.contains(kind))throw Error(422,"Tipo desconhecido");
 if(!d.is_object()||!d.contains("title")||!d["title"].is_string()||d["title"].get<std::string>().find_first_not_of(" \n\t\r")==std::string::npos)throw Error(422,"Título obrigatório; demais campos podem ficar em rascunho");
 for(auto it=d.begin();it!=d.end();++it){if(it.value().is_string()&&it.value().get_ref<const std::string&>().size()>16000)throw Error(422,"Campo muito longo");}
 for(auto key:{"title","description","state","reason","criteria","unit","reference","origin","access","fact_at","due_internal","due_formal","responsible","source","source_version","territory","goal_id","external_ref","priority","next_step","formal_start","formal_end","collaborators","resolution","internal_window","expected_evidence"})if(d.contains(key)&&!d[key].is_string())throw Error(422,std::string("Campo deve ser texto: ")+key);
 if(d.contains("quantity")&&!d["quantity"].is_null()&&(!d["quantity"].is_number()||d["quantity"].get<double>()<0))throw Error(422,"Quantidade inválida");
 const std::set<std::string> states={"unknown","unverified","draft","planned","executed","delivered","open","resolved","satisfied","unsatisfied"};
 if(d.contains("state")&&!states.contains(d["state"].get<std::string>()))throw Error(422,"Estado inválido; aceite usa revisão explícita");
 if(kind=="condition"&&d.contains("state")&&!std::set<std::string>{"unverified","satisfied","unsatisfied"}.contains(d["state"].get<std::string>()))throw Error(422,"Estado de condição inválido");
 for(auto key:{"due_internal","due_formal","formal_start","formal_end","fact_at"}){if(d.contains(key)&&d[key].is_string()&&d[key].get<std::string>().size()>=10){auto v=d[key].get<std::string>();try{auto y=std::stoi(v.substr(0,4)),m=std::stoi(v.substr(5,2)),day=std::stoi(v.substr(8,2));if(y<1900||!std::chrono::year_month_day{std::chrono::year{y},std::chrono::month{static_cast<unsigned>(m)},std::chrono::day{static_cast<unsigned>(day)}}.ok())throw Error(422,"Data de calendário inválida");}catch(const std::invalid_argument&){throw Error(422,"Data inválida");}}}
 for(auto key:{"due_internal","due_formal","formal_start","formal_end"})if(d.contains(key)&&d[key]!=""&&!std::regex_match(d[key].get<std::string>(),std::regex("[0-9]{4}-[0-9]{2}-[0-9]{2}")))throw Error(422,"Prazo deve usar AAAA-MM-DD");
 if(d.contains("fact_at")&&d["fact_at"]!=""&&!std::regex_match(d["fact_at"].get<std::string>(),std::regex("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z")))throw Error(422,"Data do fato deve ser UTC: AAAA-MM-DDTHH:MM:SSZ");
 if(kind=="evidence"&&d.contains("access")&&!std::set<std::string>{"public","internal","restricted"}.contains(d["access"].get<std::string>()))throw Error(422,"Classificação de acesso inválida");
 for(auto key:{"contacts","coordinates","families","latitude","longitude","cpf"})if(d.contains(key))throw Error(422,"Dados individuais fora do escopo desta versão");
}
}
