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

// Returns true once `buffer` holds a full HTTP request: headers finished,
// and the full body (per Content-Length) has arrived if one is expected.
// Needed because epoll delivers data in arbitrary-sized chunks — a single
// recv() is no longer guaranteed to contain the whole request.
bool isRequestComplete(const std::string& buffer);

#endif