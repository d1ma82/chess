#include <iostream>
#include <boost/program_options.hpp>
#include <stdexcept>
#include <string>
#include "log.h"
#include "game.h"

namespace {

    namespace po=boost::program_options;
    po::options_description general ("General cofiguration");
    int result = 0;
    bool server = false;
    unsigned short port  = 3000;
    bool whites;
    std::string ip_port;

    void print_help() {

        std::cout<<general<<'\n';
        exit(result);
    }

    void init (int argc, char*argv[]) {

        general.add_options()
            ("help", "This text")
            ("create", po::bool_switch(&server), "create a new game, Server mode on")
            ("port", po::value<unsigned short>(), "server port, used when create option used")
            ("whites", po::bool_switch(&whites), "play whites")
            ("connect", po::value<std::string>(&ip_port), "connect a game \"<IP>:<Port>\"");
        
        if (argc==1) print_help();                
        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, general), vm);
        po::notify(vm);

        if (vm.count("help")) print_help();

        if (server) {

            port = vm.count("port")? vm["port"].as<unsigned short>(): 3000;
            game::as_server (whites, port);
            LOGD("As server")
        } 
        else if (vm.count("connect")) {
            
            if (ip_port.empty()) print_help();
            size_t colon = ip_port.find(":");
            std::string ip = ip_port.substr(0, colon);
            std::string port_s = ip_port.substr(colon+1);
            game::as_client(ip, port_s, whites);
            LOGD("As client")
        }
        else throw std::invalid_argument("Not implemented yet");
    }

    void loop() {
    
        game::loop();
    }

    void clear() {

        game::clear();
    }
}

int main(int argc, char*argv[]) {

    try {
        init(argc, argv);
        loop();
        clear();    
    }
    catch (std::exception& e) {
        
        result=-2;
        LOGE("%s", e.what())
        clear();
    }
    catch (...) {
        
        result=-3;
        LOGE("Unknown exception %d, %s\n", errno, strerror(errno))
        clear();
        throw;
    }
    return result;
}