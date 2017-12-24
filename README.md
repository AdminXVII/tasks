# Tasks tracker

This is an helper that keep tracks of long-running job remotely
The interface listen to Server-Sent Events at a given URL. The default port is 9000.
In the backend, there is a daemon that multiplexes the data
The jobs "notify" the daemon via a custom library

The daemon is by default bound to 127.0.0.1:9000 for security concern.
It can be bound to 0.0.0.0 by the -P (public) flag and to custom port with the -p flag
To transfer the data trough a network, it is recommended to encrypt the data using stunnel or ssh.
