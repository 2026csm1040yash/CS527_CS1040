# Mock Lab Test - Task 2 and Task 3

Task 2 implements Welsh-Powell Greedy Vertex Coloring on CSR.

Task 3 implements PageRank on directed CSR with damping, tolerance, maximum iterations, simultaneous updates, and dangling-node handling.

## Windows
```powershell
gcc -Wall -Wextra -O2 Driver/main.c Src/csr.c Src/coloring.c Src/pagerank.c -I Include -o assignment4_mock.exe -lm
.\assignment4_mock.exe coloring Testcases\Task2_Coloring.txt
.\assignment4_mock.exe pagerank Testcases\Task3_PageRank.txt
```

Or:
```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\run_all.ps1
```
