# Key-Value Web Server

An HTTP web server with a REST API for persistent key-value storage.
Collects read-write statistics for the entire storage and for each key.

**Language**: `C++17`  
**Dependencies**: `Boost v1.78.0`, `RapidJSON v1.1.0-b557259-p0`, `CrowCpp v1.0+2`, `PyInstaller` (optional)  
**Software requirements**: `CMake 3.2+`, C++17 compatible compiler, `Python 3.7+`  
**Operation systems**: `Windows`, `Linux`, `macOS`

## Test Task Description

You need to write two applications, a client and a server, that communicate with each other.

### Server

The server has a configuration file on disk (`config.txt`). It stores key/value data.
You can use any format. At startup, the server reads the configuration file.
The server has 2 commands `get` and `set`.
The first command gets the value by the key, the second one sets it.

Command format: `$get <key>, $set <key>=<value>`

Recording a value must be accompanied by updating the file on disk.
You can update it not immediately, but periodically.
It is desirable to write to a file in a separate thread.
The server supports an arbitrary number of clients.
The server must be multithreaded, i.e. commands must be processed in parallel.
Reading should occur with minimal delay. We believe that the record is a rare situation.
The server should keep statistics of requests, and output to the console every 5 seconds
how many requests it has completed in total and in the last 5 seconds.
Optionally implement the calculation of access statistics for each key,
return this statistics to any of the commands as a result, for example:

```text
$get tree
```

Result:

```text
tree=Blue
reads=10
writes=1
```

When developing, you can use third-party libraries for parsing and for the network.
For example `rapidjson`, `boost` etc.

### Client Application

Single threaded. Connects to the server, then selects a random key
from the hardcoded list and executes `$get` on the server with a 99% probability,
and in 1% of cases writes random data to this key by executing `$set`.
The results of command execution are written to the console.
This is repeated in a loop 10K times (without breaking the connection) and the application terminates.
Optionally make reconnect to the server in case of a disconnection, or if the server is offline,
i.e. the client waits until it appears on the network.
The client can be written in any language, even in Python.

## Task Implementation Remarks

### Choosing Web Server Implementation

I decided to use well-known HTTP protocol for client-server communication.
Thus, we can easily debug the solution using any browser, programs like `Postman`.
This way I could reuse existing solutions to implement HTTP server in C++.

Choosing a C++ HTTP server library was quite difficult.
I had the following requirements:

- the library should be implemented with a performance in mind
- the library must be alive. I.e. it must have fresh commits and releases.
- the library should be widely used. I decided its repository should have at least 1000 stars in GitHub.
- library must be multithreaded
- library must support HTTP keep-alive
- library must support Linux and Windows
- I would prefer the library to be object-oriented and not to have C-style API.
- I would like the library to be easily integrated. I.e. it should be easy to compile (maybe header-only).
  It would be better if the library is supported by Hunter or Conan package managers.
- It should have complete HTTP server implementation, not just set of tools to create one.

I was choosing among many libraries:
`Beast`, `civetweb`, `libevhtp`, `mongoose`, `PocoCpp`, `cppserver`, `seasocks`,
`cpp-httplib`, `restinio`, `cpprestsdk`, `pistache`, `crow`, `CrowCpp`, `drogon`.

Finally, I settled on `CrowCpp`.

### Other Implementation Features

The heart of the server engine uses `std::shared_mutex
to optimize performance for many readers and single writer.

Web server statistics values are only protected by `std::atomic`,
which allows small inconsistency between different values.
This is a reasonable tradeoff for speed.

Solution is cross-platform.

It uses CMake to generate a C++ project.
All external dependencies are under source control
or downloaded automatically by embedded Hunter package manager (`Boost`, `RapidJSON`).

The implementation is divided into several components that are completely independent of each other.
The **Inversion of Control** pattern is used to combine them into a working solution.

I have used the **pImpl** pattern to hide implementation details in the `.cpp` file.
It also speeds up compilation by limiting the number of `.cpp` files
compiled with included complex C++ templates.

The solution also makes heavy use of move semantics.

## Compile and Run

1. Run CMake.
2. Compile project. You will get `WebServer` executable
3. Execute `WebServer`. It will listen on `127.0.0.1:8000`
   and it will use `database.json` file from current directory for persistence.
4. Run HTTP client script: `python3 client.py`

Database file example:
[database.example.json](database.example.json)

Client application is run this way. It needs python3 and has no external dependencies.
CI is also preparing `Client.exe` executable which is a compiled version of `client.py` with Python inside.

```bash
python3 client.py
```

## Web API

Two API methods are supported.

Here is the prepared API request collection for Postman:
[WebServer.postman_collection.json](WebServer.postman_collection.json)

### Get Value

`GET` <http://127.0.0.1:8000/api/records/{key-name}>

Reply body example:

```json
{
    "name": "name 1",
    "value": "my value",
    "stats.reads": 2,
    "stats.writes": 1
}
```

### Set Value

`POST` <http://127.0.0.1:8000/api/records/{key-name}>

Request body:

```json
{
    "value": "my value"
}
```

Reply body example:

```json
{
    "name": "name 1"
}
```

## Benchmark

### Testing Environment

CPU Intel Core i5 (8th gen), mobile version, 8 logical cores.  
Visual Studio 2019 (v16.11.13), Release build  
Windows 10 Version 21H1 (Build 19043.1645)  
Number of HTTP server threads: 8  
Logging of every request is disabled for both server and client side.

### Results

| Number of <br/>request threads | 10K requests <br/>per thread, req/sec | 100K requests <br/>per thread, req/sec |
|---:|------:|-------:|
| 1 |  4 300 |  4 600 |
| 2 |  8 100 |  8 200 |
| 3 | 10 400 |  9 200 |
| 4 | 11 800 | 11 200 |
| 6 | 13 700 | 12 900 |
| 8 | 14 300 | 13 200 |
