#include "proc.h"
#include "global.h"
#include "mmu.h"
#include "def.h"
#include "msg.h"
#include "memlayout.h"

static void fs_fork(struct proc *op, struct proc *np);
pde_t *copyuvm(pde_t *pgdir);

int
do_fork(void)
{
    struct proc *np;  // new process

    cli();
    np = allocproc();
    sti();
    if (np == NULL)
        return -1;


    if ((np->pgdir = copyuvm(mm_current->pgdir)) == NULL) {
        kfree1(np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
    }

    np->brk = mm_current->brk;
    np->parent = mm_current;
    *np->tf = *mm_current->tf;
    np->tf->eax = 0;        // child' sys_msg return 0

    lcr3(V2P(np->pgdir));
    ((struct msg *)np->tf->ecx)->type = MSG_SYSRET;
    ((struct msg *)np->tf->ecx)->i1 = 0;
    lcr3(V2P(current->pgdir));
    np->priority = mm_current->priority;
    np->counter = mm_current->counter;
    fs_fork(mm_current, np);
    // dupfiles

    safestrcpy(np->name, mm_current->name, sizeof(np->name));
    np->state = RUNNABLE;

    return np->pid; // return to father
}

pde_t*
copyuvm(pde_t *pgdir)
{

    pde_t *npgdir;
    pte_t *pte;
    u32 i, j;
    char *mem;


    if ((npgdir = setupkvm(kalloc1)) == NULL)
        return NULL;

    for (i = 0; (i << PDXSHIFT) < KBASE; i++) {
        if (pgdir[i] & PTE_P) {
            pte = (pte_t *)P2V(PTE_ADDR(pgdir[i]));
            for (j = 0; j < NPTENTRIES; j++)
                if (pte[j] & PTE_P) {
                    if ((mem = kalloc1()) == NULL)
                        goto bad;
                    memcpy(mem, (char *)P2V(PTE_ADDR(pte[j])), PGSIZE);
                    if (mappages(npgdir, (void *)((i << PDXSHIFT) | (j << PTXSHIFT)), // va
                                PGSIZE, V2P(mem), PTE_W|PTE_U, kalloc1) < 0)
                        goto bad;
                }
        }
    }
    return npgdir;

bad:
    freevm(npgdir);
    return NULL;
}

static void
fs_fork(struct proc *op, struct proc *np)
{
    struct msg m;
    m.type = MSG_FORK;
    m.p1 = op;
    m.p2 = np;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);
}

static void fs_exit(struct proc *p);

int
do_exit(int ret)
{
    struct proc *p;

    mm_current->state = EMBRYO;
    fs_exit(mm_current);


    cli();
    for (p = ptable; p < ptable + NPROC; p++){
        if (p->parent == mm_current) {
            p->parent = initproc;
            if(p->state == ZOMBIE)
                wake_up(initproc);
        }
    }
    sti();

    mm_current->msgstate = ret;
    mm_current->state = ZOMBIE;

    wake_up1(mm_current->parent);  // parent may sleep on wait()
    return 0;
}

static void
fs_exit(struct proc *p)
{
    struct msg m;
    m.type = MSG_EXIT;
    m.p1 = p;
    assert(sys_msg(BOTH, &m, TASK_FS) == 0);
    assert(m.type == MSG_SYSRET);
}

int
do_wait(int *state, int nohang)
{
    struct proc *p;

    cli();
    for (p = ptable; p < ptable + NPROC; p++)
        if (p->state == ZOMBIE &&
            p->parent == mm_current)
            break;
    sti();

    if (p == ptable + NPROC) {  // nochild
        if (nohang)
            return -1;
        else {
            mm_sleep = mm_current;    // let sleep
            return -2;
        }
    }

    kfree1(p->kstack);
    freevm(p->pgdir);
    p->name[0] = '\0';
    if (state)
        *state = p->msgstate;
    p->state = UNUSED;

    return p->pid;
}

int
do_sbrk(u32 n)
{
    u32 end, addr;

    addr = mm_current->brk;
    if ((end = allocuvm(mm_current->pgdir, addr, n)) == 0)
        return -1;

    mm_current->brk = end;

    return addr;
}

