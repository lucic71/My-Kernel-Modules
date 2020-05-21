This module prints a string to the tty when it is inserted in the kernel and 
when is removed.

To get the tty it uses the get_current_tty function defined in linux/tty.h

Each tty has a driver containt ops. In these ops there is a write operation
too. We need to call this operation to get some string displayed.

Also it is important not to put \n at the end of a line, but a \CR\LF because
the ttys are following the ASCII standard.


