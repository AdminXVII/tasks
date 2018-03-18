# Tasks tracker

This is an helper that keep tracks of long-running job remotely.
Tasks send info to the server through custom libraires. The server then transmits it via a Server-Sent Events on a net socket

# Network
The daemon is by default bound to 127.0.0.1:9000 for security concern (you dont want your LAN to listen to your tasks).

It can be bound to 0.0.0.0 by the -P (public) flag (this is not recommended) and to custom port with the -p flag

To transfer the data trough a network, it is recommended to bind the local port to the remote port using SSH port forwarding, so data can only be access on authorized computers.
e.g.

    ssh -L 9000:localhost:9000 -Nf <user>@<server>
  
# Installation

    ./configure
    make
    make install

# Usage

 1. Import a custom library, initialize it and send messages periodically in your tasks.
 2. Open your browser and navigate to https://chezxavier.ga/tasks
 3. Watch your tasks

# API
## Browser side
The net protocol this server uses is Server-Sent Events. Each task gets a unique ID for its entire duration (i.e. UIDs get reused after a while).
Three events are defined:
1-*new*: A new task is born. Content is a name.
2-*msg*: Update sent by task. Content is the update.
3-*end*: The task died. Content is an error message or an empty string to display the last message.

Data is provided as JSON mapping UIDs to the content. i.e.:

    {
      uid (int): content (string),
      ...
    }
    
## Tasks side
Events are provided as sequential packets through a UNIX socket (by default it is located at /run/tasks.sock as a systemd service or /tmp/tasks.sock otherwise). The first packet defines the name and subsequent packets are defined as messages. Interruption of the communication is understood as the end of task.

# Implementation restrictions
 - The name is truncated after 100 characters
 - Messages are truncated after 1000 characters