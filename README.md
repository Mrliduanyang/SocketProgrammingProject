a. Name:Guozhen Gao

b. Student ID:6069332657

c.What I have done in this assignment?
     In this assignment, I have successfully implemented the TCP and UDP Socket simulation among a client, an AWS server and two backend servers.

d. File Description

1.client.cpp：
The program first obtains the map id, source node, and file size entered by the user from the command line.
Then create a TCP connection with aws. After the connection is established, the program encapsulates the three input data
and sends it to aws, and then waits for aws to return the minimum delay data from the source node to other nodes in the map.


2.aws.cpp：
The program first establishes a TCP socket and listens on port 24657 in order to receive query requests sent by the client.
aws receives the client's query request, establishes a UDP connection with backend server A, sends the map id and source
node data in the query request to serverA, and then waits to receive the shortest path data from source node to other
nodes returned by serverA . After receiving the shortest path data, aws establishes a UDP connection with serverB,
sends the file size data and shortest path data in the client query request to serverB, and then waits to receive
the minimum delay data from source node to other nodes returned by serverB. After receiving the delayed result returned
by serverB, return the result to the client.


3.serverA.cpp：
The program first establishes a UDP socket and listens on port 21657 in order to receive data sent by aws.
After that, extract the property speed and transmission speed according to the map.txt file, and start building the
undirected graph. After receiving the map id and source node data sent by aws, serverA uses the Dijkstra algorithm to
find the shortest path from the source node to other nodes in the map corresponding to the map id.
After the calculation is completed, return the property speed, transmission speed, and shortest path results to aws.


4.serverB.cpp：The program first establishes a UDP socket and listens to port 22657 in order to receive data sent by aws.
 ServerB receives the property transmission speed, transmission speed, and shortest path data sent from aws,
 and calculates the transmission delay, property delay, and end to end delay corresponding to each path.
 After the calculation is complete, return transmission delay, property delay and end to end delay to aws.


e. Format of exchanged information

client -> aws:{map id,source node,file size}

aws -> client:[[destination,min length,transmission delay,propagation delay,end to end delay]]

aws -> serverA:{map id,source node}

serverA -> aws:{propagation speed,transmission speed,[[destination,min length]]}

aws -> serverB:{propagation speed,transmission speed,file size,[[destination,min length]]}

serverB -> aws:[[destination,transmission delay,propagation delay,end to end delay]]

p.s.: ->  Data exchange direction,
      {}  Which values ​​the exchange data contains
     [[]] Indicates that the data is a two-dimensional array, and the content in parentheses indicates
          the data meaning of the corresponding position of the array.
client:
    The client is up and running.
    The client has sent query to AWS using TCP over port <35278>: start vertex <6>;map <A>; file size <263646634>.
    The client has received results from AWS:
    --------------------------------------------------
    Destination    Min Length    Tt      Tp      Delay
    7              6             3.74    5.08    8.83
    8              4             3.74    3.39    7.13
    9              12            3.74    10.17   13.91
    10             14            3.74    11.86   15.61
    11             17            3.74    14.41   18.15
    12             15            3.74    12.71   16.45
    13             21            3.74    17.80   21.54
    --------------------------------------------------

aws:
    The AWS is up and running.
    The AWS has received map ID <A>, start vertex <6> and file size <263646634> from the client using TCP over port <35278>
    The AWS has sent map ID and starting vertex to server A using UDP over port <23657>
    The AWS has received shortest path from server A:
    -----------------------------
    Destination        Min Length
    -----------------------------
    7                  6
    8                  4
    9                  12
    10                 14
    11                 17
    12                 15
    13                 21
    The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <23657>
    The AWS has received delays from server B:
    --------------------------------------------
    Destination        Tt        Tp        Delay
    --------------------------------------------
    7                  3.74      5.08      8.83
    8                  3.74      3.39      7.13
    9                  3.74      10.17     13.91
    10                 3.74      11.86     15.61
    11                 3.74      14.41     18.15
    12                 3.74      12.71     16.45
    13                 3.74      17.80     21.54
    --------------------------------------------
    The AWS has sent calculated delay to client using TCP over port <24657>.

serverA:
    The Server A is up and running using UDP on port <21657>.
    The Server A has constructed a list of <2> maps:
    -------------------------------------------
    Map ID  Num Vertices  Num Edges
    -------------------------------------------
    A       8             15
    B       8             15
    -------------------------------------------
    The Server A has received input for finding shortest paths: starting vertex <6> of map <A>.
    The Server A has identified the following shortest paths:
    -----------------------------
    Destination  Min Length
    -----------------------------
    7            6
    8            4
    9            12
    10           14
    11           17
    12           15
    13           21
    -----------------------------
    The Server A has sent shortest paths to AWS.

serverB:
    The Server B is up and running using UDP on port <22657>.
    The Server B has received data for calculation:
    * Propagation speed: <1.18> km/s;
    * Transmission speed <8808038.40> Bytes/s;
    * Path length for destination <7>: <6>;
    * Path length for destination <8>: <4>;
    * Path length for destination <9>: <12>;
    * Path length for destination <10>: <14>;
    * Path length for destination <11>: <17>;
    * Path length for destination <12>: <15>;
    * Path length for destination <13>: <21>;
    The Server B has finished the calculation of the delays:
    ------------------------
    Destination        Delay
    ------------------------
    7                  8.83
    8                  7.13
    9                  13.91
    10                 15.61
    11                 18.15
    12                 16.45
    13                 21.54
    ------------------------
    The Server B has finished sending the output to AWS


f.Any idiosyncrasy of your project. It should say under what conditions the project fails, if any.
    I didn't meet with some problems when I was running my program in Ubuntu.

g. Reused code
    The code about Dijkstra algorithm is based on the tutorial from Beej's Guide.