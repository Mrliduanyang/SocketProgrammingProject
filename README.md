## 计算机网络第二次作业程序文档

### 问题描述：

许多与网络相关的应用程序需要快速识别一对节点之间的最短路径，以优化路由性能。给定一个由一组顶点𝑉和一组边组成的加权图𝐺（𝑉，𝐸），我们的目的是在𝐺中找到连接源顶点𝑣1和目标顶点𝑣𝑛的最短路径。
在此项目中，将实现一个分布式系统，以根据客户的查询来计算最短路径。假设系统存储了一个城市的地图，并且客户希望获得最短路径和城市中两点之间的相应传输延迟。下图总结了系统架构。分布式系统由三个计算节点组成：主服务器（AWS），连接到两个后端服务器（服务器A和服务器B）。在后端服务器A上，有一个名为map.txt的文件，用于存储城市的地图信息。 AWS服务器与客户端连接以接收其查询并返回计算出的答案。后端服务器A和B根据AWS服务器转发的消息执行实际的最短路径和传输延迟计算。

### 系统流程：

1. [通信]Client-> AWS：客户端通过TCP将地图ID，地图中的源节点和传输文件大小（单位：位）发送到AWS；
2. [通信] AWS-> ServerA：AWS通过UDP将地图ID和源节点转发到ServerA；
3. [计算] ServerA从map.txt中读取地图信息，使用Dijkstra查找从输入源到所有其他节点的最短路径，并以预定义的格式打印出来；
4. [通信] ServerA-> AWS：ServerA将Dijkstra的输出发送到AWS；
5. [通信] AWS-> ServerB：AWS将文件大小以及ServerA的输出发送到ServerB；
6. [计算] ServerB计算每条路径的传输延迟，传播延迟和端到端延迟；
7. [通信] ServerB-> AWS：ServerB将计算出的延迟值发送到AWS；
8. [通信] AWS->客户端：AWS向客户端发送最短路径和延迟结果，然后客户端打印最终结果；

### 城市地图数据格式：

该城市的地图信息存储在ServerA中名为map.txt的文件中。map.txt文件包含多张地图的信息，其中每张地图都可以视为城市的社区。 在每个地图中，进一步指定边缘和顶点信息，其中边缘代表通信链接。假设属于同一地图的边缘具有相同的传播速度和传输速度。
map.txt的格式定义如下：

```
<Map ID 1>
<Propagation speed>
<Transmission speed>
<Vertex index for one end of the edge> <Vertex index for the other end> <Distance between the two vertices>
… (Specification for other edges)
<Map ID 2>
```

### 端口号分配：

| 进程    | 动态端口 | 静态端口                              |
| ------- | -------- | ------------------------------------- |
| ServerA | -        | 1 UDP，21223                          |
| ServerB | -        | 1 UDP，22223                          |
| AWS     | -        | 1 UDP，23223；1 TCP with Client 24223 |
| Client  | 1 TCP    | -                                     |

### 文件描述：

1. client.cpp：Client代码。该程序首先从命令行中获取用户输入的map id，source node，file size。然后创建和aws之间的TCP连接，连接建立完成后，程序将三个输入数据封装后发送给AWS，之后等待AWS返回该map中从source node到其他各节点的最小延迟数据。
2. aws.cpp：AWS代码。该程序首先建立TCP socket，侦听24233端口，以接收Client发送过来的查询请求。AWS接收到Client的查询请求后，建立和ServerA的UDP连接，将查询请求中的map id和source node数据发送给ServerA，之后等待接收ServerA返回的从source node到其他各节点的最短路径数据。接收到最短路径数据后，AWS建立和ServerB的UDP连接，将Client查询请求中的file size数据和最短路径数据发送给ServerB，之后等待接收ServerB返回的从source node到其他各节点的最小延迟数据。接收到ServerB返回的延迟结果后，将该结果返回给Client。
3. serverA.cpp：ServerA代码。该程序首先建立UDP socket，侦听21233端口，以便接收AWS发送过来的数据。之后，根据map.txt文件提取propagation speed，transmission speed，并开始构建无向图。ServerA接收到AWS发送过来的map id 和 source node数据后，使用Dijkstra算法，查找map id 对应map中从source node到其他各节点的最短路径。计算完成后，将propagation speed，transmission speed，最短路径结果返回给AWS。
4. serverB.cpp：ServerB代码。该程序首先建立UDP socket，侦听22233端口，以便接收AWS发送过来的数据。ServerB接收到从AWS发送过来的propagation speed，transmission speed和最短路径数据，计算各路径对应的transmission delay，propagation delay，end to end delay。计算完成后，将transmission delay，propagation delay and，to end delay返回给AWS。

### 信息交换格式：

- client -> aws:{map id,source node,file size}
- aws -> client:[[destination,min length,transmission delay,propagation delay,end to end delay]]
- aws -> serverA:{map id,source node}
- serverA -> aws:{propagation speed,transmission speed,[[destination,min length]]}
- aws -> serverB:{propagation speed,transmission speed,file size,[[destination,min length]]}
- serverB -> aws:[[destination,transmission delay,propagation delay,end to end delay]]

### 编译运行：

| 功能        | 命令                                                      |
| ----------- | --------------------------------------------------------- |
| 编译源文件  | make all                                                  |
| 运行AWS     | make aws                                                  |
| 运行ServerA | make serverA                                              |
| 运行ServerB | make serverB                                              |
| 客户端查询  | ./client \<Map ID\> \<Source Vertex Index\> \<File Size\> |

