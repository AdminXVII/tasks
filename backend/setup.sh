#/bin/bash
alias run='gcc -pthread -Wall -o tasks-tracker tasks-tracker.c server.c socket.c helpers.c && sudo ./tasks-tracker'
alias dbg='gcc -pthread -g -Wall -o tasks-tracker tasks-tracker.c server.c socket.c helpers.c && sudo gdb tasks-tracker'
