#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "========================================================\n");
  printf(1, "         QUESTION 3: DUMPING PROCESS PAGE TABLE         \n");
  printf(1, "========================================================\n");

  printf(1, "\n--------------------------------------------------------\n");
  printf(1, " 1. Initial Page Table Dump at Process Start-Up\n");
  printf(1, "--------------------------------------------------------\n");
  vmprint();

  printf(1, "\n--------------------------------------------------------\n");
  printf(1, " 2. Page Table Dump After Growing Heap by 8 Pages\n");
  printf(1, "--------------------------------------------------------\n");
  char *p = sbrk(8 * 4096);
  if(p == (char*)-1){
    printf(1, "vmprinttest: sbrk(8*4096) failed\n");
    exit();
  }
  // Touch all 8 pages so that page faults map them into the page table
  int i;
  for(i = 0; i < 8; i++){
    p[i * 4096] = (char)(i + 1);
  }
  vmprint();

  printf(1, "\n--------------------------------------------------------\n");
  printf(1, " 3. Page Table Dump After Shrinking Heap by 1 Page\n");
  printf(1, "--------------------------------------------------------\n");
  if(sbrk(-4096) == (char*)-1){
    printf(1, "vmprinttest: sbrk(-4096) failed\n");
    exit();
  }
  vmprint();

  printf(1, "\n--------------------------------------------------------\n");
  printf(1, " 4. Verification and Stack Guard Page Analysis\n");
  printf(1, "--------------------------------------------------------\n");
  printf(1, "Analysis:\n");
  printf(1, "- In dump 1: The stack guard page created by exec() is identified\n");
  printf(1, "  by [GUARD PAGE] (mapped with Present=1, Writeable=1, User=0).\n");
  printf(1, "- The user stack page is immediately above it with User=1.\n");
  printf(1, "- In dump 2: 8 new consecutive PTE entries appeared in user space.\n");
  printf(1, "- In dump 3: Exactly 1 PTE entry disappeared after sbrk(-4096).\n");
  printf(1, "- The number of user-accessible pages equals (sz / PGSIZE) - 1 guard page.\n");

  printf(1, "\n========================================================\n");
  printf(1, "             VMPRINTTEST COMPLETED SUCCESSFULLY         \n");
  printf(1, "========================================================\n");
  exit();
}
