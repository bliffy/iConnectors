iConnectors
Copyright (C) 2015, Michael E. Jolley
This software is released under the terms of The MIT License.

INTENTION:
iConnectors was originally intended to be a cross-platform set of similar
abstractions for managing connection-oriented and connectionless socket
communications.
The only type currently supported is TCP.
Other connector implementations such as UDP, SSL, or ZMQ may be added in 
the future.

CURRENT USE:
The current implementation can serve as an easy solution for cross-platform
non-blocking TCP communication.
For windows, link with ws2_32[.a/.lib].

