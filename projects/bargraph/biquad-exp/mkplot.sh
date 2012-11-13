#!/bin/bash

html="bq.html"

echo '<html><body>' >$html

(echo 'set term pngcairo font "Helvetica, 12" size 800, 600 truecolor enhanced'
echo 'set xlabel "t (ms)"'
echo 'set ylabel "amplitude"'
for f in $((for f in bq-*.txt; do
            echo $(echo $f | cut -f 2 -d "-")
        done) | sort -g | uniq) ; do
    header=`head -1 bq-$f-1t-f.txt | sed s/^#//g`
    echo "set output \"bq$f.png\""
    #echo "set title \"$f Hz Bandpass 1(t) Response\""
    echo "set title \"$header\""
    echo "plot \"bq-$f-1t-f.txt\" with lines title \"FP\", \"bq-$f-1t-i.txt\" with lines title \"int\""

    echo "<img src=\"bq$f.png\"/><br/>" >> $html
done) | gnuplot

(echo 'set term pngcairo font "Helvetica, 12" size 800, 600 truecolor enhanced'
echo 'set xlabel "Freq (Hz)"'
echo 'set ylabel "mag (dB)"'
echo 'set logscale x'
echo 'set xrange [20:16000]'
echo 'set yrange [-20:0]'
echo 'set output "bands.png"'
echo 'set grid mxtics ytics'
echo 'set ytics -20,1,0'
echo 'set title "Frequency responses superimposed"'
babor=$(for f in $((for f in fr-*.txt; do
            f=${f/fr-}
            echo ${f/%.txt}
        done) | sort -g | uniq) ; do
    echo -n "\"fr-$f.txt\" with lines title \"$f\","
done)
babor=${babor/%,/}

echo 'plot' $babor
) | gnuplot

echo "<img src=\"bands.png\"/><br/>" >> $html


echo '</body></html>'>> $html
