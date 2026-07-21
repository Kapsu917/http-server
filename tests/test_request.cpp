#include "../src/request.h"
#include <cassert>
#include <iostream>

static void testBasicGetRequest() {
    std::string raw = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpRequest request = parseRequest(raw);

    assert(request.method == "GET");
    assert(request.path == "/index.html");
    assert(request.version == "HTTP/1.1");
    assert(request.headers.at("Host") == "localhost");
    assert(request.body.empty());

    std::cout << "testBasicGetRequest passed" << std::endl;
}

static void testPostWithBody() {
    std::string raw = "POST /api/echo HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello";
    HttpRequest request = parseRequest(raw);

    assert(request.method == "POST");
    assert(request.path == "/api/echo");
    assert(request.headers.at("Content-Length") == "5");
    assert(request.body == "hello");

    std::cout << "testPostWithBody passed" << std::endl;
}

static void testMissingHeaders() {
    std::string raw = "GET / HTTP/1.1\r\n\r\n";
    HttpRequest request = parseRequest(raw);

    assert(request.method == "GET");
    assert(request.path == "/");
    assert(request.headers.empty());

    std::cout << "testMissingHeaders passed" << std::endl;
}

static void testEmptyBody() {
    std::string raw = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpRequest request = parseRequest(raw);

    assert(request.body.empty());

    std::cout << "testEmptyBody passed" << std::endl;
}

static void testMalformedRequestLine() {
    // Only a method, no path or version — parser should not crash, just leave fields empty
    std::string raw = "GET\r\n\r\n";
    HttpRequest request = parseRequest(raw);

    assert(request.method == "GET");
    assert(request.path.empty());

    std::cout << "testMalformedRequestLine passed" << std::endl;
}

static void testIsRequestCompleteNoBody() {
    std::string raw = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    assert(isRequestComplete(raw) == true);
    std::cout << "testIsRequestCompleteNoBody passed" << std::endl;
}

static void testIsRequestCompletePartialBody() {
    std::string raw = "POST / HTTP/1.1\r\nContent-Length: 10\r\n\r\nhi";
    assert(isRequestComplete(raw) == false);
    std::cout << "testIsRequestCompletePartialBody passed" << std::endl;
}

static void testIsRequestCompleteHeadersNotDone() {
    std::string raw = "GET / HTTP/1.1\r\nHost: localhost";
    assert(isRequestComplete(raw) == false);
    std::cout << "testIsRequestCompleteHeadersNotDone passed" << std::endl;
}

int main() {
    testBasicGetRequest();
    testPostWithBody();
    testMissingHeaders();
    testEmptyBody();
    testMalformedRequestLine();
    testIsRequestCompleteNoBody();
    testIsRequestCompletePartialBody();
    testIsRequestCompleteHeadersNotDone();

    std::cout << "\nAll tests passed." << std::endl;
    return 0;
}