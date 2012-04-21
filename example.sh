#!/bin/bash

# build code
make

# delay start clients
(sleep 1; ./client white &> /dev/null) &
(sleep 1; ./client black &> /dev/null) &

# start server
./server 0


