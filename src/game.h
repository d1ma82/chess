#pragma once
#include <string>

namespace game {
    
    void as_server (bool whites, unsigned short port);
    void as_client (const std::string& ip, const std::string& port, bool whites);
    void loop();
    void clear ();
}