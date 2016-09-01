#HomeWork

A HTTP client that constructs an HTTP request based on user’s command line input, sends
the request to a Web server, receives the reply from the server, and displays the reply
message on screen. You should support only IPv4 connections.


#The purpose of this project is two-fold:
- give students hand-on experience with socket programming, and
- help students better understand application-level protocols by implementing a
well-known protocol HTTP. In this programming assignment you will write HTTP
client. Students are not required to implement the full HTTP specification, but only a very
limited subset of it.


##Compile the client:
gcc –o client client.c
client is the executable file.

##Test the client:
You can use the client to connect to any HTTP server

##How work the client
Command line usage: client [–h] [–d <time-interval>] <URL>. The flags and the url can
come at any order, the only limitation is that the time interval should come right after the
flag –d.
The client takes two options "-h" and "-d" and a required argument <URL>.
<URL> specifies the URL of the object that the client is requesting from server. The URL
format is http://hostname[:port]/filepath.
Option "-h" specifies that the client only wants the response headers to be sent back from
server. You should use HEAD method in your HTTP request when "-h" is specified in the
command line.
Option "-d" along with its argument <time-interval> specify that the client wants the object
only if it was modified in the past <time-interval>. <time-interval> is in the format
"day:hour:minute" For example, if "-d 1:2:15" is included in the command line, then the
client wants to get the object only if it was modified in the past 1 day, 2 hours and 15
minutes. When -d option is specified, the client should include an "If-Modified-Since:"
header in the HTTP request and the value of this header is computed using the current time
and the <time-interval> argument. In order to construct the time format for the ' If-
Modified-Since ' header.



##the flow of the app:
1. Parse the <URL> given in the command line.
2. Connect to the server
3. Construct an HTTP request based on the options specified in the command line
4. Send the HTTP request to server
5. Receive an HTTP response
6. Display the response on the screen.
