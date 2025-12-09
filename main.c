#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

// High-level tour for newcomers:
// - Build/toolchain: the Makefile drives a tiny LLVM/binutils toolchain to
//   compile the kernel into an ELF image that bootasm.S loads with QEMU/Bochs.
//   The ELF layout is documented in kernel.ld and visible with objdump -h.
// - OS structure: main() is the C entry point on the bootstrap processor and
//   wires up early allocators (kinit1/kinit2), the kernel page table
//   (kvmalloc), device controllers, and finally enters the scheduler. Other
//   CPUs start in entryother.S and rendezvous in mpmain() before scheduling.
// - Debugging/exploration: run "make qemu-nox-gdb" and connect gdb to the
//   QEMU GDB stub; cprintf() is the printf-style kernel logger; panic() drops
//   into an infinite loop so gdb can inspect memory; symbols match vmlinux.
// - Kernel browsing: defs.h provides prototypes; the proc/ files define the
//   process lifecycle; vm.c explains the 4-level x86-64 page tables; trap.c
//   describes interrupts; fs.c/log.c cover the file system stack.
// - Security/virtualization: xv6 assumes a cooperative lab VM; it relies on
//   hardware privilege levels and paging but omits production hardening.

static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  uartearlyinit();
  kinit1(end, P2V(PHYSTOP)); // phys page allocator
  kvmalloc();      // kernel page table
  mpinit();        // detect other processors
  lapicinit();     // interrupt controller
  tvinit();        // trap vectors
  seginit();       // segment descriptors
  cprintf("\ncpu%d: starting Fall 2025 xv6\n\n", cpunum());
  ioapicinit();    // another interrupt controller
  consoleinit();   // console hardware
  uartinit();      // serial port
  pinit();         // process table
  binit();         // buffer cache
  fileinit();      // file table
  ideinit();       // disk
  startothers();   // start other processors
  kinit2();
  userinit();      // first user process
  mpmain();        // finish this processor's setup
}

// Other CPUs jump here from entryother.S.
void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}

// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting\n", cpunum());
  idtinit();       // load idt register
  syscallinit();   // syscall set up
  xchg(&cpu->started, 1); // tell startothers() we're up
  scheduler();     // start running processes
}

void entry32mp(void);

// Start the non-boot (AP) processors.
static void
startothers(void)
{
  extern uchar _binary_entryother_start[], _binary_entryother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memmove(code, _binary_entryother_start,
          (addr_t)_binary_entryother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == cpus+cpunum())  // We've started already.
      continue;

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = kalloc();
    *(uint32*)(code-4) = 0x8000; // enough stack to get us to entry64mp
    *(uint32*)(code-8) = v2p(entry32mp);
    *(uint64*)(code-16) = (uint64) (stack + KSTACKSIZE);

    lapicstartap(c->apicid, V2P(code));

    // wait for cpu to finish mpmain()
    while(c->started == 0)
      ;
  }
}
