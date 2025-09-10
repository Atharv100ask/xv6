#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "heap") == 0) {
    void *p = malloc(4096 * 2);  // Allocate 2 pages
    if (p)
      printf(1, "Allocated heap memory at %p\n", p);
  }

  printf(1, "[u] Calling vmprint syscall to print the first 10 pages in the\n"
    "    current process's virtual address space:\n");
  vmprint(0);

  printf(1, "\n[u] Calling vmprint syscall to print the full page table:\n");
  vmprint(1);
  exit();
}
