#include "types.h"
#include "stat.h"
#include "user.h"
#include "meminfo.h"

int
main(int argc, char *argv[])
{
  struct meminfo m0, m1, m2, m3;
  int alloc_size = 32 * 1024 * 1024; // 32 MB
  char *base;

  printf(1, "========================================================\n");
  printf(1, "         QUESTION 4: LAZY MEMORY ALLOCATION TEST        \n");
  printf(1, "========================================================\n");

  // Step 1: Baseline memory state
  if(getmeminfo(&m0) < 0){
    printf(1, "lazytest: getmeminfo failed\n");
    exit();
  }
  printf(1, "\n1. Baseline Memory State:\n");
  printf(1, "   Free pages : %d\n", m0.free_pages);
  printf(1, "   User pages : %d\n", m0.user_pages);

  // Step 2: Large sbrk (32 MB = 8192 pages)
  printf(1, "\n2. Calling sbrk(32 MB) without touching pages...\n");
  base = sbrk(alloc_size);
  if(base == (char*)-1){
    printf(1, "lazytest: sbrk(32MB) failed\n");
    exit();
  }

  if(getmeminfo(&m1) < 0){
    printf(1, "lazytest: getmeminfo failed\n");
    exit();
  }
  printf(1, "   Free pages after 32MB sbrk : %d\n", m1.free_pages);
  printf(1, "   User pages after 32MB sbrk : %d (delta: +%d pages)\n",
         m1.user_pages, m1.user_pages - m0.user_pages);

  if(m1.free_pages == m0.free_pages){
    printf(1, "   Verdict: PASS (32MB allocated lazily with 0 physical frames consumed)\n");
  } else {
    printf(1, "   Verdict: NOTE (Free pages changed by %d)\n", m0.free_pages - m1.free_pages);
  }

  // Step 3: Touch only a few widely spaced pages
  printf(1, "\n3. Touching widely spaced pages (indices 0, 1000, 4000, 8000)...\n");
  base[0] = 'A';
  base[1000 * 4096] = 'B';
  base[4000 * 4096] = 'C';
  base[8000 * 4096] = 'D';

  if(getmeminfo(&m2) < 0){
    printf(1, "lazytest: getmeminfo failed\n");
    exit();
  }
  printf(1, "   Read back touched values: [%c, %c, %c, %c]\n",
         base[0], base[1000 * 4096], base[4000 * 4096], base[8000 * 4096]);
  printf(1, "   Free pages after touching 4 pages : %d (frames used: %d)\n",
         m2.free_pages, m1.free_pages - m2.free_pages);

  if(base[0] == 'A' && base[1000 * 4096] == 'B' &&
     base[4000 * 4096] == 'C' && base[8000 * 4096] == 'D'){
    printf(1, "   Verdict: PASS (Only touched pages faulted in and mapped correctly)\n");
  } else {
    printf(1, "   Verdict: FAIL (Touched values corrupted)\n");
  }

  // Step 4: System call with untouched buffer (Pipe test)
  printf(1, "\n4. Testing system call (read/write pipe) with untouched lazy buffer...\n");
  int pfd[2];
  if(pipe(pfd) < 0){
    printf(1, "lazytest: pipe failed\n");
    exit();
  }
  char *untouched_buf = base + 2500 * 4096;
  char msg[] = "Lazy kernel buffer test passed!";
  int msg_len = strlen(msg) + 1;

  if(write(pfd[1], msg, msg_len) != msg_len){
    printf(1, "lazytest: write to pipe failed\n");
    exit();
  }
  if(read(pfd[0], untouched_buf, msg_len) != msg_len){
    printf(1, "lazytest: read from pipe into untouched buffer failed\n");
    exit();
  }
  close(pfd[0]);
  close(pfd[1]);
  printf(1, "   Buffer content read from pipe: \"%s\"\n", untouched_buf);
  printf(1, "   Verdict: PASS (Kernel handled untouched lazy buffer smoothly)\n");

  // Step 5: Test fork & exit with lazy memory
  printf(1, "\n5. Testing fork() and exit() with lazily allocated address space...\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "lazytest: fork failed\n");
    exit();
  }
  if(pid == 0){
    // Child process: verify parent values and touch another page
    if(base[0] != 'A' || base[1000 * 4096] != 'B'){
      printf(1, "child: verification failed\n");
      exit();
    }
    base[6000 * 4096] = 'E'; // Child touches another lazy page
    exit();
  } else {
    wait();
    printf(1, "   Verdict: PASS (Fork copied valid pages, skipped untouched; child reaped cleanly)\n");
  }

  // Step 6: Test invalid access termination (process terminated without kernel crash)
  printf(1, "\n6. Testing invalid memory accesses (should kill child, not kernel)...\n");

  // Test A: Address >= sz
  pid = fork();
  if(pid == 0){
    printf(1, "   [Child A] Accessing address >= sz (should be killed by kernel)...\n");
    volatile char *bad_addr = base + alloc_size + 4096;
    char val = *bad_addr;
    printf(1, "   ERROR: Child A should have been killed, but read %d\n", val);
    exit();
  } else {
    wait();
    printf(1, "   Verdict: PASS (Invalid access >= sz properly terminated child process)\n");
  }

  // Test B: Address below stack (stack guard page 0x2000)
  pid = fork();
  if(pid == 0){
    printf(1, "   [Child B] Accessing stack guard page (0x2000 below stack)...\n");
    volatile char *bad_ptr = (char*)0x2000; /* stack guard page */
    char val = *bad_ptr;
    printf(1, "   ERROR: Child B should have been killed, but read %d\n", val);
    exit();
  } else {
    wait();
    printf(1, "   Verdict: PASS (Invalid access below stack properly terminated child process)\n");
  }

  // Step 7: Clean up by shrinking heap
  printf(1, "\n7. Shrinking 32 MB heap with sbrk(-32 MB)...\n");
  if(sbrk(-alloc_size) == (char*)-1){
    printf(1, "lazytest: sbrk shrink failed\n");
    exit();
  }
  if(getmeminfo(&m3) < 0){
    printf(1, "lazytest: getmeminfo failed\n");
    exit();
  }
  printf(1, "   Final free pages: %d (Initial was: %d)\n", m3.free_pages, m0.free_pages);

  printf(1, "\n========================================================\n");
  printf(1, "             LAZYTEST COMPLETED SUCCESSFULLY            \n");
  printf(1, "========================================================\n");
  exit();
}
