Directory 'samples' contains verious versions of a sample Client and Server written using different techniques.
Such samples have a very basic functionality and are added for illustration purposes only.
All samples use the same communication protocol to make it possible to run any Server and any Client to see how they work.
In addition to a Client and a Server, one can build and run 'ctrl_client' application which can receive and show some stats from a Server.
Fursther, for comparison purposes, directory 'nodejs_client_and_server' contains Node.JS files implementing the very same functionality.

Quick build instructions:
1. Each of Client and Server sample directory has a directory named 'build' with
(i) MSVS project file
(ii) build scripts to build respective executables on Linux platform using clang++ and g++ compilers.

Quick run and test instructions:
In the present version each sample application has a hardcoded address to communicate with 
(it can be found in a file [client|server]/user_code/NetSocket.h, presently set to 127.0.0.1)
One can reasonably change it to locations where Client and Server are actually running.
In the present configuration Client and Server can be run on the same machine.
More that a single Client (of the same or different types) can be run at any point of time.
To see some stats related to what's going on on Server, a ctrl_client can also be run.
It updates sends requests to the Server, receives reply, and updates its output based on user pressing ENTER.

Sample output of ctrl_client:

[info] 4762234734000, 0, 0, 0, 0

[info] 4762245171000, 1, 39268, 2506210, 19634

[info] 4762246625000, 1, 53214, 3403665, 26607

[info] 4762257562000, 2, 160450, 10259059, 80225

[info] 4762258562000, 2, 170280, 10889636, 85140

[info] 4762259343000, 2, 177826, 11371821, 88913

[info] 4762268296000, 1, 269276, 17223602, 134638

[info] 4762283187000, 0, 383318, 24519845, 191659

[info] 4762284093000, 0, 383318, 24519845, 191659

where columns are [timestamp in microsecs] [number of clients currently connected] [received size(bytes)] [sent size (bytes)] [number of requests processed]

Performance can then be calculated as a ration of a difference of respective params from different lines over difference of timestamps from those lines.