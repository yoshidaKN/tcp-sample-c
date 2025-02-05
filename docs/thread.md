# Threads

スレッド（Thread）とは、プロセス内で独立して実行される処理の単位のことである。通常、1つのプログラム（プロセス）は 1つのスレッド（メインスレッド） で実行されるが、複数のスレッドを作成 すれば、1つのプロセス内で複数の処理を並行して実行できる。

## 重要関数

### `malloc`

```c
struct ThreadsArgs *threadArgs;
if ((threadArgs = (struct ThreadsArgs *)malloc(sizeof(struct ThreadsArgs))) == NULL)
{
    DieWithError("malloc() failed");
}
threadArgs->clntSock = clntSock;
```

- スレッドを作るときに、関数引数として渡せるのはポインタだけであり、スレッドごとに独立したデータを保持するためには、データを動的に確保する必要がある。
- 関数のローカル変数を渡すと、スレッドが並列で動作するときにデータが破壊される。 `threadArgs` を ローカル変数 にすると、ループの次回実行時に 前のデータが上書きされる可能性がある。 `malloc()` を使ってヒープ領域にメモリを確保すれば、スレッドが処理する間 データが保持される。
- スレッドが処理を終えた後に `free(threadArgs);` で開放する

### `pthread_create()`

```c
if ((pthread_create(&threadID, NULL, ThreadMain, (void *)threadArgs)) != 0)
{
    DieWithError("pthread_create() failed");
}
```

`pthread_create()` の定義は以下の通り。

```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
```

- 引数
  - `pthread_t *thread` : スレッドIDを格納する変数（作成されたスレッドの識別子）
  - `const pthread_attr_t *attr` : スレッドの属性（NULL でデフォルト設定）
  - `void *(*start_routine) (void *)` : スレッドで実行する関数（ここでは `ThreadMain` ）
  - `void *arg` : `start_routine` に渡す引数（ここでは `threadArgs` ）
- 役割
  - `pthread_create()` を呼ぶと 新しいスレッドが作成され、`ThreadMain(threadArgs)` が実行される。
  - `threadArgs` は `void *` 型にキャストされ、新しいスレッドに渡される
  - `ThreadMain()` の中で `clntSock` を取り出し、クライアントとの通信を処理する。

### ThreadMain() の役割

```c
pthread_detach(pthread_self());
```

- pthread_self() は 現在のスレッドIDを取得。
- pthread_detach() で スレッド終了時にリソースを自動解放 する。

```c
clntSock = ((struct ThreadsArgs *)threadArgs)->clntSock;
```

`threadArgs` は `void *` なので、元の `struct ThreadsArgs *` にキャスト して `clntSock` を取得。


## コード

```c
#include "TCPEchoServer.h"
#include <pthread.h>

/* メインスレッド関数 */
void *ThreadMain(void *arg);

/* クライアントスレッドに渡す構造体 */
struct ThreadsArgs
{
    int clntSock;
};

int main(int argc, char const *argv[])
{
    int servSock;                   /* サーバのソケットディスクリプタ */
    int clntSock;                   /* クライアントのソケットディスクリプタ */
    unsigned short echoServPort;    /* サーバのポート番号 */
    pthread_t threadID;             /* スレッドID */
    struct ThreadsArgs *threadArgs; /* スレッド引数 */

    /* 引数の数をチェック */
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s <Server Port: default 7>\n", argv[0]);
        exit(1);
    }
    else if (argc == 2)
    {
        echoServPort = atoi(argv[1]);
    }
    else
    {
        echoServPort = 7;
    }

    /* サーバのソケットを作成 */
    servSock = CreateTCPServerSocket(echoServPort);

    for (;;)
    {
        /* クライアントの接続を待機 */
        clntSock = AcceptTCPConnection(servSock);

        /* クライアント引数用にメモリを新しく確保 */
        if ((threadArgs = (struct ThreadsArgs *)malloc(sizeof(struct ThreadsArgs))) == NULL)
        {
            DieWithError("malloc() failed");
        }
        threadArgs->clntSock = clntSock;

        /* クライアントスレッドを生成 */
        if ((pthread_create(&threadID, NULL, ThreadMain, (void *)threadArgs)) != 0)
        {
            DieWithError("pthread_create() failed");
        }

        printf("with thread %ld\n", (long int)threadID);
    }
}

void *ThreadMain(void *threadArgs)
{
    int clntSock; /* クライアントのソケットディスクリプタ */

    /* 戻り時に、スレッドのリソースを割り当て解除 */
    pthread_detach(pthread_self());

    /* ソケットディスクリプタを引数から取り出す */
    clntSock = ((struct ThreadsArgs *)threadArgs)->clntSock;
    free(threadArgs);

    HandleTCPClient(clntSock);

    return (NULL);
}
```