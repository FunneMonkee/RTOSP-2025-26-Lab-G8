timer runs in softirp context so lock with a spinlock
spin_lock_irqsave to disable interrupts
