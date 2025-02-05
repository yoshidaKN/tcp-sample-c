# TCP/IPソケットプログラミング

## 概要

TCP/UDPで動作するソケットプログラミングの簡単なサンプル。

## コード

1. TCPエコークライアント/サーバー
   - `src/TCPEchoClient.c` TCPソケットでやり取りするエコークライアント
   - `src/TCPEchoServer.c` TCPソケットでやり取りするエコーサーバー
2. UDPエコークライアント/サーバー
   - `src/UDPEchoClient.c` UDPソケットでやり取りするエコークライアント
   - `src/UDPEchoServer.c` UDPソケットでやり取りするエコーサーバー

## メモ（解説ドキュメント）
1. [ネットワークプロトコル](docs/network_protocol.md)
2. [TCPソケットプログラミング](docs/tcp_socket.md)
3. [UDPソケットプログラミング](docs/udp_socket.md)
4. [ノンブロッキングI/O](docs/NonblockingIO.md)
5. [マルチタスク](docs/multitask.md)
6. [マルチスレッド](docs/thread.md)

## 動作確認

- WSL バージョン: 2.3.26.0
- Ubuntu 22.04.5 LTS
- gcc version 11.4.0
