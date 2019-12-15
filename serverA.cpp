#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <stdint.h>
#include <limits.h>
#include <algorithm>

using namespace std;

map<long, long> Dijkstra(auto matrix, long source);

#define HOST "127.0.0.1"
#define SERVER_A_UDP_PORT 21233

// define a structure to store each map, iucluding propagation speed, transmission speed and graph
struct MapDetail {
    double propSpeed;
    double transSpeed;
    vector<vector<long> > graph;
};

// define receive buffer for aws
struct RecvBufForAws {
    char mapID[20];
    long sourceNode;
};

// define send buffer for aws
struct SendBufForAws {
    double propSpeed;
    double transSpeed;
    long shortestPath[10][2];
};

int main() {
    // init send and recv buffer for aws
    struct RecvBufForAws recvBufForAws;
    struct SendBufForAws sendBufForAws;

    // init socket for serverA to aws
    int serverASocket;
    if ((serverASocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }

    // bind port to socket
    struct sockaddr_in serverAUdpAddr, awsUdpAddr;
    memset(&serverAUdpAddr, 0, sizeof(serverAUdpAddr));
    memset(&awsUdpAddr, 0, sizeof(awsUdpAddr));
    serverAUdpAddr.sin_family = AF_INET;
    serverAUdpAddr.sin_port = htons(SERVER_A_UDP_PORT);
    serverAUdpAddr.sin_addr.s_addr = inet_addr(HOST);
    // establish a udp connection with aws
    if (bind(serverASocket, (struct sockaddr *) &serverAUdpAddr, sizeof(serverAUdpAddr)) < 0) {
        exit(EXIT_FAILURE);
    }
    cout << "The Server A is up and running using UDP on port <" << SERVER_A_UDP_PORT << ">." << endl;

    // use map to store each graph, the key is mapID, and the value is a structure, including propSpeed, transSpeed and graph
    map<string, MapDetail> cityMap;
    ifstream readFile("./map.txt");
    string line;
    long lineFlag = 0;
    string mapID;
    double propSpeed;
    double transSpeed;
    vector<vector<long> > graph;

    // read map.txt file and extract propSpeed,transSpeed and graph
    while (getline(readFile, line)) {
        stringstream stringIn(line);
        if (stringIn.str().find(" ") == string::npos) {
            switch (lineFlag) {
                case 0 :
                    stringIn >> mapID;
                    graph.clear();
                    break;
                case 1 :
                    stringIn >> propSpeed;
                    break;
                case 2:
                    stringIn >> transSpeed;
                    break;
            }
            lineFlag += 1;
        } else {
            lineFlag = 0;
            long num;
            vector<long> edge;
            while (stringIn >> num) {
                edge.push_back(num);
            }
            graph.push_back(edge);
        }
        if (lineFlag == 0) {
            cityMap[mapID] = MapDetail{propSpeed:propSpeed, transSpeed:transSpeed, graph:graph};
        }
    }
    readFile.close();

    // calculate edgeNum and vertexNum
    cout << "The Server A has constructed a list of <" << cityMap.size() << "> maps:" << endl
         << "-------------------------------------------" << endl
         << "Map ID  Num Vertices  Num Edges" << endl
         << "-------------------------------------------" << endl;
    for (auto item : cityMap) {
        long edgeNum = item.second.graph.size();
        vector<long> vertices;
        for (auto _item : item.second.graph) {
            vertices.push_back(_item[0]);
            vertices.push_back(_item[1]);
        }
        set<long> vertexSet(vertices.begin(), vertices.end());
        cout << left << setw(8) << item.first << setw(14) << vertexSet.size() << setw(10) << edgeNum << endl;
    }
    cout << "-------------------------------------------" << endl;

    // start listening service
    while (true) {
        // receive aws's send data
        socklen_t awsUdpAddrLen = sizeof(awsUdpAddr);
        recvfrom(serverASocket, &recvBufForAws, sizeof(recvBufForAws), 0, (
                struct sockaddr *) &awsUdpAddr, &awsUdpAddrLen);
        string recvMapID = recvBufForAws.mapID;
        long recvVertex = recvBufForAws.sourceNode;
        auto chooseMap = cityMap[recvMapID];
        cout << "The Server A has received input for finding shortest paths: starting vertex <" << recvVertex
             << "> of map <" << recvMapID
             << ">." << endl;
        // build a graphMatrix based on the selected map,
        map<long, map<long, long> > graphMatrix;
        long vertexNum = 0;
        vector<long> vertices;
        for (auto item : chooseMap.graph) {
            if (find(vertices.begin(), vertices.end(), item[0]) == vertices.end()) {
                vertices.push_back(item[0]);
                vertexNum += 1;
            }
            if (find(vertices.begin(), vertices.end(), item[1]) == vertices.end()) {
                vertices.push_back(item[1]);
                vertexNum += 1;
            }
        }
        for (auto item : chooseMap.graph) {
            graphMatrix[item[0]][item[1]] = item[2];
            graphMatrix[item[1]][item[0]] = item[2];
            graphMatrix[item[0]][item[0]] = 0;
            graphMatrix[item[1]][item[1]] = 0;
        }

        // calculate the shortest path based on graphMatrix and source
        map<long, long> shortestPath = Dijkstra(graphMatrix, recvVertex);
        cout << "The Server A has identified the following shortest paths:" << endl
             << "-----------------------------" << endl
             << "Destination  Min Length" << endl
             << "-----------------------------" << endl;
        long shortestPathIndex = 0;
        memset(sendBufForAws.shortestPath, 0, sizeof(sendBufForAws.shortestPath));
        for (auto item: shortestPath) {
            if (item.first != recvVertex) {
                sendBufForAws.shortestPath[shortestPathIndex][0] = item.first;
                sendBufForAws.shortestPath[shortestPathIndex][1] = item.second;
                cout << left << setw(13) << item.first << setw(10) << item.second << endl;
                shortestPathIndex += 1;
            }
        }
        sendBufForAws.propSpeed = chooseMap.propSpeed;
        sendBufForAws.transSpeed = chooseMap.transSpeed;

        // send shortest path to aws
        sendto(serverASocket, &sendBufForAws, sizeof(sendBufForAws), 0,
               (const struct sockaddr *) &awsUdpAddr, awsUdpAddrLen);
        cout << "-----------------------------" << endl
             << "The Server A has sent shortest paths to AWS." << endl;
    }
}

// the Dijkstra algorithm
map<long, long> Dijkstra(auto matrix, long source) {
    // store whether a node has been visited
    map<long, bool> isVisited;
    // store the distance between source and destination
    map<long, long> dist;

    isVisited[source] = true;
    // build graph based on the selected map
    for (auto item:matrix) {
        if (matrix[source].find(item.first) != matrix[source].end()) {
            dist[item.first] = matrix[source][item.first];
        } else {
            dist[item.first] = INT_MAX;
        }
    }

    // store the min length
    double minCost;
    // store the node who has the min length
    double minCostNode;

    for (auto item:matrix) {
        if (item.first != source) {
            minCost = INT_MAX;
            // find the min length from dist
            for (auto _item:dist) {
                if (!isVisited[_item.first] && dist[_item.first] < minCost) {
                    minCost = dist[_item.first];
                    minCostNode = _item.first;
                }
            }

            isVisited[minCostNode] = true;

            // update the min length between source and destination
            for (auto _item:matrix) {
                if (!isVisited[_item.first] &&
                    matrix[minCostNode].find(_item.first) != matrix[minCostNode].end() &&
                    matrix[minCostNode][_item.first] + minCost < dist[_item.first]) {
                    dist[_item.first] = matrix[minCostNode][_item.first] + minCost;
                }
            }
        }
    }
    return dist;
}
