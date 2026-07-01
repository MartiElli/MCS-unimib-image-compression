#include <iostream>
#include <fstream>
#include <Eigen/Core>
#include <fftw3.h>
#include <cmath>
#include <chrono>

/**
 * @brief Tripla che raccoglie i tempi di esecuzione della DCT2 custom e quella di FFTW data in input una matrice NxN
 */
struct Misura {
    int input_size;
    double customDCT2_time;
    double fftwDCT2_time;
};


/**
 * @brief Implementazione della Discrete Cosine Transform ad una dimensione
 * 
 * @param f un vettore di valori double
 * @return la trasformazione del vettore in input
 */
Eigen::VectorXd DCT1(Eigen::VectorXd f){

    unsigned int N = f.size();

    Eigen::MatrixXd D(N, N);
    // costruisco la matrice D
    for(unsigned int k = 0; k < N; ++k){

        double alpha_k;
        if(k == 0){
            alpha_k = 1/sqrt((double)N);
        }
        else{
            alpha_k = sqrt(2/(double)N);
        }

        for(unsigned int j = 0; j < N; ++j){
            D(k, j) = alpha_k * cos(k * M_PI * ((2 * (double)j + 1) / (2 * (double)N)));

        }

    }

    // stampe di test
    /*
    std::cout << "---------------" << std::endl;
    std::cout << "vettore da trasformare: " << std::endl << f;
    std::cout << "Matrice D: " << std::endl << D << std::endl;
    std::cout << "Vettore trasformato con DCT1:" << std::endl << D * f << std::endl;
    */
    return D * f;   // restituisco il vettore trasformato
}


/**
 * @brief Implementazione della Discrete Cosine Transform a due dimensioni
 * 
 * @param f una matrice di valori double
 * @return la trasformazione della matrice in input
 */
Eigen::MatrixXd DCT2(Eigen::MatrixXd& f){

    unsigned int rows = f.rows();
    unsigned int cols = f.cols();
    Eigen::MatrixXd c = f;

    // in pratica è DCT1 applicata alle righe e poi alle colonne

    //std::cout << "DCT1 SULLE RIGHE" << std::endl;   // stampa di test
    for(unsigned int i = 0; i < c.rows(); ++i){    // loop righe
        c.row(i) = DCT1(c.row(i));
    }

    //std::cout << "DCT1 SULLE COLONNE" << std::endl;   // stampa di test
    for(unsigned int j = 0; j < c.cols(); ++j){    // loop colonne
        c.col(j) = DCT1(c.col(j));
    }

    return c;
}


/**
 * @brief Funzione che costruisce un grafico con ordinate in scala logaritmica
 * @param data collezione di dati da "plottare"
 */
void generatePlot(const std::vector<Misura>& data) {
    
    std::ofstream data_file("dati/risultati_dct2.txt");
    for (const auto& m : data) {
        data_file << m.input_size << " " << m.customDCT2_time << " " << m.fftwDCT2_time << "\n";
    }
    data_file.close();

    // script GNUplot per generare un grafico con sole ordinate in scala logaritmica
    std::ofstream script("dati/grafico_dct2.gp");   // file dove viene scritto lo script
    script << "set terminal png size 1000,600\n";
    script << "set output 'dati/confronto_dct2.png'\n";
    script << "set title 'Confronto Tempo Esecuzione DCT2: Algoritmo custom vs FFTW'\n";
    script << "set xlabel 'Dimensione array bidimensionale (N)'\n";
    script << "set ylabel 'Tempo di esecuzione (ms)'\n";
    script << "set logscale y\n";  // Solo Y in scala logaritmica
    script << "set grid\n";
    script << "set style data linespoints\n";
    script << "set pointsize 1.5\n";
    script << "plot 'dati/risultati_dct2.txt' using 1:2 title 'Implementazione custom' pt 7, \\\n";
    script << "     'dati/risultati_dct2.txt' using 1:3 title 'FFTW' pt 5\n";
    script.close();

    system("gnuplot dati/grafico_dct2.gp");
    std::cout << "Grafico salvato come 'confronto_dct2.png' nella cartella 'dati'\n";
}


// test per verificare che viene effettivamente restituita da DCT2 una matrice modificata
void computationTest(){

    Eigen::MatrixXd m(5, 5);
    for(unsigned int i = 0; i < m.rows(); ++i){
        for(unsigned int j = 0; j < m.cols(); ++j){
            if(i == j){
                m(i, j) = 3;
            }
            else{
                m(i, j) = -1;
            }
        }
    }

    std::cout << "Matrice 5x5: " << std::endl << m << std::endl;
    Eigen::MatrixXd m_transformed = DCT2(m);
    std::cout << "Applicando DCT2 a questa matrice otteniamo: " << std::endl << m_transformed << std::endl;
}


int main(){

    //computationTest();

    std::vector<Misura> results;
    
    // Testa per N = 2, 4, 8, 16, 32, 64, 128, 256, 512 ...
    for (int N = 2; N <= 512; N *= 2) {
        // Crea una matrice N×N con dati random
        Eigen::MatrixXd mat(N, N); 
        for (unsigned int i = 0; i < N; i++) {
            for (unsigned int j = 0; j < N; j++) {
                mat(i, j) = std::rand() % 256;
            }
        }

        auto start = std::chrono::high_resolution_clock::now();
        DCT2(mat);
        auto end = std::chrono::high_resolution_clock::now();
        double customDCT2_time = std::chrono::duration<double, std::milli>(end - start).count();

        // Setup FFTW
        double* in = fftw_alloc_real(N * N);
        double* out = fftw_alloc_real(N * N);
    
        // Copia i dati da Eigen a FFTW
        for (unsigned int i = 0; i < N; i++) {
            for (unsigned int j = 0; j < N; j++) {
                in[i * N + j] = mat(i, j);
            }
        }

        // Crea il piano FFTW (con FFTW_MEASURE: stabilisce che FFTW seleziona l'algoritmo migliore)
        fftw_plan p = fftw_plan_r2r_2d(N, N, in, out, FFTW_REDFT10, FFTW_REDFT10, FFTW_MEASURE);

        start = std::chrono::high_resolution_clock::now();
        fftw_execute(p);
        end = std::chrono::high_resolution_clock::now();
        double fftw_time = std::chrono::duration<double, std::milli>(end - start).count();

        // deallocazione
        fftw_destroy_plan(p);
        fftw_free(in);
        fftw_free(out);

        results.push_back({N, customDCT2_time, fftw_time});
        
        std::cout << "N=" << N << " | Implementazione custom: " << customDCT2_time << "ms | Implementazione FFTW: " 
                  << fftw_time << "ms\n";   // ovviamente i tempi per FFTW non hanno senso perchè non sto ancora chiamando la libreria!
    }

    generatePlot(results);

    return 0;
}
