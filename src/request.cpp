#include "request.h"
#include <sstream>

// Trims leading/trailing whitespace and \r from a string.
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \r\t\n");
    size_t end = s.find_last_not_of(" \r\t\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

HttpRequest parseRequest(const std::string& rawRequest) {
    HttpRequest request;

    // Headers and body are separated by a blank line ("\r\n\r\n")
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    std::string headerSection = (headerEnd != std::string::npos)
        ? rawRequest.substr(0, headerEnd)
        : rawRequest;

    std::istringstream stream(headerSection);
    std::string line;

    // First line: "METHOD /path HTTP/1.1"
    if (std::getline(stream, line)) {
        std::istringstream requestLine(trim(line));
        requestLine >> request.method >> request.path >> request.version;
    }

    // Remaining lines: "Key: Value"
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty()) continue;

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        std::string key = trim(line.substr(0, colonPos));
        std::string value = trim(line.substr(colonPos + 1));
        request.headers[key] = value;
    }

    // Body starts right after the blank line, if one exists
    if (headerEnd != std::string::npos) {
        request.body = rawRequest.substr(headerEnd + 4);
    }

    return request;
}