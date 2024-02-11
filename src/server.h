#pragma once
#include <memory>
#include <mutex>
#include "connection.h"

namespace net {

    class TCPServer {
    private:
        connection_callbacks callbacks;
        boost::asio::deadline_timer timer;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;
        std::mutex mutex;
        std::unique_ptr<Connection> connection {nullptr};
        
        std::unique_ptr<Connection> acquire() {

            std::lock_guard<std::mutex> lock(mutex);
            if (!connection) connection = 
                std::make_unique<Connection>(socket, callbacks.recive_callback, callbacks.error_callback);
            return std::move(connection);
        }        
            
    public:

        TCPServer (unsigned short port, boost::asio::io_service& service, connection_callbacks listener)
            : callbacks{listener}, timer(service), 
             acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
             socket(service) {
        }

        void accept_timeout(unsigned short timeout) {
            
            timer.expires_from_now(boost::posix_time::seconds(timeout));

            timer.async_wait([this] (const boost::system::error_code& er) {

                if (er) {
                    if (er.value() == boost::asio::error::operation_aborted) return;
                    else { callbacks.error_callback(er); return; }
                }
                       
                acceptor.cancel();
                socket.close();
                callbacks.timeout_callback();
            });

            acceptor.async_accept(socket, [this] (const boost::system::error_code& er) {

                timer.cancel();
                if (er) callbacks.error_callback(er);
                else callbacks.connection_callback(acquire());
            } );
        }
    };
}
