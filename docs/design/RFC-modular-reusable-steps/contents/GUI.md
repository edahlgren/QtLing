## GUI

Generally speaking, the job of a graphical user interface is to execute commands, to wait for those commands to finish (often showing progress updates), and to display a result to a user. 

In this proposal, a graphical user interface should still execute a series of steps (which may or may not be separate programs) and display results to the user, but instead of reading linguistic data from memory, it should read the linguistic data from the user's filesystem (or, if the Crab algorithm is running in the Cloud, over a network socket).

#### Progress updates

Less obvious is how to display progress updates. It is simplest for the GUI to maintain a connection (via a pipe or socket) to the programs responsible for executing the Crab algorithm steps ("the step programs"). If the GUI and the step programs are all running locally, the connection would be a local one (a named pipe or a unix socket). If the GUI runs locally and step programs run remotely (Cloud setup), the connection would be a tcp (network) socket.

The purpose of the connection is to allow the GUI to receive progress updates from the step program. The GUI would repeatedly execute a blocking read on the connection to wait for progress messages, and it would stop reading from the connection when all steps complete or an error occurs. The GUI would then report an error to the user, or, if all steps were successful, read from the user's filesystem to display the computed linguistic data.

This proposal doesn't contain an example demonstrating how to setup and receive messages from a connection shared between two processes. But this can easily be integrated into each of the examples, if desirable.

[[Back to Table of Contents]](../README.md#table-of-contents)
