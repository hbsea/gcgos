struct spinlock
{
    uint32 locked;
    int count;
    struct cpu* p;
};
