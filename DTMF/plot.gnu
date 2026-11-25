
set terminal qt size 1500,1000

set multiplot layout 4,4 rowsfirst title "DTMF Signal Visualization\nGreen: Low | Magenta: High | Blue: Combined"

set xlabel "Time (s)"
set ylabel "Amplitude"

set grid
set xrange [0:0.01]
set yrange [-0.6:0.6]

set key off

set style line 1 lc rgb "#16A34A" lw 1.2
set style line 2 lc rgb "#D946EF" lw 1.2
set style line 3 lc rgb "#2563EB" lw 1.5

set tics font ",8"
set title font ",10"

array keys[16] = ["1","2","3","A", \
                  "4","5","6","B", \
                  "7","8","9","C", \
                  "*","0","#","D"]

array low[16] = [697,697,697,697, \
                 770,770,770,770, \
                 852,852,852,852, \
                 941,941,941,941]

array high[16] = [1209,1336,1477,1633, \
                  1209,1336,1477,1633, \
                  1209,1336,1477,1633, \
                  1209,1336,1477,1633]

do for [i=1:16] {

    set title sprintf("%s  (%d Hz + %d Hz)", \
                      keys[i], low[i], high[i])

    plot "signal.txt" index (i-1) using 1:2 with lines ls 1 notitle, \
         "signal.txt" index (i-1) using 1:3 with lines ls 2 notitle, \
         "signal.txt" index (i-1) using 1:4 with lines ls 3 notitle
}

unset multiplot
