# OS Lab Assignment 8: Memory Management in xv6

**Student Name:** ADITYA RAJ  
**Roll Number:** 2401MC56  
**Course:** Operating Systems Laboratory (CS342 / MA3104)  
**Institution:** Indian Institute of Technology Patna  
**Academic Year:** 2026–2027  

---

## 📁 Repository & Deliverables Structure

```text
2401MC56_OS_Lab_Assignment_8/
├── 2401MC56_Report.pdf               <-- Consolidated Lab Report (10 pages, A4)
├── OS_Lab_Assignment_8_Report.pdf    <-- Alternative named Lab Report
├── assignment8.patch                 <-- Unified Git patch of all kernel changes
├── README.md                         <-- This file
├── .gitignore                        <-- Git ignore rules for build artifacts
│
├── reports/                          <-- Report deliverables
│   ├── 2401MC56_Report.pdf
│   ├── OS_Lab_Assignment_8_Report.pdf
│   ├── OS_Lab_Assignment_8_Report.html
│   └── OS_Lab_Assignment_8_Report.md
│
├── screenshots/                      <-- Terminal test execution evidence
│   ├── Screenshot From 2026-10-19 22-50-41.png  (Q1: vatest)
│   ├── Screenshot From 2026-10-19 22-50-56.png  (Q2: memtest)
│   ├── Screenshot From 2026-10-19 22-51-19.png  (Q3: vmprinttest part 1)
│   ├── Screenshot From 2026-10-19 22-51-24.png  (Q3: vmprinttest part 2)
│   └── Screenshot From 2026-10-19 22-51-36.png  (Q4: lazytest)
│
├── user_tests/                       <-- User test source programs
│   ├── vatest.c                      (Q1 test suite)
│   ├── memtest.c                     (Q2 test suite)
│   ├── vmprinttest.c                 (Q3 test suite)
│   └── lazytest.c                    (Q4 test suite)
│
├── kernel_source/                    <-- Modified xv6 kernel source & headers
│   ├── Makefile                      (Build configuration with user test binaries)
│   ├── defs.h                        (Kernel function declarations)
│   ├── exec.c                        (heap_start initialization & guard page)
│   ├── kalloc.c                      (Physical frame accounting counters under lock)
│   ├── meminfo.h                     (struct meminfo definition)
│   ├── mmu.h                         (Paging macros and bitfield masks)
│   ├── param.h                       (System parameters)
│   ├── proc.c                        (Process allocation & heap_start tracking)
│   ├── proc.h                        (struct proc with heap_start)
│   ├── syscall.c                     (Syscall dispatch table & argptr pre-faulting)
│   ├── syscall.h                     (Syscall numbers: va2pa, getmeminfo, vmprint)
│   ├── sysproc.c                     (Syscall implementations & lazy sys_sbrk)
│   ├── trap.c                        (T_PGFLT page fault handler for demand paging)
│   ├── types.h                       (pte_t typedef)
│   ├── user.h                        (User-space syscall declarations)
│   ├── usys.S                        (User-space syscall assembly stubs)
│   └── vm.c                          (walkpgdir, vmprint, copyuvm lazy skipping)
│
├── patch/                            <-- Git diff patch
│   └── assignment8.patch
│
└── xv6-complete-source/              <-- Complete, compilable xv6 tree with all changes
    ├── Makefile
    ├── ... (all source files)
    └── (ready to run 'make qemu-nox')
```

---

## 🚀 How to Build and Run in QEMU

### Option A: Using the Complete xv6 Source Tree
```bash
cd xv6-complete-source
make clean
make qemu-nox
```

### Option B: Applying the Patch to a Clean xv6 Repository
```bash
git clone git://github.com/mit-pdos/xv6-public.git
cd xv6-public
git apply ../assignment8.patch
make qemu-nox
```

---

## 🧪 Testing the Implementations in xv6

Inside the xv6 QEMU terminal, run each test command:

### 1. Question 1 — Virtual to Physical Address Translation (`va2pa`)
```bash
$ vatest
```
- Decodes **PDX**, **PTX**, **Offset**, and permission flags (**P, W, U**) for global data (`0x11D4`), stack (`0x3FBC`), and heap (`0xBF00`).
- Validates sentinel check: returns `-1` for unmapped addresses (`0x7FFFFFFF`).
- Verifies address divergence across `fork()`: identical VA maps to separate physical addresses.

### 2. Question 2 — Physical Memory Statistics (`getmeminfo`)
```bash
$ memtest
```
- Tracks `total_pages` (57,064 pages &approx; 222.9 MB) and real-time `free_pages`.
- Verifies exact memory allocation delta upon `sbrk(10 * 4096)`.
- Validates fork memory overhead (child duplicates 14 user frames + 66 page table pages + 1 kstack).
- **Proves zero memory leaks:** Free pages return to exact baseline upon child wait and heap shrink.

### 3. Question 3 — Dumping Process Page Table (`vmprint`)
```bash
$ vmprinttest
```
- Traverses and prints the caller's two-level page hierarchy below `KERNBASE` (`0x80000000`).
- Identifies the stack guard page created by `exec()` (`PTE_U` cleared, flagged `[GUARD PAGE]`).
- Verifies formula: $\text{User Accessible Pages} = (sz / \text{PGSIZE}) - 1$.
- Demonstrates dynamic page addition upon `sbrk(8 * 4096)` and unmapping upon `sbrk(-4096)`.

### 4. Question 4 — Lazy Memory Allocation (Demand Paging)
```bash
$ lazytest
```
- **Zero-frame allocation:** `sbrk(32 MB)` increases virtual size by 32 MB with **0 physical frames** consumed.
- **On-demand fault servicing:** Touching pages triggers `T_PGFLT` (vector 14 via `%cr2`), allocating zeroed frames seamlessly.
- **Syscall buffer safety:** Pre-faults unmapped lazy buffers inside `argptr()` before kernel I/O (e.g., `pipe()`).
- **Process survival:** Illegal out-of-bounds accesses terminate offending children without crashing the kernel.

---

## 📊 Summary of Results & Verification

| Question | Feature | Verification Program | Result | Status |
| :--- | :--- | :--- | :--- | :--- |
| **Q1** | `va2pa` syscall | `vatest` | 32-bit VA &rarr; PA decoding, fork divergence | **PASSED (100%)** |
| **Q2** | `getmeminfo` syscall | `memtest` | Atomic frame tracking, zero memory leaks | **PASSED (100%)** |
| **Q3** | `vmprint` syscall | `vmprinttest` | Two-level tree dump, guard page validated | **PASSED (100%)** |
| **Q4** | Lazy Heap Allocation | `lazytest` | 32MB 0-frame alloc, demand paging, pipe test | **PASSED (100%)** |
| **Theory** | Page Replacement Study | Reference string (m=3,4,5) | Optimal achieves 40–44% reduction vs FIFO | **VERIFIED** |

---

## 👤 Author
- **Name:** ADITYA RAJ
- **Roll No:** 2401MC56
- **Department:** Computer Science & Engineering, IIT Patna
