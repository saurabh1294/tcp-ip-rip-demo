Part I)

Product name : A reliable transport layer and network layer protocol for overlay networks

Source code by:- saurabh S Gattani

About the lab :- The lab mainly focusses on developing a reliable transport layer protocol
and a network layer protocol for overlay networks. The main aim is to send large data files
over the network reliably in the presence of a garbler which intercepts the data packets
and introduces data loss, corruption and duplication. The routing protocol used is
RIP(routing information protocol) where in each node builds its routing information and 
broadcasts to its neighbours after every 't' seconds. Thus, after some time each
node will learn about the network topology and based on the destination node, the next hop
will be decided. The routing table is such that it only includes the shortest path
to the destination(based on the number of hops).

Steps to execute :-

1) First copy all the .c and .h files on each node alongwith the Makefile
2) Do a "make clean" and make sure the existing .o files , if any are removed
3) Then do a "make all". It will generate a .o file. Here it will be start.o
4) Besides this, each node will also have a configuration file which will tell
   the node about its neighbours. The structure of the configuration file will 
   be like
   <Node ID of the current node>
   <No of links to that node>
   <Node ID of neighbour1>
   <IP addr of neighbour1>
   <Data port used to communicate with neighbour1>
   <Control port used to communicate with neighbour1>
	.
	.
	.
	.
	.
   <Node ID of neighbour n>
   <IP addr of neighbour n>
   <Data port used to communicate with neighbour n>
   <Control port used to communicate with neighbour n>

   A sample configuration file will be like


	1
	2
	2
	129.82.232.18
	1500
	1501
	6
	129.82.232.23
	1600
	1601


 5) On each node, run an instance of the project using the following command
    start <config-file name>. On execution a menu will appear with the options
	i)   Display routing table of this node
	ii)  Send file to a node
	iii) Exit


Option i) will show the routing table of the node
Option ii) will send a file to a node
Option iii) is for exit



A file will be split into chunks and sent to the destination node using the best path





Part II)
Dead node detection:

Suppose, if a node is dead, then if it is possible to reach the destination through some other path, the
routing tables of all the alive nodes should reflect that and packets should be routed via that path.
RIP(routing information procotocl) is the routing procotol being used and, number of hops is used as the
routing metric for best path selection. If, a node is dead, the hop count on its path(which will then be
inactive) is set to 255(which is the maximum number of hops supported in this case), so that when the
next time, a routing update is received, it will be replaced by a route which has fewer hops than 255 and
packets will be routed through that path.
 