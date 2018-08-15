# Chatbox_group_version
This is the local chatting program for group. Using TCP and select method to implement

# What it does
each client connect to server in order to communicate with a big group.
you can send message to all other client in the group
and it shows which client send the message

# reference
Beej's Guide to Network Programming

# how to test
compile with
 ```
 gcc chatbox.c -o chatbox
```
To run the server:
```
./chatbox
```
To run the client use telnet or nc for now
```
nc localhost 4444
```
or 
```
telnet localhost 4444
```
