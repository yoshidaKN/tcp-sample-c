# TCP/IPソケットプログラミング

## 概要

TCP/UDPで動作するソケットプログラミングの簡単なサンプル。

## コード

1. TCPエコークライアント/サーバー
   - `src/TCP-Echo/TCPEchoClient.c` TCPソケットでやり取りするエコークライアント
   - `src/TCP-Echo/TCPEchoServer.c` TCPソケットでやり取りするエコーサーバー
2. UDPエコークライアント/サーバー
   - `src/UDP-Echo/UDPEchoClient.c` UDPソケットでやり取りするエコークライアント
   - `src/UDP-Echo/UDPEchoServer.c` UDPソケットでやり取りするエコーサーバー
3. ノンブロッキングエコーサーバーとタイムアウト処理付きクライアント
   - `src/NonblockingIO/SigAction.c` シグナル処理のサンプルコード
   - `src/NonblockingIO/UDPEchoServer-SIGIO.c` SIGALRMやSIGCHLDといったシグナルによって処理の途中終了を防ぐUDPエコーサーバー
   - `src/NonblockingIO/UDPEchoClient-Timeout.c` SIGALRMシグナルでサーバーに再送要求を行う非同期UDPエコークライアント
4. クライアントの接続処理ごとにプロセス生成するマルチタスクエコーサーバークライアント
   - `src/Multitask/TCPEchoServer-fork.c` 接続要求ごとにプロセスを生成するTCPエコーサーバー
   - `src/Multitask/TCPEchoServer.c` 共通関数実装をまとめたもの
   - `src/Multitask/TCPEchoServer.h` ヘッダー
5. マルチスレッドエコーサーバークライアント
   - `src/Threads/TCPEchoServer-Threads.c` 接続要求ごとにPOSIXスレッドを生成するTCPエコーサーバー

## メモ（解説ドキュメント）
1. [ネットワークプロトコル](docs/network_protocol.md)
2. [TCPソケットプログラミング](docs/tcp_socket.md)
3. [UDPソケットプログラミング](docs/udp_socket.md)
4. [ノンブロッキングI/O](docs/NonblockingIO.md)
5. [マルチタスク](docs/multitask.md)

## 動作確認

- WSL バージョン: 2.3.26.0
- Ubuntu 22.04.5 LTS
- gcc version 11.4.0