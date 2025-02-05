# マルチタスク

現行のTCPエコーサーバーでは、一度の一つのクライアントしか処理できない。サーバーは最初のクライアントとの通信を終えるまで、サーバーは後続のクライアントのデータをエコーバックすることはない。そのために今回は「クライアントごとにプロセスを生成する」方法を見ていく。しかし後に見ていくように、接続ごとにプロセスを生成するのは非常にコストが大きいので、後にマルチスレッドで対応していく。

Linuxでは、新しいプロセスを `fork` を使用する。この関数は、呼び出し元と全く異なる同じプロセスを生成する。呼び出し元のプロセスを「親プロセス」、生成したプロセスを「子プロセス」という。

## マルチプロセスエコーサーバーの方法

```text
親プロセス (forループ)
 ├── クライアント A 接続 → fork() → 子プロセスA (クライアント処理)
 ├── クライアント B 接続 → fork() → 子プロセスB (クライアント処理)
 ├── クライアント C 接続 → fork() → 子プロセスC (クライアント処理)
    ... (繰り返し)
```

## 重要関数

#### fork

```c
pid_t fork(void)
```

`fork()` は 親プロセスをコピーして新しい子プロセスを作成 する。 `fork()` が成功すると、親プロセスと子プロセスの2つのプロセスが実行される。 `processID == 0` のときは 子プロセス なので、HandleTCPClient(clntSock) でクライアントの処理を実行。

```c
if ((processID = fork()) < 0)
{
    DieWithError("fork() failed");
}
else if (processID == 0) // 子プロセス
{
    close(servSock);  // 親プロセスのリスニングソケットは不要
    HandleTCPClient(clntSock); // クライアントを処理
    exit(0); // 子プロセス終了
}
```

#### waitpid

```c
pid_t waitpid(pid_t pid, int *status, int options);
```

`waitpid` はプロセスの状態を呼び出す関数である。 `waitpid((pid_t)-1, NULL, WNOHANG)` を呼び出すと、終了した子プロセスがあれば回収する。
- `-1` : 任意の子プロセス
- `WNOHANG` : 終了した子プロセスがなければすぐに戻る
  
`processID == 0` なら ゾンビプロセスはない のでループを抜ける。


## コード
```c
#include "TCPEchoServer.h"
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    int servSock;                    /* サーバーのソケットディスクリプタ */
    int clntSock;                    /* クライアントのソケットディスクリプタ */
    unsigned short echoServPort;     /* サーバーのポート */
    pid_t processID;                 /* プロセスID */
    unsigned int childProcCount = 0; /* 子プロセスの数 */

    /* 引数をチェック */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }
    echoServPort = atoi(argv[1]); /* 1つ目の引数: ポート */

    /* サーバーのソケットを作成 */
    servSock = CreateTCPClientSocket(echoServPort);

    for (;;)
    {
        /* クライアントの接続を待機 */
        clntSock = AcceptTCPConnection(servSock);

        /* プロセスをフォーク */
        if ((processID = fork()) < 0)
        {
            /* fork() に失敗 */
            DieWithError("fork() failed");
        }
        else if (processID == 0)
        {
            /* 子プロセス
             * クライアントのソケットをクローズし、
             * クライアントとの通信を処理 */
            close(servSock);
            HandleTCPClient(clntSock);
            exit(0);
        }

        printf("with child process: %d\n", processID);
        close(clntSock);  /* 親プロセスはクライアントのソケットをクローズ */
        childProcCount++; /* 子プロセスの数をインクリメント */

        /* ゾンビプロセスの処理 */
        while (childProcCount)
        {
            /* ゾンビプロセスの処理 */
            processID = waitpid((pid_t)-1, NULL, WNOHANG);
            if (processID < 0)
            {
                DieWithError("waitpid() failed");
            }
            else if (processID == 0)
            {
                break;
            }
            else
            {
                childProcCount--; /* 子プロセスの数をデクリメント */
            }
        }
    }
}
```