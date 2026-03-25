# Produces three PNG graphs of throughput results from the "bench" example
# program.
#
# Output filenames:
#
#     throughput-all.png : Graph of all the data
#     throughput-small.png : Graph of small message payloads
#     throughput-large.png : Graph of large message payloads
#
# Usage:
#   gnuplot -e "filename='MY_FILENAME'; prefix='MY_PREFIX'" throughput.gp
#
#   where prefix is a string that will be prepended to the title. This can
#   be used to describe the test scenario.
#

set terminal png size 1920,1080 enhanced font "Roboto, 20"
set ylabel 'MB/s' tc lt 1
set ytics nomirror tc lt 1
set y2label 'Kmsgs/s' tc lt 2
set y2tics nomirror tc lt 2
set xlabel 'Message size (bytes)'
set grid
set xtics (0, '256' 1, '512' 2, "1K" 3, "2K" 4, "4K" 5, "8K" 6, "16K" 7,\
           "32K" 8, "64K" 9, "128K" 10, "256K" 11, "512K" 12, "1MB" 13,\
           "2MB" 14, "4MB" 15)
set linetype 1 lw 2
set linetype 2 lw 2

set output sprintf("throughput-%s-all.png", prefix)
set title sprintf("%s Throughput", prefix)
plot filename using 1:3 linetype 1 with linespoints title 'MB/s', \
     filename using 1:4 linetype 2 with linespoints title 'Kmsgs/s' axes x1y2

set output sprintf("throughput-%s-small.png", prefix)
set title sprintf("%s Throughput with Small Message", prefix)
set xrange [1:9]
plot filename using 1:3 linetype 1 with linespoints title 'MB/s', \
     filename using 1:4 linetype 2 with linespoints title 'Kmsgs/s' axes x1y2

set output sprintf("throughput-%s-large.png", prefix)
set title sprintf("%s Throughput with Large Message", prefix)
set xrange [7:15]
plot filename using 1:3 linetype 1 with linespoints title 'MB/s', \
     filename using 1:4 linetype 2 with linespoints title 'Kmsgs/s' axes x1y2
