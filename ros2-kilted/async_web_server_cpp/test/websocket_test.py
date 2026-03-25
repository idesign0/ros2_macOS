#!/usr/bin/env python3

import websocket
import unittest
import time

class TestWebsocket(unittest.TestCase):
    def setUp(self):
        self.ws = websocket.create_connection("ws://localhost:9849/websocket_echo")

    def tearDown(self):
        self.ws.close()

    def test_ok(self):
        self.ws.send("hello")
        self.assertEqual("hello", self.ws.recv())
        self.ws.send("test")
        self.assertEqual("test", self.ws.recv())
        self.ws.send("hi")
        self.assertEqual("hi", self.ws.recv())

        self.ws.ping(b"test ping")
        ping_echo = self.ws.recv_frame()
        self.assertEqual(9, ping_echo.opcode)
        self.assertEqual(b"test ping", ping_echo.data)

        self.ws.pong(b"test pong")
        pong_echo = self.ws.recv_frame()
        self.assertEqual(10, pong_echo.opcode)
        self.assertEqual(b"test pong", pong_echo.data)

if __name__ == '__main__':
    time.sleep(1) # ensure server is up

    unittest.main()
