#!/usr/bin/env python3

import json
import http.client
import multiprocessing
import random
import time
import urllib.parse

# =======================================

CONNECT_HOST = '127.0.0.1'
CONNECT_PORT = 8000

VALUE_NAMES = [
    'name',
    'name2',
    'name 3',
    'name #4',
    'another',
    'non-existent',
    *[f'name_{x}' for x in range(100_000)],
]

COUNT_OF_OPERATIONS_PER_PROCESS = 10_000
COUNT_OF_PROCESSES = multiprocessing.cpu_count()

PERCENT_OF_WRITES = 1

LOG_EVERY_REQUEST = False

# =======================================


def one_request(conn: http.client.HTTPConnection) -> None:
    rand = random.randint(1, 100)
    writing = (rand <= PERCENT_OF_WRITES)

    name = random.choice(VALUE_NAMES)
    url = '/api/records/' + urllib.parse.quote_plus(name)

    headers_get = {
        'Accept': 'application/json',
    }
    headers_post = {
        'Content-type': 'application/json',
        'Accept': 'application/json',
    }

    if writing:
        new_value = f'val_{random.randint(1000, 1000_000_000)}'

        data = {"value": new_value}

        if LOG_EVERY_REQUEST:
            print('====================================================')
            print(f'POST {url}: {data}')

        request_body_bytes = json.dumps(data, indent=2, sort_keys=True).encode(encoding='utf8')
        conn.request(method='POST', url=url, body=request_body_bytes, headers=headers_post)
    else:
        if LOG_EVERY_REQUEST:
            print(f'GET {url}')
        conn.request(method='GET', url=url, headers=headers_get)

    response = conn.getresponse()
    if LOG_EVERY_REQUEST:
        print(f'REPLY: {response.status} {response.reason}')

    body_bytes = response.read()

    if response.status not in (200, 404):
        print(f'RAW_REPLY_BODY: {body_bytes}')

    if writing:
        assert response.status == 200
    else:
        assert response.status in (200, 404)

    if LOG_EVERY_REQUEST and response.status == 200:
        body_text = body_bytes.decode(encoding='utf8')
        body = json.loads(body_text)

        assert body['name'] == name
        print(f'REPLY_BODY: {body}')


def main() -> None:
    print('main: begin')

    print('main: connect server')

    operation_count = COUNT_OF_OPERATIONS_PER_PROCESS

    conn = http.client.HTTPConnection(host=CONNECT_HOST, port=CONNECT_PORT)
    print('HTTP connection was established')

    begin = time.perf_counter()

    for i in range(operation_count):
        one_request(conn)
        if (i + 1) % 1000 == 0:
            print(f'STATS: Finished {i + 1} requests')

    end = time.perf_counter()
    elapsed = max(end - begin, 0.001)
    ops_per_sec = int(operation_count / elapsed)

    conn.close()

    print(f'main: It took {elapsed:.3f} seconds to execute {operation_count} requests; {ops_per_sec} requests per second')

    print('main: end')


def main_owner() -> None:
    print(f'main_owner: start {COUNT_OF_PROCESSES} child processes')

    pool = [multiprocessing.Process(target=main, args=()) for _ in range(COUNT_OF_PROCESSES)]

    begin = time.perf_counter()

    for process in pool:
        process.start()

    for process in pool:
        process.join()
        print('Child process finished')

    end = time.perf_counter()
    elapsed = max(end - begin, 0.001)
    operation_count = COUNT_OF_OPERATIONS_PER_PROCESS * COUNT_OF_PROCESSES
    ops_per_sec = int(operation_count / elapsed)

    print(f'main_owner: It took {elapsed:.3f} seconds to execute {operation_count} requests; {ops_per_sec} requests per second')

    print('main_owner: end')


if __name__ == '__main__':
    main_owner()
