#include "proc.h"
#include "global.h"
#include "def.h"
#include "proc.h"
#include "const.h"
#include "memlayout.h"

struct proc *
allocproc(void)
{
    struct proc *p;
    char *sp;

    for (p = ptable; p < ptable + NPROC; p++)
        if (p->state == UNUSED)
            break;


    if (p == ptable + NPROC)
        return NULL;

    p->state = EMBRYO;
    p->pid = nextpid++;

    if ((p->kstack = kalloc()) == NULL) {
        p->state = UNUSED;
        return NULL;
    }

    sp = p->kstack + KSTACKSZ;

    // Leave room for trap frame.
    sp -= sizeof(*p->tf);
    p->tf = (struct trapframe *)sp;

    sp -= sizeof(*p->context);
    p->context = (struct context *)sp;
    memset(p->context, 0, sizeof *p->context);
    p->context->eip = (u32)restart;

    return p;
}


void
schedule()
{
    struct proc *p;
    int c, idle;

    while (1) {
        c = 0;
        idle = 1;

        while (1) {
            for (p = ptable; p < ptable + NPROC; p++)
                if (p->state == RUNNABLE) {
                    if (p != ptable + TASK_IDLE)
                        idle = 0;
                    if (p->counter > c) {
                        c = p->counter;
                        current = p;
                    }
                }

            if ((current == ptable + TASK_IDLE && idle) ||
                    current != ptable + TASK_IDLE)
                break;
                // task_idle && !idle
            for (p = ptable; p < ptable + NPROC; p++)
                    p->counter = p->priority;
        }
        // break to here
        // current has been set
        // char *a[] = {"0", "1", "2", "3", "4", "5", "6", "7","8"};
        // kputs(a[current - ptable]);
        tss.esp0 = (u32)current->kstack + KSTACKSZ;
        lcr3(V2P(current->pgdir));
        swtch(&scheduler, current->context);
    }
}

void
sched()  //trans to schedule !NOTE must cli
{
    swtch(&current->context, scheduler);
}



struct task_t {
    void (*start)(void);
    char    name[16];
};

void
task_idle() // 0
{
    while (1) {
        asm("hlt"); // annote this in bochs
        cli();
        sched();
        sti();
    }
}

struct task_t ktasks[4] = {
    [TASK_IDLE] = {task_idle, "task_idle"},
    [TASK_SYS]  = {task_sys, "task_sys"},
    [TASK_MM]   = {task_mm, "task_mm"},
    [TASK_FS]   = {task_fs, "task_fs"},
};

void
taskinit(void)
{
    struct task_t *t;
    struct proc *p;
    for (t = ktasks; t < ktasks + LEN(ktasks); t++) {
        p = allocproc();
        assert(p);
        assert(p - ptable == t - ktasks);

        p->pgdir = kpgdir;

        safestrcpy(p->name, t->name, sizeof(p->name));
        memset(p->tf, 1, sizeof(*p->tf));//debug
        p->tf->cs = (SEG_KCODE << 3);
        p->tf->ss = (SEG_KDATA << 3);
        p->tf->ds = p->tf->es =
                p->tf->fs = p->tf->gs = p->tf->ss;
        p->tf->esp = (u32)p->kstack + KSTACKSZ;
        p->tf->eip = (u32)t->start;
        p->tf->eflags = FL_IF;
        p->brk = 0; // never used

        p->state = RUNNABLE;
        p->counter = p->priority = 20;
        if (t - ktasks == TASK_IDLE)
            p->counter = p->priority = 1;
    }
}



void
userinit(void)
{
    struct proc *p;
    extern char _binary_kernel_initcode_start[],
            _binary_kernel_initcode_size[];
    char *mem;

    p = allocproc();
    assert(p);

    initproc = p;
    if((p->pgdir = setupkvm(kalloc)) == NULL)
        panic("userinit: out of memory?");

    if ((int)_binary_kernel_initcode_size > PGSIZE)
        panic("inituvm: initcode more than a page");

    mem = kalloc();
    memset(mem, 0, PGSIZE);
    mappages(p->pgdir, 0, PGSIZE, V2P(mem), PTE_W|PTE_U, kalloc);
    memcpy(mem, (char *)_binary_kernel_initcode_start,
           (int)_binary_kernel_initcode_size);

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->brk = PGSIZE;
    memset(p->tf, 1, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ss = (SEG_UDATA << 3) | DPL_USER;
    p->tf->ds = p->tf->es =
            p->tf->fs = p->tf->gs = p->tf->ss;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0;  // beginning of initcode

    p->counter = p->priority = 10;
    p->state = RUNNABLE;
}

void
sleep_on(void *p)
{
    current->tmp = p;
    current->state = SLEEPING;
    sched();

    //wake up
    current->tmp = NULL;
}

void
sleep_on1(void *p)
{
    cli();
    sleep_on(p);
    sti();
}

void
wake_up(void *w)
{
    struct proc *p;
    for (p = ptable; p < ptable + NPROC; p++)
        if(p->state == SLEEPING && p->tmp == w)
            p->state = RUNNABLE;
}

void
wake_up1(void *p)
{
    cli();
    wake_up(p);
    sti();
}
