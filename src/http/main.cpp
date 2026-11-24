#include "prumo/core.hpp"
#include <httplib.h>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <csignal>
using namespace prumo;
namespace {
void send(httplib::Response& r,const json& j,int status=200){r.status=status;r.set_content(j.dump(),"application/json; charset=utf-8");}
json body(const httplib::Request& r){if(r.get_header_value("Content-Type").find("application/json")!=0)throw Error(415,"Envie application/json");auto b=json::parse(r.body);if(!b.is_object())throw Error(400,"Objeto JSON obrigatório");return b;}
}
int main(int argc,char** argv){try{
 int port=8786;std::string data,seed;bool importing=false;
 auto xdg=std::getenv("XDG_DATA_HOME"),home=std::getenv("HOME");data=xdg?std::string(xdg)+"/prumo":std::string(home?home:"/tmp")+"/.local/share/prumo";
 for(int i=1;i<argc;i++){std::string a=argv[i];if(a=="--port"&&i+1<argc)port=std::stoi(argv[++i]);else if(a=="--data"&&i+1<argc)data=argv[++i];else if(a=="--import"){importing=true;}else if(a=="--seed"&&i+1<argc)seed=argv[++i];else if(a=="--help"){std::cout<<"prumo [--data DIR] [--port 8786] [--import] [--seed MANIFEST]\nBind exclusivo 127.0.0.1; --import importa e encerra.\n";return 0;}else throw Error(400,"Opção inválida: "+a);}
 if(port<1024||port>65535)throw Error(400,"Porta deve estar entre 1024 e 65535");
 auto dir=std::filesystem::absolute(data).lexically_normal();auto root=std::filesystem::canonical(PRUMO_ROOT);
 if(dir==root||dir.string().starts_with(root.string()+"/"))throw Error(400,"Dados operacionais devem ficar fora do repositório");
 std::filesystem::create_directories(dir);std::filesystem::permissions(dir,std::filesystem::perms::owner_all,std::filesystem::perm_options::replace);
 Service svc((dir/"prumo.sqlite3").string());
 if(importing){std::cout<<svc.import_seed(seed.empty()?std::string(PRUMO_ROOT)+"/seed/manifest.json":seed).dump(2)<<"\n";return 0;}
 httplib::Server server;server.set_socket_options([](socket_t sock){int yes=1;setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));});server.new_task_queue=[] {return new httplib::ThreadPool(4);};server.set_payload_max_length(128*1024);server.set_read_timeout(5,0);server.set_write_timeout(5,0);
 std::string host="127.0.0.1:"+std::to_string(port),origin="http://"+host;
 server.set_pre_routing_handler([&](const httplib::Request& req,httplib::Response& res){
 res.set_header("Content-Security-Policy","default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self'; object-src 'none'; base-uri 'none'; frame-ancestors 'none'");res.set_header("X-Content-Type-Options","nosniff");res.set_header("Referrer-Policy","no-referrer");res.set_header("Cache-Control","no-store");
 if(req.get_header_value("Host")!=host){send(res,{{"error","Host não autorizado; use "+origin}},403);return httplib::Server::HandlerResponse::Handled;}
 if(req.method!="GET"&&req.method!="HEAD"){
 auto o=req.get_header_value("Origin");if((!o.empty()&&o!=origin)||req.get_header_value("X-Prumo-Local")!="1"||req.get_header_value("Sec-Fetch-Site")=="cross-site"){send(res,{{"error","Escrita local exige origem válida e X-Prumo-Local: 1"}},403);return httplib::Server::HandlerResponse::Handled;}}
 return httplib::Server::HandlerResponse::Unhandled;});
 auto guard=[&](auto fn){return [&,fn](const httplib::Request& req,httplib::Response& res){try{std::lock_guard lock(svc.store.mutex);fn(req,res);}catch(const Error& e){send(res,{{"error",e.what()}},e.status);}catch(const json::exception&){send(res,{{"error","JSON inválido ou campo obrigatório ausente/tipo incorreto"}},400);}catch(const std::exception& e){std::cerr<<"Falha interna: "<<e.what()<<'\n';send(res,{{"error","Falha interna; operação não concluída"}},500);}};};
 server.Get("/health",guard([&](const auto&,auto& r){send(r,{{"status","ok"},{"version","0.1.0"},{"schema",1},{"identity","ator local declarado, sem autenticação"}});}));
 server.Get("/api/v1/entities",guard([&](const auto& q,auto& r){send(r,svc.list(q.has_param("project")?q.get_param_value("project"):""));}));
 server.Get(R"(/api/v1/entities/([A-Za-z0-9_-]+))",guard([&](const auto& q,auto& r){send(r,svc.get(q.matches[1]));}));
 server.Post("/api/v1/entities",guard([&](const auto& q,auto& r){auto b=body(q);send(r,svc.save(b,b.at("actor"),b.value("reason","")),201);}));
 server.Put(R"(/api/v1/entities/([A-Za-z0-9_-]+))",guard([&](const auto& q,auto& r){auto b=body(q);b["id"]=q.matches[1].str();send(r,svc.save(b,b.at("actor"),b.at("reason")));}));
 server.Delete(R"(/api/v1/entities/([A-Za-z0-9_-]+))",guard([&](const auto& q,auto& r){auto b=body(q);send(r,svc.remove(q.matches[1],b.at("revision"),b.at("actor"),b.at("reason")));}));
 server.Get("/api/v1/events",guard([&](const auto& q,auto& r){send(r,svc.events(q.get_param_value("project")));}));
 server.Get("/api/v1/now",guard([&](const auto& q,auto& r){send(r,svc.dashboard(q.get_param_value("project")));}));
 server.Post(R"(/api/v1/commitments/([A-Za-z0-9_-]+)/accept)",guard([&](const auto& q,auto& r){send(r,svc.accept(q.matches[1],body(q)),201);}));
 server.Post(R"(/api/v1/acceptances/([A-Za-z0-9_-]+)/revoke)",guard([&](const auto& q,auto& r){send(r,svc.revoke(q.matches[1],body(q)));}));
 server.Post("/api/v1/links",guard([&](const auto& q,auto& r){send(r,svc.link(body(q)),201);}));
 server.Post("/api/v1/import",guard([&](const auto& q,auto& r){body(q);send(r,svc.import_seed(std::string(PRUMO_ROOT)+"/seed/manifest.json"));}));
 server.Get("/api/v1/source",guard([&](const auto&,auto& r){auto seeddata=json::parse(read(std::string(PRUMO_ROOT)+"/seed/manifest.json"));r.set_content(read(std::string(PRUMO_ROOT)+"/seed/sources/"+seeddata["source"].template get<std::string>()),"text/plain; charset=utf-8");}));
 server.set_mount_point("/",std::string(PRUMO_ROOT)+"/web");
 server.set_error_handler([](const auto&,auto& r){if(r.body.empty())send(r,{{"error",r.status==413?"Limite de 128 KiB excedido":"Rota ou recurso indisponível"}},r.status);});
 if(!server.bind_to_port("127.0.0.1",port))throw Error(409,"Porta indisponível; nenhum serviço existente foi alterado");
 std::cout<<"PRUMO "<<origin<<" | dados: "<<dir<<std::endl;server.listen_after_bind();
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
