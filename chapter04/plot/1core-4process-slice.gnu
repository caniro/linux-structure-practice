set key top left
set grid
set xlabel "Elapsed time[ms]"
set ylabel "Progress[%]"
filename="log/1core-4process.log"
set title filename
plot filename using 2:($1=="0"?$1:1/0) title "Process 0", \
    filename using 2:($1=="1"?$1:1/0) title "Process 1", \
    filename using 2:($1=="2"?$1:1/0) title "Process 2", \
    filename using 2:($1=="3"?$1:1/0) title "Process 3"
pause -1
