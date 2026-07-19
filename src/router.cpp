#include "router.h"
#include "response.h"
#include <fstream>
#include <sstream>
#include <mutex>
#include <ctime>

static std::mutex requestCountMutex;
static int requestCount = 0;
static time_t serverStartTime = time(nullptr);

std::string getMimeType(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")  return "text/css";
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")   return "text/javascript";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")  return "image/png";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg")  return "image/jpeg";
    return "application/octet-stream";
}

// Reads a file fully into outContent. Returns false if it doesn't exist.
static bool readFile(const std::string& filePath, std::string& outContent) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return false;

    std::ostringstream buffer;
    buffer << file.rdbuf();
    outContent = buffer.str();
    return true;
}

static std::string serveStaticFile(std::string path) {
    if (path == "/") path = "/index.html";

    std::string filePath = "./public" + path;
    std::string content;

    if (!readFile(filePath, content)) {
        return buildResponse(404, "text/plain", "404 Not Found");
    }

    return buildResponse(200, getMimeType(filePath), content);
}

static std::string handleApiStatus() {
    int count;
    {
        std::lock_guard<std::mutex> lock(requestCountMutex);
        count = requestCount;
    }
    int uptime = (int)difftime(time(nullptr), serverStartTime);

    std::ostringstream json;
    json << "{"
         << "\"server\": \"running\", "
         << "\"uptime\": " << uptime << ", "
         << "\"requests_served\": " << count
         << "}";

    return buildResponse(200, "application/json", json.str());
}

static std::string handleApiEcho(const HttpRequest& request) {
    return buildResponse(200, "text/plain", request.body);
}

std::string routeRequest(const HttpRequest& request) {
    // Count every request that comes through, regardless of route
    {
        std::lock_guard<std::mutex> lock(requestCountMutex);
        requestCount++;
    }

    if (request.path == "/api/status" && request.method == "GET") {
        return handleApiStatus();
    }

    if (request.path == "/api/echo" && request.method == "POST") {
        return handleApiEcho(request);
    }

    if (request.method == "GET") {
        return serveStaticFile(request.path);
    }

    return buildResponse(405, "text/plain", "Method Not Allowed");
}

int getRequestCount() {
    std::lock_guard<std::mutex> lock(requestCountMutex);
    return requestCount;
}