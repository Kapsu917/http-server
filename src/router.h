#ifndef ROUTER_H
#define ROUTER_H

#include "request.h"
#include <string>

// Returns the MIME type for a file path based on its extension.
std::string getMimeType(const std::string& path);

// Routes a parsed HTTP request to the correct handler (static file,
// /api/status, /api/echo) and returns a complete HTTP response string.
std::string routeRequest(const HttpRequest& request);

// Total requests served since the server started (thread-safe read).
int getRequestCount();

#endif