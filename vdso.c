#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"
#include "param.h"
#include "proc.h"
#include "vdso.h"

static void
vdso_init_data(struct proc *p, char *page)
{
  struct vdso_data *data = (struct vdso_data *)page;
  data->magic = VDSO_MAGIC;
  data->pid = p->pid;
}

int
setupvdso(pml4e_t *pgdir, struct proc *p)
{
  char *page = kalloc();
  if(page == 0)
    return -1;

  memset(page, 0, PGSIZE);
  if(mappages(pgdir, (void *)VDSO_ADDR, PGSIZE, V2P(page), PTE_U) < 0){
    kfree(page);
    return -1;
  }

  vdso_init_data(p, page);
  return 0;
}

int
copyvdso(pml4e_t *dst, pml4e_t *src, struct proc *child)
{
  pte_t *pte = walkpgdir(src, (void *)VDSO_ADDR, 0);
  if(pte == 0 || (*pte & PTE_P) == 0)
    return 0;

  addr_t pa = PTE_ADDR(*pte);
  uint flags = PTE_FLAGS(*pte);
  char *page = kalloc();
  if(page == 0)
    return -1;

  memmove(page, (char *)P2V(pa), PGSIZE);
  if(mappages(dst, (void *)VDSO_ADDR, PGSIZE, V2P(page), flags) < 0){
    kfree(page);
    return -1;
  }

  vdso_init_data(child, page);
  return 0;
}

void
updatevdso(struct proc *p)
{
  char *page = uva2ka(p->pgdir, (char *)VDSO_ADDR);
  if(page)
    vdso_init_data(p, page);
}
