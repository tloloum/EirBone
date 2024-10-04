#!/bin/bash

# Run make command in parallel
for i in {1..6}; do
    (cd ../../../src/peer && make) &
done

# Wait for all background processes to finish
wait