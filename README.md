# Tasks tracker

This is an helper that keep tracks of long-running job remotely.
The interface listen to Server-Sent Events at a given URL which are sent by custom libraires.

# Network
The daemon is by default bound to 127.0.0.1:9000 for security concern (you dont want your LAN to listen to your tasks).

It can be bound to 0.0.0.0 by the -P (public) flag and to custom port with the -p flag

To transfer the data trough a network, it is recommended to bind local ports to remote ports using SSH port forwarding, so data can only be access on designed computer.
e.g.

    ssh -L 9000:localhost:9000 -Nf <user>@<server>
  
# Installation

    make install
