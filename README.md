# Tasks tracker

This is an helper that keep tracks of long-running job remotely
The interface refresh periodically a json file at a given URL.
In the backend, there is a daemon that send the content of a designed tmp folder formatted as a the json
The jobs "notify" the daemon via a custom library

# Backend requirements
 + unix with mktemp
 + python

# TODO
 + Add support for other languages (C)
 + Add multiple URL
 + Parse arguments in backend

# JSON architecture:
[[["running section",""],["title","message"]],[["failed section",""],["title","error message"]]]

 + separate section -> minize length with multiple messages
 + title in tuple -> same title may be reused
