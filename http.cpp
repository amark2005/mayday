
#define CPPHTTPLIB_OPENSSL_SUPPORT


#include "src/httplib.h"
#include <string>
#include<nlohmann/json.hpp>
using nlohmann::json;

struct Plane{
  std::string flight,category,emergency,tailnum,type;
  double alt,speed,mach,lat,lon,vspeed;
  bool is_live;

  //update plane data
  bool updatedata(const std::string& rawdata){
    auto data=json::parse(rawdata);
    if(data["total"]==0){
      is_live=false;
      return false;}
    auto& ac=data["ac"][0];
    flight   = ac.value("flight", "");
    category = ac.value("category", "");
    emergency =ac.value("emergency", "none");
    alt    = ac.value("alt_baro", 0.0);
    speed  = ac.value("gs", 0.0);
    mach   = ac.value("mach", 0.0);
    lat    = ac.value("lat", 0.0);
    lon    = ac.value("lon", 0.0);
    vspeed = ac.value("baro_rate", 0.0);
    type=ac.value("t","");
    tailnum=ac.value("r","");
    return true;
  }
  int getdata(std::string CS){
    std::string Callsign=CS;
    httplib::SSLClient cli("api.adsb.lol");
    std::string path="/v2/callsign/"+Callsign;
    auto res=cli.Get(path.c_str());
    if (!res) {
      return 1;
      is_live=false;
    }
    is_live=true;
    updatedata(res->body);
    return 0;
  }
};




