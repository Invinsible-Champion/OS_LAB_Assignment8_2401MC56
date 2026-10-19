#ifndef _MEMINFO_H_
#define _MEMINFO_H_

// Structure containing physical memory and process memory statistics
struct meminfo {
  int total_pages;        // Total physical memory pages managed by kernel
  int free_pages;         // Number of currently free physical memory pages
  int user_pages;         // Number of pages in process user address space (sz / PGSIZE)
  int page_table_pages;   // Number of physical pages used by process page tables (incl pgdir)
};

#endif
