#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>

#define ECHOMAX 255
#define TIMEOUT_SECS 2
#define MAXTRIES 5

void CatchAlarm(int ignored);
void DieWithError(const char *errorMessage);

/* 試行回数 */
int tries = 0;

int main(int argc, char const *argv[])
{
    int sock;                           /* ソケットディスクリプタ */
    struct sockaddr_in echoServAddr;    /* エコーサーバーのアドレス */
    struct sockaddr_in fromAddr;        /* 受信メッセージの送信元アドレス */
    unsigned short echoServPort;        /* エコーサーバーのポート */
    unsigned int fromSize;              /* 受信メッセージの送信元アドレスの長さ */
    struct sigaction handler;           /* シグナルハンドラ */
    char *servIP;                       /* サーバーのIPアドレス */
    char *echoString;                   /* エコーメッセージ */
    char echoBuffer[ECHOMAX + 1];       /* エコーメッセージの受信バッファ */
    int echoStringLen;                  /* エコーメッセージの長さ */
    int respStringLen;                  /* 受信メッセージの長さ */

    /* 引数の取り出し */
    if((argc < 3) || (argc > 4))
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Server Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];
    echoString = argv[2];

    if((echoStringLen = strlen(echoString)) > ECHOMAX)
    {
        DieWithError("Echo word too long");
    }

    if(argc == 4)
    {
        echoServPort = atoi(argv[3]);
    }
    else
    {
        echoServPort = 7;
    }

    /* UDPによるデータグラムソケットを作成する */
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* アラームシグナル用シグナルハンドラをセット */
    handler.sa_handler = CatchAlarm;
    if(sigfillset(&handler.sa_mask) < 0)
    {
        DieWithError("sigfillset() failed");
    }
    handler.sa_flags = 0;

    /* SIGALRMだけを受け入れる */
    if(sigaction(SIGALRM, &handler, 0) < 0)
    {
        DieWithError("sigaction() failed for SIGALRM");
    }

    /* サーバーのアドレス構造体を作成 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServAddr.sin_port = htons(echoServPort);

    /* エコーメッセージをサーバーに送信 */
    if(sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoStringLen)
    {
        DieWithError("sendto() sent a different number of bytes than expected");
    }

    /* 応答を受信する */
    fromSize = sizeof(fromAddr);
    alarm(TIMEOUT_SECS);    /* タイムアウト時間を設定 */
    while ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&fromAddr, &fromSize)) < 0)
    {
        /* タイムアウト */
        if(errno == EINTR)  
        {
            /* タイムアウト時間を再設定 */
            if(tries < MAXTRIES)
            {
                printf("timed out, %d more tries...\n", MAXTRIES - tries);
                if(sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!= echoStringLen)
                {
                    DieWithError("sendto() failed");
                }
                alarm(TIMEOUT_SECS);
            }
            /* タイムアウト回数を超えた */
            else
            {
                DieWithError("No Response");
            }
        }
        else
        {
            DieWithError("recvfrom() failed");
        }

        /* recvfromが何かを受信したら、タイムアウトをキャンセル */
        alarm(0);

        /* 受信した文字列を表示 */
        echoBuffer[respStringLen] = '\0';
        printf("Received: %s\n", echoBuffer);

        close(sock);
        exit(0); 
    }  

    return 0;
}


void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void CatchAlarm(int ignored)
{
    tries += 1;
}