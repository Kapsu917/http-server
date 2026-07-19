#include "request.h"
#include <sstream>

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \r\t\n");
    size_t end = s.find_last_not_of(" \r\t\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

HttpRequest parseRequest(const std::string& rawRequest) {
    HttpRequest request;

    size_t headerEnd = rawRequest.find("\r\n\r\n");
    std::string headerSection = (headerEnd != std::string::npos)
        ? rawRequest.substr(0, headerEnd)
        : rawRequest;

    std::istringstream stream(headerSection);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream requestLine(trim(line));
        requestLine >> request.method >> request.path >> request.version;
    }

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty()) continue;

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        std::string key = trim(line.substr(0, colonPos));
        std::string value = trim(line.substr(colonPos + 1));
        request.headers[key] = value;
    }

    if (headerEnd != std::string::npos) {
        request.body = rawRequest.substr(headerEnd + 4);
    }

    return request;
}

bool isRequestComplete(const std::string& buffer) {
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return false; // headers haven't fully arrived yet
    }

    HttpRequest partial = parseRequest(buffer);
    auto it = partial.headers.find("Content-Length");
    if (it == partial.headers.end()) {
        return true; // no body expected, headers are enough
    }

    size_t expectedBodyLength = std::stoul(it->second);
    size_t actualBodyLength = buffer.size() - (headerEnd + 4);
    return actualBodyLength >= expectedBodyLength;
}