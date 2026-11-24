#include "prumo/core.hpp"
#include <filesystem>
namespace prumo {
Store::Store(const std::string& path){
 std::filesystem::create_directories(std::filesystem::absolute(path).parent_path());
 if(sqlite3_open(path.c_str(),&db)!=SQLITE_OK) throw Error(500,"Não foi possível abrir SQLite");
 sqlite3_busy_timeout(db,5000);
 exec("PRAGMA foreign_keys=ON; PRAGMA journal_mode=WAL;");
 auto v=query("PRAGMA user_version")[0]["user_version"].get<int>();
 if(v>1) throw Error(500,"Banco de versão futura: migração recusada");
 if(v<1) exec(read(std::string(PRUMO_ROOT)+"/migrations/001.sql"));
}
Store::~Store(){if(db)sqlite3_close(db);}
void Store::exec(const std::string& sql){char* err=nullptr; if(sqlite3_exec(db,sql.c_str(),nullptr,nullptr,&err)!=SQLITE_OK){std::string msg=err?err:"SQLite";sqlite3_free(err);throw Error(500,msg);}}
json Store::query(const std::string& sql,const json& args){
 sqlite3_stmt* raw=nullptr;
 if(sqlite3_prepare_v2(db,sql.c_str(),-1,&raw,nullptr)!=SQLITE_OK)throw Error(500,sqlite3_errmsg(db));
 std::unique_ptr<sqlite3_stmt,decltype(&sqlite3_finalize)> stmt(raw,sqlite3_finalize);
 int i=1;for(const auto& a:args){
 if(a.is_null()) sqlite3_bind_null(raw,i);
 else if(a.is_number_integer())sqlite3_bind_int64(raw,i,a.get<int64_t>());
 else if(a.is_number())sqlite3_bind_double(raw,i,a.get<double>());
 else if(a.is_boolean())sqlite3_bind_int(raw,i,a.get<bool>());
 else {auto s=a.is_string()?a.get<std::string>():a.dump();sqlite3_bind_text(raw,i,s.c_str(),-1,SQLITE_TRANSIENT);}++i;
 }
 json out=json::array();int rc;
 while((rc=sqlite3_step(raw))==SQLITE_ROW){json row=json::object();for(int c=0;c<sqlite3_column_count(raw);++c){auto name=sqlite3_column_name(raw,c);switch(sqlite3_column_type(raw,c)){case SQLITE_INTEGER:row[name]=sqlite3_column_int64(raw,c);break;case SQLITE_FLOAT:row[name]=sqlite3_column_double(raw,c);break;case SQLITE_NULL:row[name]=nullptr;break;default:row[name]=reinterpret_cast<const char*>(sqlite3_column_text(raw,c));}}out.push_back(row);}
 if(rc!=SQLITE_DONE)throw Error(sqlite3_extended_errcode(db)==SQLITE_CONSTRAINT_UNIQUE?409:422,sqlite3_errmsg(db));
 return out;
}
void Store::run(const std::string& sql,const json& args){query(sql,args);}
}
