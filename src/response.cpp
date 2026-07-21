#include "response.h"
#include <sstream>

std::string statusText(int statusCode) {
    switch (statusCode) {
        case 200: return "OK";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        default:  return "Internal Server Error";
    }
}

std::string buildResponse(int statusCode, const std::string& contentType, const std::string& body, bool keepAlive) {
    std::ostringstream response;

    response << "HTTP/1.1 " << statusCode << " " << statusText(statusCode) << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}