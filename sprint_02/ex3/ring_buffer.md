wait queue solution also uses a mutex for protecting the data since the wait queue does not guarantee another thread doesn't read or write as soon as the thread wakes.
remove data override, since a wait is now implemented, no need to override data (would defeat the purpose of waiting for space).
