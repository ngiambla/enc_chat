# Enc Chat #

This is an implementation of a chatroom, (using multicast over unicast), secured by a symmetric key algorithm

## Compilation ##

Using the terminal, enter the commands:

```bash
make
```

## Execution ##
In order for the chatroom to operate, have two terminal sessions open (at least),
Reserve one terminal for the server (as the multicast over unicast forwarder)
Run:
```bash
./server
```
This process will ask you to set a session password. Feel free to choose any password.


Run:
```bash
./client
```

Only 5 users are known to the server, `r1cardo`, `leon4rdo`, `appl3`, `micr0` & `c0de`.


## Bugs, Comments or Concerns ##
Let me know at gdev.engineering@gmail.com, and 'fork' off.

