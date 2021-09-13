set key top left
set grid
set xlabel "Elapsed time[ms]"
set ylabel "Progress[%]"
filename="log/2core-1process.log"
set title filename
plot filename using 2:($1=="0"?$3:1/0) title "Process 0"
pause -1
