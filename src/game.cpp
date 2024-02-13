#include "game.h"
#include <iostream>
#include <thread>
#include "chess.h"
#include "server.h"
#include "client.h"
#include "GLFW_wnd.h"
#include "board.h"

namespace game {
    
    window::GLFW* window {nullptr};

    boost::asio::io_service service;
    net::TCPServer* server {nullptr};
    net::TCPClient* client {nullptr};
    filter::Board* board {nullptr};
    std::unique_ptr<net::Connection> connection {nullptr};
    
    const int WIDTH=640;
    const int HEIGHT=640;
    const char* WHITES = "whites";
    const char* BLACKS = "blacks";
    const char* self_color;


/**
 * 
 * The callback function for exchanging messages 
 * between the server and the client, here information about the game progress takes place
*/
    void on_message(const std::string& message) {
        
        if (!connection) return;

        if (message.find("Hello", 0) != std::string::npos) {
            
            LOGD("Request opponent color")
            connection->send_message("color");
        }
        else if (message.find("move_done") != std::string::npos) {

            size_t colon = message.find(":");
            if (connection) connection->send_message("move:"+message.substr(colon+1));        // TODO: check if not connected
        }
        else if (message.find("move") != std::string::npos) {
            
            size_t colon = message.find(":");
            chess::opponent_move(message.substr(colon+1));
        }
        else if (message.compare("color") == 0) {

            std::string send_str {"color:"+std::string(self_color)};
            LOGD("Color request, send %s", send_str.c_str())
            connection->send_message(send_str);
        }
        else if (message.compare(std::string("color:"+std::string(WHITES))) == 0) {
            
            LOGD("set color %s", BLACKS)
            chess::init(false, on_message);
            self_color = BLACKS;
        }
        else if (message.compare(std::string("color:"+std::string(BLACKS))) == 0) {
            
            LOGD("set color %s", WHITES)
            chess::init(true, on_message);
            self_color = WHITES;
        }
        connection->read_message();
    }

    void init_internal (bool whites) {

        chess::init(whites, on_message);        
        
        window = new window::GLFW(dims{WIDTH, HEIGHT}, "Chess");
        window->set_mouse_key_listener([] (double X, double Y) {
            
                int x = static_cast<int>(X/WIDTH*chess::BOARD_SIZE);
                int y = static_cast<int>(Y/HEIGHT*chess::BOARD_SIZE);
              //  LOGD("Coord %dx%d", x,y)
                chess::on_select_cell(x,y);
        });
        board  = new filter::Board(chess::BOARD_SIZE, dims{WIDTH, HEIGHT});
        board->set_uniformuiv(chess::position.data());
        (*window)->attach_filter(board);
        LOGD("Creating game for %s", (whites? WHITES: BLACKS))        
    }


    void on_timeout() { LOGE("Operation timeout") }

    void on_error (const boost::system::error_code& error) {
        
        LOGE("Network error: %s", error.message().c_str())
    }
/**
 * Client connected, callback
*/
    void on_connection (std::unique_ptr<net::Connection> connection_) {
        
        game::connection = std::move(connection_);

        LOGI("Connection from %s", game::connection->remote().c_str())
        
        game::connection->send_message ("Hello from chess game server\n");
        game::connection->read_message();
    }
/**
 * Connect to server callback
*/
    void on_connect_server (std::unique_ptr<net::Connection> connection_) {

        game::connection = std::move(connection_);
        LOGI("Connected to %s", game::connection->remote().c_str())
        game::connection->read_message();
    }

/**
 *  Create new game, and switch to server mode
 *  
 */ 
    void as_server (bool whites, unsigned short port) {
        
        assert(client==nullptr);
        self_color = whites? WHITES: BLACKS;
        server = new net::TCPServer(port, service, {on_error, on_timeout, on_connection, on_message});
        server->accept_timeout(600); 
        
        std::thread thread = std::thread([] { service.run(); });
        if (thread.joinable()) thread.detach();
        init_internal(whites); 
    }
/**
 *  Connect to game, and switch to client mode
 *  
 */ 
    void as_client (const std::string& ip, const std::string& port, bool whites) {
        
        assert(server==nullptr);
        self_color = whites? WHITES: BLACKS;
        init_internal(whites);

        client = new net::TCPClient(ip, port, service, {on_error, on_timeout, on_connect_server, on_message});
        client->connect_timeout(600);
        std::thread thread = std::thread([] { service.run(); });
        if (thread.joinable()) thread.detach();
    }

    void loop() {

        while (!window->should_close()) {

            window->draw();
        }
    }

    void clear () {
// TODO: send EOF when exit
        chess::clear();
        delete board; board = nullptr;
        delete window; window = nullptr; 
        delete server; server = nullptr;
        delete client; client = nullptr;        
        service.stop();
        LOGD("Game destroyed")
    }

}