#ifndef CPP_WEB_SERVER_WEBSOCKET_REQUEST_HANDLER_HPP
#define CPP_WEB_SERVER_WEBSOCKET_REQUEST_HANDLER_HPP

#include "async_web_server_cpp/http_request_handler.hpp"
#include "async_web_server_cpp/websocket_connection.hpp"

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace async_web_server_cpp
{

class WebsocketConnection;

typedef boost::function<WebsocketConnection::MessageHandler(
    const HttpRequest&, boost::shared_ptr<WebsocketConnection>)>
    WebsocketRequestHandler;

/**
 * A HTTP request handler that upgrades a HttpConnection to a
 * WebsocketConnection.
 */
class WebsocketHttpRequestHandler
{
public:
    WebsocketHttpRequestHandler(WebsocketRequestHandler handler);
    bool operator()(const HttpRequest& request,
                    boost::shared_ptr<HttpConnection> connection,
                    const char* begin, const char* end);

    static const std::string KEY_MAGIC_STRING;

private:
    WebsocketRequestHandler handler_;
};

}  // namespace async_web_server_cpp

#endif
