#pragma once
#include <json.hpp>
#include <sqlite3.h>
#include <mutex>
#include <string>
#include <stdexcept>
namespace prumo {
using json=nlohmann::json;
struct Error:std::runtime_error { int status; Error(int s,const std::string& m):runtime_error(m),status(s){} };
std::string now();
std::string uid();
std::string hash(const std::string&);
std::string read(const std::string&);
void validate(const std::string&,const json&);
struct Store {
 sqlite3* db=nullptr;
 std::recursive_mutex mutex;
 explicit Store(const std::string&);
 ~Store();
 void exec(const std::string&);
 json query(const std::string&,const json& params=json::array());
 void run(const std::string&,const json& params=json::array());
};
struct Service {
 Store store;
 explicit Service(const std::string& path):store(path){}
 json list(const std::string& project="");
 json get(const std::string&);
 json save(json,const std::string& actor,const std::string& reason,bool importing=false);
 json remove(const std::string&,int,const std::string&,const std::string&);
 json accept(const std::string&,const json&);
 json revoke(const std::string&,const json&);
 json link(const json&);
 json import_seed(const std::string&);
 json events(const std::string&);
 json dashboard(const std::string&);
 void event(const std::string&,const std::string&,const std::string&,const std::string&,const json&,const std::string& fact="");
};
}
