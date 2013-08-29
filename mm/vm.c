#include "memlayout.h"
#include "mmu.h"
#include "global.h"
#include "types.h"
#include "def.h"
#include "x86.h"

extern char data[];
extern char end[];
static char *newend;

pte_t *
walkpgdir(pde_t *pgdir, const void *va, char *(*alloc)(void))
{
    pde_t   *pde;
    pte_t   *pte;
    pde = &pgdir[PDX(va)];
    if (*pde & PTE_P)
        pte = (pte_t *)P2V(PTE_ADDR(*pde));
    else {
        if (!alloc || (pte = (pte_t *)alloc()) == NULL)
            return NULL;

        memset(pte, 0, PGSIZE);
        *pde = V2P(pte) | PTE_P | PTE_W | PTE_U;
    }

    return &pte[PTX(va)];
}

int
mappages(pde_t *pgdir, void *va, u32 size, u32 pa,
          int perm, char *(*alloc)(void))
{
    char *a, *last;
    pte_t *pte;

    a       = (char *)PGROUNDDOWN((u32)va);
    last    = (char *)PGROUNDDOWN((u32)va + size - 1);

    for (; a <= last; a += PGSIZE, pa += PGSIZE) {
        if ((pte = walkpgdir(pgdir, a, alloc)) == NULL)
            return -1;
        if (*pte & PTE_P)
            panic("remap");

        *pte = pa | perm | PTE_P;

        if (a >= (char *)0xfffff000)    break;
    }

    return 0;
}


static struct kmap {
    void *virt;
    u32 phys_start;
    u32 phys_end;  //addr just over the last byte
    u32 perm;
} kmap[] = {
    { (void *)P2V(0), 0, 1024*1024, PTE_W},  // I/O space
    { (void*)KLINK, V2P(KLINK), V2P(data),  0}, // kernel text+rodata
    { data, V2P(data), PHYSTOP,  PTE_W},  // kernel data, free memory
    { (void*)DEVSPACE, DEVSPACE, 0, PTE_W},  // more devices
};





pde_t *
setupkvm(char *(*alloc)(void))
{
    pde_t *pgdir;
    struct kmap *k;

    if ( (pgdir = (pde_t *)alloc()) == 0)
        return NULL;
    memset(pgdir, 0, PGSIZE);

    if (P2V(PHYSTOP) > DEVSPACE)
        panic("PHYSTOP TOO HIGHT");
    for (k = kmap; k < kmap + LEN(kmap); k++)
        if (mappages(pgdir, k->virt, k->phys_end - k->phys_start,
                     (u32)k->phys_start, k->perm, alloc) < 0)
            return NULL;
    return pgdir;
}


char *enter_alloc(void);

void
kpageinit(void)
{
    kpgdir = setupkvm(enter_alloc);
    lcr3(V2P(kpgdir));
}


void
seginit()
{
    lgdt(gdt, sizeof(gdt[0]) * NSEGS);
    gdt[SEG_TSS]   = SEGTSS(STS_T32A, &tss, sizeof(tss)-1, 0);
    ltr(SEG_TSS << 3);
}


char *
enter_alloc(void)   //only used during entry
{
    if (newend == 0)
        newend = (char *)PGROUNDUP((u32)end);

    if ((u32)newend + PGSIZE >= KBASE + 0x400000)
        panic("the entry 4M map is not enough!");
    char *p = newend;
    memset(p, 0, PGSIZE);
    newend += PGSIZE;
    return p;
}



struct page {
    struct page *next;
} *kfreelist;


void
kfree1(char *v)
{
    struct page *p;
    if ((u32)v % PGSIZE || v < newend || V2P(v) > PHYSTOP)
        panic("kfree");

    memset(v, 1, PGSIZE);
    cli();
    p = (struct page *)v;
    p->next = kfreelist;
    sti();
    kfreelist = p;
}

void
kfree(char *v)
{
    struct page *p;
    if ((u32)v % PGSIZE || v < newend || V2P(v) > PHYSTOP)
        panic("kfree");

    memset(v, 1, PGSIZE);

    p = (struct page *)v;
    p->next = kfreelist;
    kfreelist = p;
}

char *
kalloc1(void)
{
    struct page *p;
    cli();
    p = kfreelist;
    if (p)
        kfreelist = p->next;
    sti();
    return (char *)p;
}

char *
kalloc()
{
    struct page *p;

    cli();
    p = kfreelist;
    if (p)
        kfreelist = p->next;
    sti();
    return (char *)p;
}

void
kallocinit()
{
    char *p = newend;
    for(; p + PGSIZE <= (char*)P2V(PHYSTOP); p += PGSIZE)
        kfree(p);
}

void
freevm(pde_t *pgdir)
{
    u32 i, j;
    pte_t *pte;

    if (!pgdir)
        return;

    for(i = 0; i < NPDENTRIES; i++){
        if(pgdir[i] & PTE_P) {
            if ((i << PDXSHIFT) < KBASE) {      // low 2 GB is shared
                pte = (pte_t *)P2V(PTE_ADDR(pgdir[i]));
                for (j = 0; j < NPTENTRIES; j++)
                    if (pte[j] & PTE_P)
                        kfree1((char *)P2V(PTE_ADDR(pte[j])));
            }
            kfree1((char *)P2V(PTE_ADDR(pgdir[i])));
        }
    }
    kfree1((char*)pgdir);
}

