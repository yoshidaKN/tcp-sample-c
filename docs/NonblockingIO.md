# ノンブロッキングI/O

## ソケットオプション

TCP/IPプロトコルの開発では、できるだけ多くのアプリケーションにとって都合が良くなるなるようにデフォルトの動作が決められている。多くのアプリケーションで、デフォルト動作で満足する動作ができるが、時には調節が必要となる。例えば、各ソケットには受信バッファが関連付けられているが、そのバッファサイズがどれくらいであれば足りるだろうか。どのソケットもデフォルト値が決まっているが、アプリケーションによってはその値が適切でない場合もあるだろう。

受信バッファサイズなど、ソケットの動作を決める様々な特性は、ソケットオプションの値を変更することで調節できる。

```c
int getsockopt(int socket, int level, int optName, void *optVal, unsigned int *optLen);
int setsockopt(int socket, int level, int optName, const void *optVal, unsigned int optLen);
```

`getsockopt()` 関数は指定されたソケットディスクリプタに関連したオプションの取得できる。用意されているソケットオプションは、プロトコルスタックのレイヤに対応する形でレベルに分類されている。

- socket : ソケットディスクリプタ
- level : TCPのレベルに該当する定数。ソケット・レベルでオプションを操作するには、SOL_SOCKET。IPv4 または IPv6 レベルでオプションを操作するには、IPPROTO_IP。その他のレベル (TCP レベルなど) でオプションを操作するには、オプションを制御するプロトコルに該当するプロトコル番号を指定する。
- optName : オプションの種類を指定する。
- optVal : getsockoptでは、optNameのオプションの値が格納される。setsockoptでは、optNameのオプション値をセットする。

```c
    /* デフォルトのバッファサイズを取得し、表示する */
    sockOptSize = sizeof(rcvBufferSize);
    if (getsockopt(0, SOL_SOCKET, SO_RCVBUF, &rcvBufferSize, &sockOptSize) < 0)
    {
        perror("getsockopt");
        return 1;
    }

    /* バッファサイズを2倍にする */
    rcvBufferSize *= 2;
    if (setsockopt(0, SOL_SOCKET, SO_RCVBUF, &rcvBufferSize, sizeof(rcvBufferSize)) < 0)
    {
        perror("setsockopt");
        return 1;
    }
```

## シグナル

シグナルとは、ユーザーによる割り込み文字の入力やタイマー切れといった、特定のイベントの発生をプログラムに伝えるための仕組みである。発生したイベントは、その時のコードどの部分が実行中であるかに関わらず、シグナルによって非同期でプログラムに通知される。

1. シグナルは無視される。プロセスはシグナルが送信されたことを感知しない。
2. プログラムがOSによって強制終了される
3. プログラムで指定されたシグナル処理関数が実行される。この関数は、プログラムのメインスレッドから呼び出された別の制御スレッドで実行されるため、プログラムがシグナルを直ちに感知するとは限らない。
4. シグナルがブロックされる。つまり、プログラムがシグナルの受信が可能にするためのアクションを起こすまで、何も影響受けないようにする。各プロセスでどのシグナルをブロックするかは、シグナルマスク（どのシグナルをブロックするかを記述したフラグの集合）によって決められる。

UNIXには、各種イベントの発生を知らせることが出来るよう、様々なシグナルが用意されている。いかに基本的なものだけを取り出す。

| シグナル | トリガーイベント                 | デフォルト動作 |
| :------- | :------------------------------- | :------------- |
| SIGALRM  | アラーム・タイマーの終了         | 終了           |
| SIGCHLD  | 子プロセスの終了                 | 無視           |
| SIGINT   | 割り込み文字（Ctrl+C）の入力     | 終了           |
| SIGIO    | ソケットのI/O準備完了            | 無視           |
| SIGPIPE  | クローズしたソケットへの書き込み | 終了           |

アプリケーションプログラムにおいてシグナルのデフォルト動作を変更するには、sigaction()を実行する。

```c
int sigaction(int whichSignal, const struct sigaction *newAction, struct sigaction *oldAction);
```

例によって成功時は0, 失敗時は-1を返す。

```c
struct sigaction {
    void (*sa_handler) (int);   /* シグナルハンドラ */
    sigset_t sa_mask;           /* ハンドラ実行中にブロックされるシグナル */
    int sa_flags;               /* デフォルト動作を変更するためのフラグ */
}
```

sigaction()を使った簡単なプログラムの例をいかに示す。

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/* エラー処理関数 */
void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void InterruptSignalHandler(int signalType)
{
    printf("Interrupt Received. Exiting program.\n");
    exit(1);
}

int main(int argc, char const *argv[])
{
    struct sigaction handler; /* シグナルハンドラ */

    /* InterruptSignalHandler()をハンドラ関数として設定 */
    handler.sa_handler = InterruptSignalHandler;
    
    /* int sigfillset(sigset_t *set) 全フラグをセットする */
    /* 全シグナルをマスクするマスクを作成する 
        → 全てのシグナルをブロックする　*/
    if (sigfillset(&handler.sa_mask) < 0)
    {
        DieWithError("sigfillset() failed");
    }

    /* 割り込みシグナルに対する処理を設定 
        →　このシグナルだけを受け入れる */
    if (sigaction(SIGINT, &handler, 0) < 0)
    {
        DieWithError("sigaction() failed");
    }

    for (;;)
    {
        pause(); /* プログラムが終了するまで待機 */
    }

    return 0;
}
```

