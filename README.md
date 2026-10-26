_This project has been created as part
of the 42 curriculum by **lsarraci, ilemos-c and csuomins**_

# FT_IRC #

### Description

This project is a implementation of a simple IRC server in
C++. It contains the basic funcionality of an IRC server, such as handling multiple clients, managing channels, and processing commands.
It follows the IRC protocol as defined in RFC 1459 and RFC 2812.

### Instructions

Testing: for purpose of testing the server, you can use the following command in the terminal:

```bash
	./ircserv <port> <password> ###you can use the port 6667 and any password you want
```

and then you can use the following command in another terminal to connect to the server:

```bash
	nc localhost <port> ###you can use the port 6667
```
or if you have irssi installed you can use the following command in another terminal to connect to the server:

```bash
	irssi -c localhost -p <port> ###you can use the port 6667
```

and after that, on the client side using nc, you can use the following commands to test the server:

```bash
	PASS <password> ###to set your password
	NICK <nickname> ###to set your nickname
	USER <username> <hostname> <servername> <realname> ###to set your username
	QUIT ###to disconnect from the server
```

### Resources

Use of AI: this project was done using Copilot to assist with the unit tests and tools like ChatGPT and Deepseek to help to understand the concepts, but the code itself was written by the students.
