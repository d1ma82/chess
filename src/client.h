#pragma once
#include <memory>
#include <mutex>
#include "connection.h"

namespace net {

    class TCPClient {
    private:
        connection_callbacks callbacks;
        boost::asio::deadline_timer timer;
        boost::asio::ip::tcp::resolver::iterator endpoint;
        boost::asio::ip::tcp::socket socket;
        std::mutex mutex;
        std::unique_ptr<Connection> connection {nullptr};

        std::unique_ptr<Connection> acquire() {

            std::lock_guard<std::mutex> lock(mutex);
            return std::make_unique<Connection>(socket, callbacks.recive_callback, callbacks.error_callback);
        }   

    public:
        TCPClient (
            const std::string& ip, 
            const std::string& port, 
            boost::asio::io_service& service, 
            connection_callbacks listener
        ): callbacks{listener}, timer(service), socket(service) {
            
            endpoint = boost::asio::ip::tcp::resolver(service).resolve({ip, port});
        }

        void connect_timeout (unsigned short timeout) {

            timer.expires_from_now(boost::posix_time::seconds(timeout));
            timer.async_wait([this] (const boost::system::error_code& er) {

                if (er) {
                    if (er.value() == boost::asio::error::operation_aborted) return;
                    else { callbacks.error_callback(er); return; }
                }
                socket.close();
                callbacks.timeout_callback();
            });           

            boost::asio::async_connect(socket, endpoint, 
                [this] (const boost::system::error_code& er, boost::asio::ip::tcp::resolver::iterator i) {

                    timer.cancel();
                    if (er) callbacks.error_callback(er);
                    else callbacks.connection_callback(acquire());
                }
            );
        };
    };
}