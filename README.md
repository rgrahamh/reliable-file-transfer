# Reliable File Transfer Protocol
Data Com Final Project

README.md : Includes the following sections:

    Description: Brief description of this project
    Build: Brief instructions on how to build the client and server
    Specification: Specification created for requirement 2
    Test: Description of testing procedure and results

## Description
RFTP (Reliable File Transfer Protocol) is a stop-and-wait transport-layer protocol built on top of UDP. It ensures that packets are delivered correctly (without corruption) and in order, as well as ensuring that packets actually make it to their final destination.

## Build
In order to build the client-side application, navigate to the `client` folder and type:
```
make client
```

In order to build the client-side application, navigate to the `client` folder and type:
```
make server
```

## Specification
RFTP packets will have four bits of additional fields in the first byte of the body (and four bits of buffer after said flags before the actual start of the body's contnt):  
1. `HANDSHAKE_BIT` - If the packet being sent is part of a handshake. If it is, set to 1; otherwise, set to 0.
2. `ACK` - If the last packet was valid; set to 1 if that last packet was fine, set to 0 if the last packet was corrupted.
3. `SEQUENCE_BIT` - Tracks the current sequence number of the packet; flips between 1 and 0 on every packet successfully received. Set to 0 during handshake, and the first packet sent should be a sequence of 1.
4. `LAST_BIT` - Tracks if that's supposed to be the last packet sent

Firstly, we will have a three-way handshake to initiate a connection. When sending handshake packets, the `HANDSHAKE_BIT` will be set to 1. During file transfer, the `HANDSHAKE_BIT` will stay at 0. The flow of said handshake is as follows:
1. The client sends a handshake packet to the server
2. The server sends a handshake response
3. The client sends a request for a file

So the the handshake should look something like this:

![handshake](/docs/handshake.png)

After the handshake is completed, we may start transferring the requested file from the client to the server.  

We will utilize the checksums already implemented in UDP in order to ensure that packets are delivered correctly. If a packet's body doesn't match its checksum (meaning their contents have been tainted along the way), a NACK will be sent back and the sender will re-send the bad packet. If the packet's contents are good, the receiver will send out an ACK packet. If no ACK comes back to the sender within two seconds, the packet will be re-sent. This will happen up to eight times before finally giving up on the connection. This will ensure that the content will actually reach its destination.  

In order to ensure proper packet ordering, the server will wait for an ACK packet to be received before sending new packets. We will use a `SEQUENCE_BIT` to ensure that we are receiving ACKs for the correct packet (to avoid interpreting re-sent ACKs as an acknowledgement for possibly lost packets). Every time we successfully receive and ACK, we will flip the `SEQUENCE_BIT` and send out the next packet. This should ensure proper packet ordering.  

If the requested file is not readable, the server will respond with `ACK`, `HANDSHAKE_BIT`, and `SEQUENCE_BIT` all set to 1. This is the only time that this should ever occur, since the `SEQUENCE_BIT` should be 0 during the handshake.

When properly implemented, the flow of packets would look like this:

![packet-example](/docs/packets-example.png)

## Test

In order to test if this implementation of RFTP works correctly, we should first follow the build instructions to build the client-side and server-side applications. In order to do this, we will open up VMWare, make a clone of hummingbird, and go to `Settings -> Network Adapter`. While we're there, we'll change the Network Adapter to `Host-only` and, in the `Advanced...` tab, introduce some amount of packet loss in the incoming and outgoing traffic.

After we've done this set-up, we can go ahead and start our VMs and open up a terminal on our host machine. In this example, we will use the host machine as the server and the VM as the client. Build the server and client using the instructions in the `Build` section, and start up the server using a command like this:
```
./server 8080
```
Then, go over to the VM and run a command similar to this:
```
./client <server_ip> 8080 ./files/eagle.txt ./eagle.txt
```
It may take a while since we wait a full 2 seconds to timeout before resending on a lost packet, but so long as it doesn't time out 8 times in a row (in which case it will print a message telling you that it timed out), the file will eventually be transferred. You can then open up said file and confirm that its contents are proper.
