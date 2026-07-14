#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>

// Builds a complete HTTP/1.1 response as a single string:
// status line + headers + blank line + body.
std::string buildResponse(int statusCode, const std::string& contentType, const std::string& body);

// Maps a status code to its reason phrase (e.g. 200 -> "OK").
std::string statusText(int statusCode);

#endif