#include "TCPEchoServer.h"

#define MAXPENDING 5   /* 待機中の接続要求の最大数 */
#define RCVBUFSIZE 256 /* 受信バッファサイズ */

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int CreateTCPServerSocket(unsigned short port)
{
    int sock;
    struct sockaddr_in echoServAddr;

    /* サーバのソケットを作成 */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* サーバのアドレス構造体を作成 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(port);

    /* サーバのアドレス構造体にソケットをバインド */
    if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("bind() failed");
    }

    /* クライアントからの接続要求を待機 */
    if (listen(sock, MAXPENDING) < 0)
    {
        DieWithError("listen() failed");
    }

    return sock;
}

int AcceptTCPConnection(int servSock)
{
    int clntSock;
    struct sockaddr_in echoClntAddr;
    unsigned int clntLen;

    /* クライアントのアドレス構造体の長さを初期化 */
    clntLen = sizeof(echoClntAddr);

    /* クライアントからの接続要求を受け入れ */
    if ((clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) < 0)
    {
        DieWithError("accept() failed");
    }

    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

    return clntSock;
}

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE]; /* エコー文字列のバッファ */
    int recvMsgSize;             /* 受信メッセージのサイズ */

    /* クライアントからのメッセージを受信 */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
    {
        DieWithError("recv() failed");
    }

    /* 受信したデータをクライアントにエコーバック */
    while (recvMsgSize > 0)
    {
        /* クライアントにデータを送信 */
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

    sleep(3); /* クライアントがデータを受信するのを待つ */

    close(clntSocket); /* クライアントのソケットをクローズ */

    printf("\tClient disconnected: %d\n", clntSocket);
}