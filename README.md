# C++23 HTTP/1.1 Server

An HTTP/1.1 server implementation in C++23, built as part of the CodeCrafters challenge.

## Features

- **Routing**: Express.js-style API with parameterized routes (`/users/:id`)
- **Methods**: GET and POST
- **Persistent connections**: HTTP/1.1 keep-alive with explicit `Connection: close` support
- **Compression**: gzip encoding via zlib (Accept-Encoding negotiation)
- **Multiplexing**: epoll-based non-blocking I/O for concurrent connections
- **File serving**: Static file read/write with binary support

## HTTP concepts

| Concept | Implementation |
|---|---|
| Connection multiplexing | `epoll_create1` + `epoll_wait` event loop |
| Non-blocking I/O | `fcntl(O_NONBLOCK)` on all sockets |
| Persistent connections | Keep-alive by default, close on `Connection: close` header |
| Request parsing | `string_view`-based zero-copy parser |
| Route matching | Trie with exact-match priority over parameter capture |
| Gzip compression | zlib `deflateInit2` with gzip window bits (`15 + 16`) |

## C++23 highlights

- `std::expected<T, E>` for error handling without exceptions (request parsing pipeline)
- `std::string_view` for zero-copy HTTP parsing
- `std::optional<T>` for route handler lookup
- `std::function` with move-captured lambdas for route handlers
- `constexpr` status codes and configuration constants
- Structured bindings in range-based loops

## Notable design patterns

### Express.js-style routing

```cpp
http::get("/echo/:content", [](const http::Request &req, http::Response &res) {
  res.send(req.params.at("content"));
});

http::post("/files/:filename", [](const http::Request &req, http::Response &res) {
  // ...
});
```

Handlers receive `(const Request &, Response &)` and modify the response in-place. GET routes default to `200 OK`, POST routes to `201 Created`.

### Route trie with parameter capture

```
GET /echo/:content
GET /files/:filename
GET /users/:id/posts/:postId

         (root)
        /      \
     echo     files    users
      |         |        |
   :content  :filename  :id
                          |
                        posts
                          |
                       :postId
```

Exact path segments are matched before parameter nodes, so `/users/me` takes priority over `/users/:id`.

### epoll event loop

```
                    epoll_wait()
                        |
            +-----------+-----------+
            |                       |
       server_fd               client_fd
     (new connection)        (data ready)
            |                       |
    accept + set                parse request
    non-blocking               find route
    + add to epoll             execute handler
                               encode gzip?
                               write response
                                    |
                          Connection: close?
                          /                \
                        yes                no
                   shutdown(SHUT_WR)    keep in epoll
                   remove from epoll    (next request)
                   close fd
```

### Request processing pipeline

```
raw bytes
  -> parse_request() : std::expected<Request, ParseError>
    -> get_route_handler(request) : std::optional<RouteHandler>
      -> handler(request, response)
        -> encode_gzip() if Accept-Encoding matches
          -> check Connection: close
            -> response.to_str()
```

Each stage uses value types (`expected`, `optional`) instead of exceptions for control flow.

## Dependencies

- CMake 3.14+
- C++23 compiler
- zlib (`zlib1g-dev` on Debian/Ubuntu)
- GoogleTest (fetched automatically via CMake)
- pthreads

## Build

```sh
cmake -B build -S .
cmake --build build
```

## Project structure

```
lib/
├── config.h         # Port, buffer size, directory path
├── types.h          # Request, Response, Headers, Status types
├── parse.cpp/h      # HTTP request parser (string_view-based)
├── route.cpp/h      # Trie router with parameter extraction
├── response.cpp/h   # Response builder with gzip compression
└── server.cpp/h     # epoll TCP server with persistent connections

src/
└── main.cpp         # Route definitions and server startup

tests/
├── parse.cpp        # Header parsing and connection semantics
├── route.cpp        # Parameter extraction and priority matching
└── response.cpp     # Gzip encoding and header generation
```
