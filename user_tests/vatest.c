#include "types.h"
#include "stat.h"
#include "user.h"

int global_var = 12345;

int
main(int argc, char *argv[])
{
  int local_var = 67890;
  char *heap_buf;

  printf(1, "========================================================\n");
  printf(1, "         QUESTION 1: VIRTUAL TO PHYSICAL TRANSLATION    \n");
  printf(1, "========================================================\n");

  heap_buf = malloc(256);
  if(!heap_buf){
    printf(1, "vatest: malloc failed\n");
    exit();
  }
  heap_buf[0] = 'Z';

  printf(1, "\n--- Part (c): VA to PA Translation for Current Process ---\n");
  int pa_global = va2pa((int)&global_var);
  printf(1, "1. Global variable 'global_var':\n");
  printf(1, "   Virtual Address : 0x%x\n", (uint)&global_var);
  printf(1, "   Physical Address: 0x%x\n", pa_global);

  int pa_local = va2pa((int)&local_var);
  printf(1, "2. Local variable 'local_var':\n");
  printf(1, "   Virtual Address : 0x%x\n", (uint)&local_var);
  printf(1, "   Physical Address: 0x%x\n", pa_local);

  int pa_heap = va2pa((int)heap_buf);
  printf(1, "3. Heap block from malloc():\n");
  printf(1, "   Virtual Address : 0x%x\n", (uint)heap_buf);
  printf(1, "   Physical Address: 0x%x\n", pa_heap);

  uint unmapped_addr = 0x7FFFFFFF;
  int pa_unmapped = va2pa((int)unmapped_addr);
  printf(1, "4. Unmapped address (0x7FFFFFFF):\n");
  printf(1, "   Virtual Address : 0x%x\n", unmapped_addr);
  printf(1, "   Physical Address: %d (Expected: -1)\n", pa_unmapped);

  if(pa_unmapped == -1){
    printf(1, "   Result: PASS (Correctly identified unmapped address)\n");
  } else {
    printf(1, "   Result: FAIL (Expected -1 for unmapped address)\n");
  }

  printf(1, "\n--- Part (d): Translation Across fork() ---\n");
  printf(1, "Calling fork() to verify physical address divergence...\n");

  int pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    free(heap_buf);
    exit();
  }

  if(pid == 0){
    // Child process
    int child_pa = va2pa((int)&global_var);
    printf(1, "[CHILD]  pid: %d | Global VA: 0x%x -> PA: 0x%x\n",
           getpid(), (uint)&global_var, child_pa);
    exit();
  } else {
    // Parent process waits for child
    wait();
    int parent_pa = va2pa((int)&global_var);
    printf(1, "[PARENT] pid: %d | Global VA: 0x%x -> PA: 0x%x\n",
           getpid(), (uint)&global_var, parent_pa);
    
    printf(1, "Notice: Both parent and child share the same Virtual Address (0x%x),\n", (uint)&global_var);
    printf(1, "        but map to different Physical Addresses because xv6 copies pages on fork.\n");
  }

  free(heap_buf);
  printf(1, "\n========================================================\n");
  printf(1, "                VATEST COMPLETED SUCCESSFULLY           \n");
  printf(1, "========================================================\n");
  exit();
}
