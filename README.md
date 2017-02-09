# gChat
Chat program with room to improve

The software is composed by three different files:

1. Input client (cl.c)

2. Visualizer/output client (vis.c)

3. The server (ser.c)


##1 - The client

This is like an input box for the chat, you won't see any output here.

###1.1 - conf.txt

The first time is executed the client will crete a file conf.txt and ask you to compile the host and nickname field.
Conf.txt can be modified inside the folder of execution.

###1.2 - Commands
###
You have some useful services that the client can ask to the server (don't write the []):

`/admn [password]`

This command will make you and administrator(if you know the password for the server),
quite lame state for now but you will be able to execute the next command:

`/cast [something]`

You will broadcast everything inside the brakets, on every chatroom.

`/prch [room] [room_password]`

You will create or enter (if already exist) a private room; the password field is optional,
if entered the room will be protected by password, set this entry to 0 is the same of leaving empty.

`/prrm [put something]`

You will destroy the private room and send every user back to "plaza". You need to be an admin.

`/pass [something/room_name]`

Normal user: will send back the password of the current room (NEED to put something to work).
Admin user: will send back the password for the room entered.

`/nick [new nick]`

You will be able to change your nick in the server, is only a temporary change, the default nick can be changed in the conf.txt file

`/end`

Close the client. You can also use `Ctrl+C`.

##2 - The Visualizer

The visualizer work with the same conf.txt file of the client so be sure tu put the in the same folder.

You will be asked to enter a room, leave blank or "plaza" and you will enter the default room of the server.
The room must be created from the client before be entered.

##3 - The server
The server do everything by itself, the first time is executed it will create the file ser_conf.txt where you can put the password for admin,
after it will start on port 9734;

The `-` means that the "Hall" is waiting a client to join and be dispatched in some room.

The `!name of room!` mean the room is waiting for an input

The output is formatted this way: `nameofroom, |nick| message`

#Compile it!
Compile by using
```
gcc -D_REENTRANT -g ser.c -o ser -lpthread

gcc -D_REENTRANT -g vis.c -o vis -lpthread

gcc -D_REENTRANT -g cl.c -o cl -lpthread

```
