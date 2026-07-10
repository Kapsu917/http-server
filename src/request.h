#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Parses a raw HTTP request string into an HttpRequest struct.
HttpRequest parseRequest(const std::string& rawRequest);

#endif