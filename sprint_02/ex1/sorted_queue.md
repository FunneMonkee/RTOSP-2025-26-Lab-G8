lembrete 
https://www.geeksforgeeks.org/dsa/introduction-to-red-black-tree/Node Color: Each node is either red or black.
Root Property: The root of the tree is always black.
Red Node Property: Red nodes cannot have red children (Red nodes cannot be adjacent).
Black Node Property: Every path from a node to its descendant leaves must have the same number of black nodes.
Leaf Property: All leaves (NIL nodes) are black.
-------------------------------------

CONSTANT BUFFER_LEN = 100

STRUCT QueueItem:
    buffer[BUFFER_LEN]
    left
    right
    parent
    color   // for RB-tree balancing

GLOBAL root = EMPTY_TREE
GLOBAL first = NULL   // pointer to smallest element

FUNCTION push(input_buffer):

    CREATE new_node
    IF allocation fails:
        RETURN false

    COPY input_buffer INTO new_node.buffer

    parent = NULL
    current = root
    is_first = true

    // Find correct position 
    WHILE current is not NULL:
        parent = current

        IF new_node.buffer < current.buffer:
            current = current.left
        ELSE:
            current = current.right
            is_first = false

    // Insert node
    SET new_node.parent = parent

    IF parent == NULL:
        root = new_node
    ELSE IF new_node.buffer < parent.buffer:
        parent.left = new_node
    ELSE:
        parent.right = new_node

    // Update pointer to smallest element
    IF is_first == true:
        first = new_node

    // Rebalance tree (RB-tree rules)
    RB_INSERT_FIXUP(root, new_node)

    RETURN true

FUNCTION pop(output_buffer):

    IF first == NULL:
        RETURN false

    node = first

    COPY node.buffer INTO output_buffer

    // Find next smallest element
    next_node = TREE_SUCCESSOR(node)

    IF next_node exists:
        first = next_node
    ELSE:
        first = NULL

    REMOVE node FROM RB-tree
    FREE node memory

    RETURN true

FUNCTION TREE_SUCCESSOR(node):

    IF node.right exists:
        RETURN LEFTMOST node in node.right subtree

    parent = node.parent
    WHILE parent exists AND node == parent.right:
        node = parent
        parent = parent.parent

    RETURN parent

FUNCTION proc_read(user_buffer, count, file_position):

    IF file_position > 0:
        RETURN 0

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

    ADD null terminator

    IF push(local_buffer) == false:
        RETURN error

    RETURN count

FUNCTION proc_init():

    CREATE /proc entry "sorted_queue"

    IF creation fails:
        RETURN error

    root = EMPTY_TREE
    first = NULL

    RETURN success

FUNCTION proc_exit():

    REMOVE /proc entry "sorted_queue"

    WHILE tree is not empty:
        node = SMALLEST node (leftmost)
        REMOVE node FROM tree
        FREE node memory
