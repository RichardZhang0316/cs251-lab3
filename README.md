-implement pipes in your shell.
The shell should handle least two pipes form the command line.
If A, B and C are Unix commands you shell should be able to run the following commands:
A | B
A | B | C

-implement the history command.
You should create a hidden file called my history (.myhistory).
When the shell reads a command from the command line, it will appended the file. The history
command will display all commands stored in the file, each line preceded by the number of the command
relative to the start of the file. The file can a maximum of MAX_HISTORY commands. After the limit has
been reach, new commands will still be appended to the file, but the oldest entry will be deleted. The
MAX_HISTORY constant is declared within the shell.
The command :
% erase history
will reset the history file by deleting all entries in the file.
