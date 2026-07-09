set terminal png size 1000,600
set output 'dati/confronto_dct2.png'
set title 'Confronto Tempo Esecuzione DCT2: Algoritmo custom vs FFTW'
set xlabel 'Dimensione array bidimensionale (N)'
set ylabel 'Tempo di esecuzione (ms)'
set logscale y
set grid
set style data linespoints
set pointsize 1.5
plot 'dati/risultati_dct2.txt' using 1:2 title 'DCT2 custom' pt 7, \
     'dati/risultati_dct2.txt' using 1:3 title 'FFTW MEASURE' pt 5, \
     'dati/risultati_dct2.txt' using 1:4 title 'FFTW ESTIMATE' pt 3
