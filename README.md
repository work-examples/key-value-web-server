# Key-Value Web Server

Asyncronous HTTP web server with REST API for single key-value storage.
Persistency is implemented.
Gathers read-write statistics for whole storage and for each key.

## Run

1. Run CMake.
2. Compile project. You will get `WebServer` executable
3. Execute `WebServer`. It will listen on `127.0.0.1:8000` and it will use `database.json` file from current directory for persistency.
4. Run HTTP client script: `python3 client.py`

Database file example:
[database.example.json](database.example.json)

## API

Two API methods are supported.

Here is the prepared API request collection for Postman:
[WebServer.postman_collection.json](WebServer.postman_collection.json)

### Get value

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

### Set value

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

### Testing environment

CPU Intel Core i5 (8th gen), mobile version, 8 logical cores.  
Visual Studio 2019 (v16.11.13), Release build  
Windows 10 Version 21H1 (Build 19043.1645)  
Number of HTTP server threads: 8

### Results

| Number of <br/>request threads | 10K requests <br/>per thread, req/sec | 100K requests <br/>per thread, req/sec |
|---:|------:|-------:|
| 1 |  4 300 |  4 600 |
| 2 |  8 100 |  8 200 |
| 3 | 10 400 |  9 200 |
| 4 | 11 800 | 11 200 |
| 6 | 13 700 | 12 900 |
| 8 | 14 300 | 13 200 |
