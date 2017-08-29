# Enc Chat #

This is an implementation of a chatroom, (using multicast over unicast), secured by a symmetric key algorithm

## Compilation ##

Using the terminal, enter the commands:

```bash
gcc -o client client.c enc.c -lncurses -lpthread
gcc -o server server.c enc.c
```

## Execution ##
In order for the chatroom to operate, have two terminal sessions open (at least),
Reserve one terminal for the server (as the multicast over unicast forwarder)
Run:
```bash
./server
```

This particular program requires that each user enter the password 'thirtyfive' in order to enter the chatroom.
This chatroom requires 5 members to be online in order for communication to start. Each user has a name (defined in server.c)

Run:
```bash
./client
```

## Bugs, Comments or Concerns ##
Let me know at gdev.engineering@gmail.com, and 'fork' off.

