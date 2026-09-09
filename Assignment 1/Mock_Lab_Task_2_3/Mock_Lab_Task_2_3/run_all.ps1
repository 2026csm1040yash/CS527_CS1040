gcc -Wall -Wextra -O2 Driver/main.c Src/csr.c Src/coloring.c Src/pagerank.c -I Include -o assignment4_mock.exe -lm
if ($LASTEXITCODE -ne 0) { exit 1 }
New-Item -ItemType Directory -Force Output | Out-Null
& .\assignment4_mock.exe coloring Testcases\Task2_Coloring.txt | Tee-Object Output\Task2_Coloring_Output.txt
& .\assignment4_mock.exe pagerank Testcases\Task3_PageRank.txt | Tee-Object Output\Task3_PageRank_Output.txt
