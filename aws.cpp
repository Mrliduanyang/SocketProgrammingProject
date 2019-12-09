#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <iomanip>

using namespace std;

#define AWS_TCP_PORT 24233
#define AWS_UDP_PORT 23233
#define SERVER_A_UDP_PORT 21233
#define SERVER_B_UDP_PORT 22233
#define HOST "127.0.0.1"

// define receive buffer for client
struct RecvBufForClient {
    char mapID[20];
    long sourceNode;
    long fileSize;
};

// define send buffer for client
struct SendBufForClient {
    double result[10][5];
};

// define receive buffer for serverA
struct RecvBufForServerA {
    double propSpeed;
    double transSpeed;
    long shortestPath[10][2];
};

// define receive buffer for serverB
struct RecvBufForServerB {
    double delay[10][4];
};

// define send buffer for serverA
struct SendBufForServerA {
    char mapID[20];
    long sourceNode;
};

// define send buffer for serverB
struct SendBufForServerB {
    double propSpeed;
    double transSpeed;
    long fileSize;
    long shortestPath[10][2];
};

int main() {
    cout << "The AWS is up and running." << endl;
    // init send and recv buffer for client,serverA,serverB
    struct RecvBufForClient recvBufForClient;
    struct SendBufForClient sendBufForClient;
    struct RecvBufForServerA recvBufForServerA;
    struct RecvBufForServerB recvBufForServerB;
    struct SendBufForServerA sendBufForServerA;
    struct SendBufForServerB sendBufForServerB;

    // init socket for aws to client
    int awsClientSocket;
    if ((awsClientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }
    // bind port to socket
    struct sockaddr_in awsTcpAddr, awsUdpAddr, clientTcpAddr, serverAUdpAddr, serverBUdpAddr;
    memset(&awsTcpAddr, 0, sizeof(awsTcpAddr));
    awsTcpAddr.sin_family = AF_INET;
    awsTcpAddr.sin_port = htons(AWS_TCP_PORT);
    awsTcpAddr.sin_addr.s_addr = inet_addr(HOST);
    if (bind(awsClientSocket, (struct sockaddr *) &awsTcpAddr, sizeof(awsTcpAddr)) < 0) {
        exit(EXIT_FAILURE);
    }

    memset(&awsUdpAddr, 0, sizeof(awsUdpAddr));
    memset(&serverAUdpAddr, 0, sizeof(serverAUdpAddr));
    memset(&serverBUdpAddr, 0, sizeof(serverBUdpAddr));
    awsUdpAddr.sin_family = AF_INET;
    awsUdpAddr.sin_port = htons(AWS_UDP_PORT);
    awsUdpAddr.sin_addr.s_addr = inet_addr(HOST);

    serverAUdpAddr.sin_family = AF_INET;
    serverAUdpAddr.sin_port = htons(SERVER_A_UDP_PORT);
    serverAUdpAddr.sin_addr.s_addr = inet_addr(HOST);

    serverBUdpAddr.sin_family = AF_INET;
    serverBUdpAddr.sin_port = htons(SERVER_B_UDP_PORT);
    serverBUdpAddr.sin_addr.s_addr = inet_addr(HOST);

    // listen tcp socket
    if (listen(awsClientSocket, 5) < 0) {
        exit(EXIT_FAILURE);
    }

    // start listening service
    int awsClientConn;
    while (true) {
        // init socket for aws to serverA,aws to serverB
        int awsServerASocket;
        int awsServerBSocket;
        if ((awsServerASocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            exit(EXIT_FAILURE);
        }
        if ((awsServerBSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            exit(EXIT_FAILURE);
        }
        // accept client's tcp connection
        socklen_t clientTcpAddrLen = sizeof(clientTcpAddr);
        if ((awsClientConn = accept(awsClientSocket, (struct sockaddr *) &clientTcpAddr, &clientTcpAddrLen)) < 0) {
            exit(EXIT_FAILURE);
        }
        // receive client's send data
        recv(awsClientConn, &recvBufForClient, sizeof(recvBufForClient), 0);
        cout << "The AWS has received map ID <" << recvBufForClient.mapID
             << ">, start vertex <" << recvBufForClient.sourceNode
             << "> and file size <" << fixed << recvBufForClient.fileSize
             << "> from the client using TCP over port <" << ntohs(clientTcpAddr.sin_port) << ">" << endl;

        // pass client's send data to send buffer of serverA
        strcpy(sendBufForServerA.mapID, recvBufForClient.mapID);
        sendBufForServerA.sourceNode = recvBufForClient.sourceNode;
        // establish a udp connection with serverA
        socklen_t serverAUdpAddrLen = sizeof(serverAUdpAddr);
        if (bind(awsServerASocket, (struct sockaddr *) &awsUdpAddr, sizeof(awsUdpAddr)) < 0) {
            exit(EXIT_FAILURE);
        }
        // send data to serverA
        sendto(awsServerASocket, &sendBufForServerA, sizeof(sendBufForServerA), 0,
               (const struct sockaddr *) &serverAUdpAddr, serverAUdpAddrLen);
        cout << "The AWS has sent map ID and starting vertex to server A using UDP over port <"
             << AWS_UDP_PORT << ">" << endl;
        // receive shortest path from serverA
        memset(recvBufForServerA.shortestPath, 0, sizeof(recvBufForServerA.shortestPath));
        recvfrom(awsServerASocket, &recvBufForServerA, sizeof(recvBufForServerA), 0, (
                struct sockaddr *) &serverAUdpAddr, &serverAUdpAddrLen);
        cout << "The AWS has received shortest path from server A:" << endl
             << "-----------------------------" << endl
             << "Destination        Min Length" << endl
             << "-----------------------------" << endl;
        for (auto item: recvBufForServerA.shortestPath) {
            if (item[1] != 0) {
                cout << left << setw(19) << item[0] << setw(10) << item[1] << endl;
            }
        }
        close(awsServerASocket);

        // pass shortestPath, propSpeed, transSpeed, fileSize to send buffer of serverB
        memcpy(sendBufForServerB.shortestPath, recvBufForServerA.shortestPath, sizeof(recvBufForServerA.shortestPath));
        sendBufForServerB.propSpeed = recvBufForServerA.propSpeed;
        sendBufForServerB.transSpeed = recvBufForServerA.transSpeed;
        sendBufForServerB.fileSize = recvBufForClient.fileSize;

        // establish a udp connection with serverB
        socklen_t serverBUdpAddrLen = sizeof(serverBUdpAddr);
        if (bind(awsServerBSocket, (struct sockaddr *) &awsUdpAddr, sizeof(awsUdpAddr)) < 0) {
            exit(EXIT_FAILURE);
        }
        // send data to serverB
        sendto(awsServerBSocket, &sendBufForServerB, sizeof(sendBufForServerB), 0,
               (const struct sockaddr *) &serverBUdpAddr, serverBUdpAddrLen);
        cout
                << "The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <"
                << AWS_UDP_PORT << ">" << endl;

        // receive delay from serverB
        memset(recvBufForServerB.delay, 0, sizeof(recvBufForServerB.delay));
        recvfrom(awsServerBSocket, &recvBufForServerB, sizeof(recvBufForServerB), 0, (
                struct sockaddr *) &serverBUdpAddr, &serverBUdpAddrLen);
        cout << "The AWS has received delays from server B:" << endl
             << "--------------------------------------------" << endl
             << "Destination        Tt        Tp        Delay" << endl
             << "--------------------------------------------" << endl;
        for (auto item:recvBufForServerB.delay) {
            if (item[0] != 0) {
                cout << left << setw(19) << (long) item[0]
                     << setw(10) << setprecision(2) << item[1]
                     << setw(10) << setprecision(2) << item[2]
                     << setw(5) << setprecision(2) << item[3] << endl;
            }
        }
        close(awsServerBSocket);

        // pass shortestPath, propSpeed, transSpeed, fileSize to send buffer of serverB
        long resultIndex = 0;
        memset(sendBufForClient.result, 0, sizeof(sendBufForClient.result));
        for (auto item:recvBufForServerB.delay) {
            if (item[0] != 0) {
                sendBufForClient.result[resultIndex][0] = item[0];
                sendBufForClient.result[resultIndex][1] = recvBufForServerA.shortestPath[resultIndex][1];
                sendBufForClient.result[resultIndex][2] = item[1];
                sendBufForClient.result[resultIndex][3] = item[2];
                sendBufForClient.result[resultIndex][4] = item[3];
                resultIndex += 1;
            }
        }
        cout << "--------------------------------------------" << endl;
        // send data to client
        send(awsClientConn, &sendBufForClient, sizeof(sendBufForClient), 0);
        cout << "The AWS has sent calculated delay to client using TCP over port <" << AWS_TCP_PORT
             << ">." << endl;
        close(awsClientConn);
//        close(awsClientSocket);
    }
    return 0;
}

