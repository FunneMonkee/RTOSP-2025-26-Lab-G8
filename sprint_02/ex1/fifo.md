CONSTANT BUFFER_LEN = 100

STRUCT QueueItem:
    buffer[BUFFER_LEN]
    next
    prev

GLOBAL queue_head 

FUNCTION push(input_buffer):

    node = ALLOCATE memory for QueueItem
    IF node == NULL:
        RETURN false

    COPY input_buffer INTO node.buffer

    ADD node TO END of queue_head   // enqueue

    RETURN true

FUNCTION pop(output_buffer):

    IF queue_head is empty:
        RETURN false

    node = FIRST element of queue_head

    COPY node.buffer INTO output_buffer

    REMOVE node FROM queue_head     // dequeue

    FREE node memory

    RETURN true

FUNCTION proc_read(user_buffer, count, file_position):

    IF file_position > 0:
        RETURN 0   // only one read

    DECLARE local_buffer[BUFFER_LEN]

    IF pop(local_buffer) == false:
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

    ADD null terminator to local_buffer

    IF push(local_buffer) == false:
        RETURN error

    RETURN count

FUNCTION proc_init():

    CREATE /proc entry "fifo_queue"

    IF creation fails:
        RETURN error

    INITIALIZE queue_head as empty

    RETURN success

FUNCTION proc_exit():

    REMOVE /proc entry "fifo_queue"

    FOR EACH node IN queue_head:
        REMOVE node
        FREE node memory
