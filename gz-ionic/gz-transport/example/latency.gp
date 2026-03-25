# Produces three PNG graphs of latency results from the "bench" example program.
#
# Output filenames:
#
#     latency-small.png : Graph of small message payloads
#     latency-large.png : Graph of large message payloads
#     latency-all.png : Graph of all the data
#
# Usage:
#   gnuplot -e "filename='MY_FILENAME'; prefix='MY_PREFIX'" latency.gp
#
#   where prefix is a string that will be prepended to the title. This can
#   be used to describe the test scenario.

set terminal png size 1920,1080 enhanced font 'Verdana, 20'
set ylabel 'microseconds'
set xlabel 'Message size (bytes)'
set grid
set xtics (0, '256' 1, '512' 2, "1K" 3, "2K" 4, "4K" 5, "8K" 6, "16K" 7,\
           "32K" 8, "64K" 9, "128K" 10, "256K" 11, "512K" 12, "1MB" 13,\
           "2MB" 14, "4MB" 15)

set output sprintf("latency-%s-all.png", prefix)
set title sprintf("%s Latency", prefix)
plot filename using 1:3 with linespoints title 'Avg' lw 2

set output sprintf("latency-%s-small.png", prefix)
set title sprintf("%s Latency with Small Messages", prefix)
set xrange [1:9]
plot filename using 1:3 with linespoints title 'Avg' lw 2

set output sprintf("latency-%s-large.png", prefix)
set title sprintf("%s Latency with Large Messages", prefix)
set xrange [7:15]
plot filename using 1:3 with linespoints title 'Avg' lw 2
