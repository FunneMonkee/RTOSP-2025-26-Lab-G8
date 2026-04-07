both first and the tree's structure need protecting from concurrent writes and reads.
Same thing as the fifo queue, we can use mutexes here.
first is protected and computed after balancing.
protect clean up too.
