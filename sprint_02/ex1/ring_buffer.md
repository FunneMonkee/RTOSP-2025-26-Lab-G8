CONSTANT BUFFER_LEN = 100
CONSTANT QUEUE_LEN = 5

STRUCT QueueItem:
    buffer[BUFFER_LEN]

STRUCT Ring:
    queue[QUEUE_LEN]
    write_index
    read_index

GLOBAL ring

FUNCTION increment(index_pointer):

    old_value = index_pointer

    index_pointer = (index_pointer + 1) MOD QUEUE_LEN

    RETURN old_value

FUNCTION is_empty(read, write):

    RETURN (read == write)

FUNCTION is_full(read, write):

    next_write = (write + 1) MOD QUEUE_LEN

    RETURN (next_write == read)

FUNCTION enqueue(input_buffer):

    IF buffer is full:
        increment(ring.read_index) // overwrite oldest 

    COPY input_buffer INTO ring.queue[write_index]

    increment(ring.write_index)

    RETURN true

FUNCTION dequeue(output_buffer):

    IF buffer is empty:
        RETURN false

    COPY ring.queue[read_index] INTO output_buffer

    increment(ring.read_index)

    RETURN true

FUNCTION proc_read(user_buffer, count, file_position):

    DECLARE local_buffer[BUFFER_LEN]

    IF file_position > 0:
        RESET file_position
        RETURN 0

    IF dequeue(local_buffer) == false:
        RETURN 0   // empty

    length = LENGTH(local_buffer)

    IF length <= 0 OR count < length:
        RETURN error

    COPY local_buffer TO user_buffer

    UPDATE file_position

    RETURN length

FUNCTION proc_write(user_buffer, count):

    IF count > BUFFER_LEN:
        RETURN error

    DECLARE local_buffer[BUFFER_LEN]

    COPY user_buffer INTO local_buffer

    ADD null terminator

    IF enqueue(local_buffer) == false:
        RETURN error

    RETURN count

FUNCTION proc_init():

    CREATE /proc entry "ring_buffer"

    IF creation fails:
        RETURN error

    ring.read_index = 0
    ring.write_index = 0

    RETURN success

FUNCTION proc_exit():
    REMOVE /proc entry "ring_buffer"
