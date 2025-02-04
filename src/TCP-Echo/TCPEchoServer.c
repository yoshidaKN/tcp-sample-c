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

    /* 引数の数が正しいか確認 */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }

    /* エコーサーバのポートを指定 */
    echoServPort = atoi(argv[1]);

    /* ソケットの作成 
        * PF_INET: インターネットアドレスファミリ
        * SOCK_STREAM: ストリームソケット
        * IPPROTO_TCP: TCPトランスポートプロトコル
    */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* サーバのアドレス構造体を作成 
        htonl: ホストバイト順の IPv4 アドレスをネットワークバイト順の IPv4 アドレスに変換
        htons: ホストのバイト順の IP ポート番号をネットワーク バイト順の IP ポート番号に変換  */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 構造体をゼロで初期化 */
    echoServAddr.sin_family = AF_INET;                /* インターネットアドレスファミリ */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 任意のIPアドレス */
    echoServAddr.sin_port = htons(echoServPort);      /* サーバのポート */

    /* サーバのアドレス構造体にソケットをバインド */
    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("bind() failed");
    }

    /* クライアントからの接続要求を待機 */
    if (listen(servSock, MAXPENDING) < 0)
    {
        DieWithError("listen() failed");
    }

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
