#pragma once
#include <iostream>
#include <functional>
#include <boost/asio.hpp>
#include "log.h"

namespace net {

    class Connection;
    using on_recive = std::function<void (std::string_view message)>;
    using on_error = std::function<void (const boost::system::error_code&)>;
    using on_timeout = std::function<void ()>;
    using on_connection = std::function<void (std::unique_ptr<Connection>)>;

    struct connection_callbacks {

        on_error error_callback;
        on_timeout timeout_callback;
        on_connection connection_callback;
        on_recive recive_callback;
    };

    class Connection {
    private:
        boost::asio::ip::tcp::socket& client_sock;
        size_t bytes_write, bytes_read = 0;
        boost::asio::streambuf buffer;
        on_recive recive_callback;
        on_error error_callback;

    public:
        Connection(boost::asio::ip::tcp::socket& client_sock, on_recive listener, on_error error_listener) 
             : client_sock {client_sock}, recive_callback {listener}, error_callback{error_listener} { }

        ~Connection() { 
            LOGI("Client total send %ld recieve %ld", bytes_write, bytes_read)
            LOGD("Destroy Connection"); 
        }
        std::string remote() { 
        
            std::stringstream str; 
            str << client_sock.remote_endpoint(); 
            return str.str(); 
        }
        
        void send_message(std::string_view message) {
            
            boost::asio::async_write(client_sock, boost::asio::buffer(message), 
                [this] (const boost::system::error_code& er, size_t write) {
                 
                    if (er) {
                        if (er.value() == boost::asio::error::operation_aborted) return;
                        else { error_callback(er); return; }
                    }
                    bytes_write += write;
                }
            );
        }


        void read_message () {

            boost::asio::async_read_until(client_sock, buffer, "\0", 
                [this] (const boost::system::error_code& er, size_t read) {

                    if (er) {
                        if (er.value() == boost::asio::error::operation_aborted) return;
                        else { error_callback(er); return; }
                    }

                    std::istream stream(&buffer);
                    std::string message;
                    std::getline(stream, message);
                    bytes_read += read;
                    recive_callback(message);
                }
            );
        }
    };
}