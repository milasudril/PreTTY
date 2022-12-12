# PreTTY

Inspired by Jupyter, PreTTY is intended to be used for prototyping applications that need advanced pretty-printing during the prototyping phase. The name comes from an imaginary pretty-printing TTY. Also, PreTTY is not a fully functional TTY, since there is currently not possible to send data to stdin of a program. Thus, one can think of the name as a Pre TTY. Its operation is more comparable to Compiler Explorer, with a panel for source code on the left, and an output window to the right.

PreTTY relies on starting a web server, so that the output can be easily presented in a web browser. The web server only listens to requests from localhost, and runs as the user who started PreTTY. When the user exits the PreTTY Workbench, the server automatically shuts down.