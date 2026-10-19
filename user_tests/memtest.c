#include "types.h"
#include "stat.h"
#include "user.h"
#include "meminfo.h"

void
print_meminfo(char *label, struct meminfo *mi)
{
  printf(1, "--- %s ---\n", label);
  printf(1, "  Total Physical Pages : %d (%d MB)\n", mi->total_pages, (mi->total_pages * 4096) / (1024 * 1024));
  printf(1, "  Free Physical Pages  : %d (%d MB)\n", mi->free_pages, (mi->free_pages * 4096) / (1024 * 1024));
  printf(1, "  Process User Pages   : %d (%d KB)\n", mi->user_pages, (mi->user_pages * 4096) / 1024);
  printf(1, "  Page Table Pages     : %d (%d KB)\n", mi->page_table_pages, (mi->page_table_pages * 4096) / 1024);
}

int
main(int argc, char *argv[])
{
  struct meminfo mi_initial, mi_allocated, mi_child, mi_after_child;
  int num_pages = 10;
  int alloc_bytes = num_pages * 4096;

  printf(1, "========================================================\n");
  printf(1, "         QUESTION 2: PHYSICAL MEMORY STATISTICS         \n");
  printf(1, "========================================================\n");

  // Step 1: Initial memory statistics
  if(getmeminfo(&mi_initial) < 0){
    printf(1, "memtest: getmeminfo failed\n");
    exit();
  }
  print_meminfo("1. Initial Memory State", &mi_initial);

  // Step 2: Grow memory using sbrk() and touch the pages
  printf(1, "\nAllocating %d pages (%d bytes) with sbrk and touching them...\n", num_pages, alloc_bytes);
  char *p = sbrk(alloc_bytes);
  if(p == (char*)-1){
    printf(1, "memtest: sbrk failed\n");
    exit();
  }
  int i;
  for(i = 0; i < num_pages; i++){
    p[i * 4096] = (char)(i + 1); // Touch page to allocate physical frame
  }

  if(getmeminfo(&mi_allocated) < 0){
    printf(1, "memtest: getmeminfo failed\n");
    exit();
  }
  print_meminfo("2. Memory State After sbrk(10 pages)", &mi_allocated);

  printf(1, "\nObservations after allocation:\n");
  printf(1, "  User pages delta : +%d pages (expected +%d)\n",
         mi_allocated.user_pages - mi_initial.user_pages, num_pages);
  printf(1, "  Free pages delta : -%d pages\n",
         mi_initial.free_pages - mi_allocated.free_pages);

  // Step 3: Fork a child and observe free_pages
  int free_before_fork = mi_allocated.free_pages;
  printf(1, "\nForking a child process (free pages before fork = %d)...\n", free_before_fork);

  int pid = fork();
  if(pid < 0){
    printf(1, "memtest: fork failed\n");
    exit();
  }

  if(pid == 0){
    // Child process
    if(getmeminfo(&mi_child) < 0){
      printf(1, "child: getmeminfo failed\n");
      exit();
    }
    printf(1, "\n[CHILD pid %d] Memory state while child is running:\n", getpid());
    printf(1, "  Child Free Pages : %d (decreased by %d due to fork copy)\n",
           mi_child.free_pages, free_before_fork - mi_child.free_pages);
    printf(1, "  Child User Pages : %d\n", mi_child.user_pages);
    exit();
  } else {
    // Parent process waits for child to exit and be reaped
    wait();
    printf(1, "\n[PARENT pid %d] Child process exited and reaped by wait().\n", getpid());

    if(getmeminfo(&mi_after_child) < 0){
      printf(1, "parent: getmeminfo failed\n");
      exit();
    }
    print_meminfo("3. Memory State After Child Exited and Reaped", &mi_after_child);

    // Step 4: Verify NO memory leak
    printf(1, "\n--- Part (d): Memory Leak Verification ---\n");
    printf(1, "  Free pages before fork : %d\n", free_before_fork);
    printf(1, "  Free pages after reap  : %d\n", mi_after_child.free_pages);
    if(mi_after_child.free_pages == free_before_fork){
      printf(1, "  Verdict: SUCCESS! Zero memory leak detected (exact match).\n");
    } else {
      printf(1, "  Verdict: WARNING: Free page mismatch (difference: %d)\n",
             mi_after_child.free_pages - free_before_fork);
    }
  }

  // Clean up allocated memory
  sbrk(-alloc_bytes);
  struct meminfo mi_final;
  getmeminfo(&mi_final);
  printf(1, "\nAfter shrinking heap by %d pages: Free pages = %d (Initial was %d)\n",
         num_pages, mi_final.free_pages, mi_initial.free_pages);

  printf(1, "\n========================================================\n");
  printf(1, "                MEMTEST COMPLETED SUCCESSFULLY          \n");
  printf(1, "========================================================\n");
  exit();
}
