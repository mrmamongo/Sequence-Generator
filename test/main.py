#!/usr/bin/python
# encoding: utf-8
import argparse
import logging
import os
import sys
from socket import socket, SOCK_STREAM, AF_INET
from time import sleep

import test_constants

_log_format = f"%(asctime)s - [%(levelname)s] - %(name)s - (%(filename)s).%(funcName)s(%(lineno)d) - %(message)s"


def get_file_handler():
    file_handler = logging.FileHandler("x.log")
    file_handler.setLevel(logging.INFO)
    file_handler.setFormatter(logging.Formatter(_log_format))
    return file_handler


def get_stream_handler():
    stream_handler = logging.StreamHandler()
    stream_handler.setLevel(logging.INFO)
    stream_handler.setFormatter(logging.Formatter(_log_format))
    return stream_handler


def get_logger(name):
    logger = logging.getLogger(name)
    logger.setLevel(logging.INFO)
    logger.addHandler(get_file_handler())
    logger.addHandler(get_stream_handler())
    return logger


logger = get_logger(__name__)


def test(func):
    def wrapper():
        sock = socket(AF_INET, SOCK_STREAM)
        passed, msg = func(sock)
        sock.close()
        if passed:
            logger.info(f"{func.__name__.upper()} Test passed")
        else:
            logger.fatal(f"{func.__name__.upper()} Test failed, {msg}")
        sleep(2)

    return wrapper


def right_test(sock: socket):
    sock.connect(('localhost', 8085))
    logger.debug('RIGHT TEST connection established')
    sleep(1)
    sent = sock.send(b'seq1 1 2\n')
    logger.debug('RIGHT TEST sent seq1 1 2')
    if sent == 0:
        return False, 'sent 0 bytes'
    sleep(1)
    data = sock.recv(1024)
    if data != b"OK" or sent == 0:
        return False, 'sent 0 bytes'
    sock.send(b'seq2 2 3\n')
    logger.debug('RIGHT TEST sent seq2 2 3')
    sleep(1)
    data = sock.recv(1024)
    if data != b"OK":
        return False, 'sent 0 bytes'
    sock.send(b'seq3 3 4\n')
    logger.debug('RIGHT TEST sent seq3 3 4')
    sleep(1)
    data = sock.recv(1024)
    if data != b"OK":
        return False, 'sent 0 bytes'
    sock.send(b'export seq\n')
    sleep(1)
    data = sock.recv(1024)
    if data != test_constants.right_const:
        return False, f'requested {data} but expected {test_constants.right_const}'
    return True, ''


def wrong_test(sock: socket):
    sock.connect(('localhost', 8085))
    logger.debug('WRONG TEST connection established')
    sleep(1)
    sock.send(b'seq1 0 2\n')
    logger.debug('WRONG TEST sent seq1 0 2')
    sent = sock.recv(1024)
    if sent != b'ERROR':
        return False, 'not error'
    sleep(1)
    sock.send(b'seq2 1 0\n')
    logger.debug('WRONG TEST sent seq1 0 2')
    sent = sock.recv(1024)
    if sent != b'ERROR':
        return False, 'not error'
    sleep(1)
    sock.send(b'seq3 0 2\n')
    logger.debug('WRONG TEST sent seq1 0 2')
    sent = sock.recv(1024)
    if sent != b'ERROR':
        return False, 'not error'
    sleep(1)
    sock.send(b'seq0 1 2\n')
    logger.debug('WRONG TEST sent seq1 0 2')
    sent = sock.recv(1024)
    if sent != b'ERROR':
        return False, 'not error'
    return True, ""


def timeout_test(sock: socket):
    sock.connect(('localhost', 8085))
    sock.send(b'seq1 1 2')
    sleep(15)
    if sock.__slots__[2] != 0:
        return True, ''
    else:
        return False, 'Socket is not closed'


def overflow_test(sock: socket):
    # print maximum number of int

    sock.connect(('localhost', 8085))
    sock.send(b'seq1 18446744073709551614 1\n')
    sock.recv(10)
    sleep(1)
    sock.send(b'seq2 18446744073709551614 1\n')
    sock.recv(10)
    sleep(1)
    sock.send(b'seq3 18446744073709551614 1\n')
    sock.recv(10)
    sleep(1)
    sock.send(b'export seq\n')
    sleep(1)
    data = sock.recv(1024).split()
    if data[0] != b'18446744073709551614' or data[7] != b'0' or data[10] != b'1':
        return False, f'requested {data} but expected 0 - 18446744073709551614, 7 - 0, 10 - 1'
    return True, ''


def break_connection_test(sock: socket):
    try:
        sock.connect(('localhost', 8085))
        sock.send(b'seq1 1 2\n')
        sock.close()
        sock.send(b'seq2 1 2\n')
        sock.recv(1024)
    except OSError:
        return True, ''
    except Exception as e:
        return False, f'Wrong exception {e}'


def buddy(sock):
    sock.send(b'seq1 1 2\n')
    sock.recv(1024)
    sock.send(b'seq2 1 2\n')
    sock.recv(1024)
    sock.send(b'seq3 1 2\n')
    sock.recv(1024)
    sock.send(b'export seq\n')
    sleep(1)
    data = sock.recv(1024)
    if data != test_constants.right_buddy:
        return False, f'requested {data} but expected {test_constants.right_const}'
    return True, ''


def multiple_threads_test(threads, t=buddy):
    for i in range(threads):
        print(f'PARENT {os.getpid()}: Forking {i}')
        worker_pid = os.fork()
        if not worker_pid:
            with socket(AF_INET, SOCK_STREAM) as sock:
                sock.connect(('localhost', 8085))
                passed, msg = t(sock)
                sock.close()
                print(f'WORKER {i}: {passed}: {msg}')
                sys.exit(i)

    for i in range(threads):
        print(f'PARENT: Waiting for {i}')
        done = os.wait()
        print(f'PARENT: Child done: {done}')


tests = {
    'right_test': right_test,
    'wrong_test': wrong_test,
    'timeout_test': timeout_test,
    'overflow_test': overflow_test,
    'break_connection_test': break_connection_test,
}


def main():
    parser = argparse.ArgumentParser(description='Test script for the server')
    parser.add_argument('-t', '--test', type=str, help='Test to run', required=True)
    parser.add_argument('-a', type=bool, help='Run all tests asynchronously', required=False, default=False)
    parser.add_argument('--threads', type=int, help='Number of threads to run', required=False, default=1)
    args = parser.parse_args()
    if args.test.lower() == 'multi':
        multiple_threads_test(args.threads)
        return
    if args.test.lower() == 'all':
        if args.a:
            for t in tests.values():
                multiple_threads_test(args.threads, t)
        else:
            for case in tests:
                test(tests[case])()
    elif args.test in tests:
        test(tests[args.test])()
    else:
        print("No such test", "Usage -t <test_name> or -t all, -a to run all tests asynchronously")


if __name__ == '__main__':
    main()
