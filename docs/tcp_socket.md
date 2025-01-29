# TCPソケット

## 作成と破棄

TCPまたはUDPを使って通信を行うには、プログラムの最初の部分でソケットの作成をOSに依頼する必要がある。

```c
int socket(int protocolFamily, int type, int protocol);
```

- `int protocolFamily` : 要求するアドレス・ドメイン。AF_INET、AF_INET6、 AF_UNIX、AF_RAW のいずれか
- `int type` : 作成するソケットのタイプ。SOCK_STREAM、SOCK_DGRAM、または SOCK_RAW のいずれか
- `int protocol` : 要求済みプロトコル。 可能な値は、0、IPPROTO_UDP、または IPPROTO_TCP 
- 戻り値 : 成功時には負ではない値を、失敗時には-1、-1以外のあたいは、そのつど与えられる数値であり、ファイルディスクリプタと同等に扱われる。この数値は「ソケットディスクリプタ」と呼ばれ、ソケットAPI関数で処理されるソケットの識別に使用される。

## アドレスの指定

ソケットを使うアプリケーションでは、IPアドレスとポート番号をOSカーネルに知らせる必要がある。例えばクライアントでは、通信相手であるサーバアプリケーションのIPアドレスとを知らせなければならない。加えて、ソケットがアプリケーションプログラムに対してアドレスを通知しなければならない。サーバでは、この情報に基づいて、個々のクライアントのIPアドレスを識別する。

```c
struct sockaddr
{
    unsigned short sa_family;   /* アドレスファミリ */
    char sa_data[14];           /* アドレス情報 */
};
```

`sa_family` はアドレスファミリを表すあたいであり、基本的にはAF_INET、AF_INET6、 AF_UNIX、AF_RAWといった定数が入る。 `sa_data` はアドレス情報を表すビット列である。


TCPに特化した `sockaddr` 構造体には、 `sockaddr_in` 構造体が用意されている。

```c
struct in_addr
{
    unsigned long s_addr;       /* IPアドレス（32ビット） */
};

struct sockaddr_in
{
    unsigned short sin_family;  /* TCP/IP（AF_INET） */
    unsigned short sin_port;    /* アドレスポート */
    struct in_addr sin_addr;    /* IPアドレス（32ビット） */
    char sin_zero[8];           /* 不使用 */
};
```

## TCPエコークライアント

クライアントの仕事は、接続を受動的に待機するサーバーとの通信を開始することである。TCPクライアントの処理の流れは以下の通りである。

1. socket()でTCPソケットを作成する
2. connect()でサーバーへの接続を確立する
3. send()とrecv()を実行して通信を行う
4. close()で接続をクローズする

クライアントとサーバの大きな違いは、接続を確立するための手段である。クライアントは自律的に接続を開始できるが、サーバーは接続を待っているだけである。サーバーとの接続を確立するには

```c
int connect(int socket, struct sockaddr *foreignAddress, unsigned int addressLenght);
```

- socket : ソケットディスクリプタ
- foreignAddress : ソケットAPIが汎用であるため、sockaddrへのポインタ。TCPアドレス構造体はsockaddr_inへのポインタである
- addressLenght : 基本的には `sizeof(foreignAddress)` で良い

その他にも 

```c
int send(int socket, const void *msg, unsigned int msgLength, int flags);
int recv(int socket, void *rcvBuffer, unsigned int bufferLength, int flags);
```

send()では送信するメッセージへのポインタをmsgに、メッセージの長さは `msgLength` に、それぞれ指定する。recv()では、rcvBufferに受診データを格納するバッファへのポインタを、bufferLengthにそのバッファを指定する。

```c
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 受信バッファサイズ */
#define RCVBUFSIZE 32

/* エラー処理関数 */
void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    /* パラメータを解析する */
    int sock;                        /* ソケットディスクリプタ */
    struct sockaddr_in echoServAddr; /* エコーサーバのアドレス */
    unsigned short echoServPort;     /* エコーサーバのポート */
    char *servIP;                    /* サーバのIPアドレス */
    char *echoString;                /* エコーサーバに送信する文字列 */
    char echoBuffer[RCVBUFSIZE];     /* エコーサーバから受信するデータ */
    unsigned int echoStringLen;      /* エコーサーバに送信する文字列の長さ */
    int bytesRcvd, totalBytesRcvd;   /* エコーサーバから受信したバイト数 */

    /* -------------------- */
    /* 引数の数が正しいか確認 */
    /* -------------------- */
    if ((argc < 3) || (argc > 4))
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];     /* １つ目の引数：サーバのIPアドレス */
    echoString = argv[2]; /* ２つ目の引数：エコーサーバに送信する文字列 */

    if (argc == 4)
    {
        /* エコーサーバのポートを指定するかどうかを確認 */
        echoServPort = atoi(argv[3]);
    }
    else
    {
        echoServPort = 7;
    }

    /* -------------------- */
    /* ソケットを作成する     */
    /* -------------------- */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* ---------------------------------------- */ 
    /* エコーサーバーのアドレス構造体を初期化する   */
    /* ---------------------------------------- */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 構造体をゼロで初期化 */
    echoServAddr.sin_family = AF_INET;                /* インターネットアドレスファミリ */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* サーバのIPアドレス */
    echoServAddr.sin_port = htons(echoServPort);      /* サーバのポート */

    /* -------------------- */
    /* サーバーに接続確立    */
    /* -------------------- */
    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("connect() failed");
    }

    /* -------------------- */
    /* 文字列を送信する      */
    /* -------------------- */
    echoStringLen = strlen(echoString);

    /* データを送信 */
    if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
    {
        DieWithError("send() sent a different number of bytes than expected");
    }

    /* ------------------------ */
    /* サーバーからエコーを受信   */
    /* ------------------------ */
    totalBytesRcvd = 0;
    printf("Received: ");
    while (totalBytesRcvd < echoStringLen)
    {
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        {
            DieWithError("recv() failed or connection closed prematurely");
        }
        totalBytesRcvd += bytesRcvd;  /* 受信したバイト数を加算 */
        echoBuffer[bytesRcvd] = '\0'; /* 文字列の終端を追加 */
        printf("%s", echoBuffer);     /* 受信した文字列を表示 */
    }

    printf("\n");

    close(sock); /* ソケットをクローズ */
    return 0;
}
```

## TCPエコーサーバー

TCPサーバーの処理は以下の通りである。

1. socket()で、TCPソケットを作成する
2. bind()で、ソケットにポート番号を割り当てる
3. listen()で、割り当てたポート番号へ接続を作成できる事をシステムに伝える
4. 以下の手順に繰り返し実行する
   - クライアントから接続要求を受けるたびに、accept()を呼び出してソケットを新しく取得する
   - send()とrecv()を実行しながら、作成したソケットを介してクライアントとやり取りする
   - close()を実行してクライアントとの接続をクローズする

クライアントと異なるのは、ソケットにアドレスをバインドし、そのソケットをチャンネルとして使用して、もう一方の（クライアントに接続されている）ソケットを「受信」するという点である。

```c
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 未処理の接続要求の最大数 */
#define MAXPENDING 5
/* 受信バッファサイズ */
#define RCVBUFSIZE 32

/* エラー処理関数 */
void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE]; /* エコーバッファ */
    int recvMsgSize;             /* 受信メッセージのサイズ */

    /* クライアントからのメッセージを受信 */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
    {
        DieWithError("recv() failed");
    }

    /* 受信したメッセージをクライアントにエコーバック */
    while (recvMsgSize > 0)
    {
        /* クライアントにメッセージを送信 */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
        {
            DieWithError("send() failed");
        }

        /* クライアントからのメッセージを受信 */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        {
            DieWithError("recv() failed");
        }
    }

    close(clntSocket); /* クライアントのソケットをクローズ */
}

int main(int argc, char *argv[])
{
    int servSock;                    /* サーバのソケットディスクリプタ */
    int clntSock;                    /* クライアントのソケットディスクリプタ */
    struct sockaddr_in echoServAddr; /* エコーサーバのアドレス */
    struct sockaddr_in echoClntAddr; /* クライアントのアドレス */
    unsigned short echoServPort;     /* エコーサーバのポート */
    unsigned int clntLen;            /* クライアントのアドレス構造体の長さ */

    /* ----------------------------------------- */
    /* 引数の数が正しいか確認                      */
    /* ----------------------------------------- */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }
    echoServPort = atoi(argv[1]);    /* エコーサーバのポートを指定 */

    /* ----------------------------------------- */
    /* ソケットを作成する                          */
    /* ----------------------------------------- */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* ----------------------------- */
    /* サーバのアドレス構造体を作成    */
    /* ----------------------------- */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 構造体をゼロで初期化 */
    echoServAddr.sin_family = AF_INET;                /* インターネットアドレスファミリ */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 任意のIPアドレス */
    echoServAddr.sin_port = htons(echoServPort);      /* サーバのポート */

    /* ----------------------------------------- */
    /* サーバのアドレス構造体にソケットをバインド    */
    /* ----------------------------------------- */
    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("bind() failed");
    }

    /* ----------------------------------------- */
    /* クライアントからの接続要求を待機             */
    /* ----------------------------------------- */
    if (listen(servSock, MAXPENDING) < 0)
    {
        DieWithError("listen() failed");
    }

    /* ----------------------------------------- */
    /* 接続要求を繰り返し処理する                   */
    /* ----------------------------------------- */
    for (;;)
    {
        /* クライアントのアドレス構造体の長さを初期化 */
        clntLen = sizeof(echoClntAddr);

        /* クライアントからの接続要求を受け入れ */
        if ((clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) < 0)
        {
            DieWithError("accept() failed");
        }

        /* クライアントの処理を行う */
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        HandleTCPClient(clntSock);
    }
    return 0;
}
```