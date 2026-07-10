#include <iostream>
#include <fstream>
#include <filesystem>
#include <Eigen/Core>
#include <fftw3.h>
#include <cmath>
#include <chrono>

/**
 * @file dct2compare.cpp
 * @brief Parte 1 del progetto: 
 * confronto tra DCT2 custom (Eigen, scaling ortonormale) e DCT2 FFTW 
 * (fast, FFTW_REDFT10/REDFT01), per correttezza e tempo di esecuzione
 */

/**
 * @brief Matrice fornita dalla traccia di progetto per testare scaling di DCT2
 */
static const Eigen::MatrixXd scalingCheck = (Eigen::MatrixXd(8, 8) << 
    231,  32, 233, 161,  24,  71, 140, 245,
    247,  40, 248, 245, 124, 204,  36, 107,
    234, 202, 245, 167,   9, 217, 239, 173,
    193, 190, 100, 167,  43, 180,   8,  70,
     11,  24, 210, 177,  81, 243,   8, 112,
     97, 195, 203,  47, 125, 114, 165, 181,
    193,  70, 174, 167,  41,  30, 127, 245,
     87, 149,  57, 192,  65, 129, 178, 228).finished();


/**
 * @brief Matrice fornita dalla traccia di progetto come risultato atteso della DCT2 applicata alla matrice sopra
 */
static const Eigen::MatrixXd expectedMatrix = (Eigen::MatrixXd(8, 8) <<
    1.11e+03,  4.40e+01,  7.59e+01, -1.38e+02,  3.50e+00,  1.22e+02,  1.95e+02, -1.01e+02,
    7.71e+01,  1.14e+02, -2.18e+01,  4.13e+01,  8.77e+00,  9.90e+01,  1.38e+02,  1.09e+01,
    4.48e+01, -6.27e+01,  1.11e+02, -7.63e+01,  1.24e+02,  9.55e+01, -3.98e+01,  5.85e+01,
    -6.99e+01, -4.02e+01, -2.34e+01, -7.67e+01,  2.66e+01, -3.68e+01,  6.61e+01,  1.25e+02,
    -1.09e+02, -4.33e+01, -5.55e+01,  8.17e+00,  3.02e+01, -2.86e+01,  2.44e+00, -9.41e+01,
    -5.38e+00,  5.66e+01,  1.73e+02, -3.54e+01,  3.23e+01,  3.34e+01, -5.81e+01,  1.90e+01,
    7.88e+01, -6.45e+01,  1.18e+02, -1.50e+01, -1.37e+02, -3.06e+01, -1.05e+02,  3.98e+01,
    1.97e+01, -7.81e+01,  9.72e-01, -7.23e+01, -2.15e+01,  8.13e+01,  6.37e+01,  5.90e+00).finished();


/**
 * @brief Tripla che raccoglie i tempi di esecuzione della DCT2 custom e quella di FFTW data in input una matrice NxN
 */
struct Misura {
    int input_size;
    double customDCT2_time;
    double fftwDCT2_time;
    double fftwDCT2_time1;
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
Eigen::MatrixXd DCT2(const Eigen::MatrixXd& f){

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

    std::filesystem::create_directories("dati"); // crea la cartella se non esiste, altrimenti ofstream fallisce silenziosamente

    std::ofstream data_file("dati/risultati_dct2.txt");
    for (const auto& m : data) {
        data_file << m.input_size << " " << m.customDCT2_time << " " << m.fftwDCT2_time << " " << m.fftwDCT2_time1 << "\n";
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
    script << "plot 'dati/risultati_dct2.txt' using 1:2 title 'DCT2 custom' pt 7, \\\n";
    script << "     'dati/risultati_dct2.txt' using 1:3 title 'FFTW MEASURE' pt 5, \\\n";
    script << "     'dati/risultati_dct2.txt' using 1:4 title 'FFTW ESTIMATE' pt 3\n";
    script.close();

    int ret = system("gnuplot dati/grafico_dct2.gp");
    if (ret != 0) {
        std::cerr << "Errore: gnuplot ha restituito codice " << ret << ", grafico non generato\n";
    } else {
        std::cout << "Grafico salvato come 'confronto_dct2.png' nella cartella 'dati'\n";
    }
}


/**
 * @brief test per verificare che viene effettivamente restituita da DCT2 una matrice modificata
 */
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


/**
 * @brief test sulla scalabilità di DCT1 e DCT2 di FFTW versione ESTIMATE
 */
void FFTW_ESTIMATEScalingTest(){

    std::cout << "Test sullo scaling delle implementazioni di DCT1 e DCT2 della libreria FFTW - versione ESTIMATE" << std::endl;
    std::cout << "Matrice 8x8:" << std::endl << scalingCheck << std::endl;
    std::cout << "-----------------" << std::endl;

    double *in = fftw_alloc_real(8 * 8);
    double *out = fftw_alloc_real(8 * 8);


    Eigen::Map<Eigen::Matrix<double,
        Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(in, 8, 8) = scalingCheck; // copia contenuto di scalingCheck nel puntatore da dare in input alla trasformata

    fftw_plan dtc2_test = fftw_plan_r2r_2d(8, 8, in, out, FFTW_REDFT10, FFTW_REDFT10, FFTW_ESTIMATE);

    fftw_execute(dtc2_test);

    // scaling ortonormale posizionale: alpha_0 = 1/sqrt(N), alpha_k = sqrt(2/N) per k>0
    // (stessa convenzione usata nella DCT1/DCT2 custom). Il forward REDFT10 di FFTW
    // applica un fattore "2" per ogni dimensione (quindi "4" in 2D, da cui il vecchio
    // 0.25), e in piu' la convenzione ortonormale richiede alpha_k per riga/colonna:
    // il fattore corretto e' il prodotto di entrambi, non uno dei due da solo.
    double alpha[8];
    for (int i = 0; i < 8; i++) {
        alpha[i] = (i == 0) ? 1.0 / std::sqrt(8.0) : std::sqrt(2.0 / 8.0);
    }
    double baseScale = 0.25; // rimuove il fattore "4" del forward REDFT10 2D di FFTW
    for (int k = 0; k < 8; k++) {
        for (int l = 0; l < 8; l++) {
            out[k * 8 + l] *= baseScale * alpha[k] * alpha[l];
        }
    }

    Eigen::MatrixXd transformed = Eigen::Map<Eigen::Matrix<double,
        Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(out, 8, 8);    // copia ouptut in una matrice definita con Eigen

    fftw_destroy_plan(dtc2_test);
    fftw_free(in);
    fftw_free(out);

    std::cout << "Matrice 8x8 trasformata con DCT2 di FFTW:" << std::endl << transformed << std::endl;

    std::cout << "Matrice attesa (dalla traccia del progetto):" << std::endl << expectedMatrix << std::endl;
    std::cout << "Rapporti (calcolato/atteso, atteso ~1 ovunque se lo scaling e' corretto): " << std::endl;

    Eigen::MatrixXd ratios = transformed.array() / expectedMatrix.array();

    std::cout << "Rapporti (calcolato/atteso, atteso ~1 ovunque se lo scaling e' corretto): "
               << std::endl << ratios << std::endl;

}


/**
 * @brief test sulla scalabilità di DCT1 e DCT2 di FFTW
 */
void FFTW_MEASUREScalingTest(){

    std::cout << "Test sullo scaling delle implementazioni di DCT1 e DCT2 della libreria FFTW - versione MEASURE" << std::endl;
    std::cout << "Matrice 8x8:" << std::endl << scalingCheck << std::endl;
    std::cout << "-----------------" << std::endl;

    double *in = fftw_alloc_real(8 * 8);
    double *out = fftw_alloc_real(8 * 8);


    Eigen::Map<Eigen::Matrix<double,
        Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(in, 8, 8) = scalingCheck; // copia contenuto di scalingCheck nel puntatore da dare in input alla trasformata

    fftw_plan dtc2_test = fftw_plan_r2r_2d(8, 8, in, out, FFTW_REDFT10, FFTW_REDFT10, FFTW_MEASURE);

    Eigen::Map<Eigen::Matrix<double,
        Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(in, 8, 8) = scalingCheck; // copia contenuto della matrice nel puntatore da dare in input alla trasformata (DOPO la creazione del piano, altrimenti FFTW_MEASURE la sovrascrive)

    fftw_execute(dtc2_test);

    // scaling ortonormale posizionale: alpha_0 = 1/sqrt(N), alpha_k = sqrt(2/N) per k>0
    // (stessa convenzione usata nella DCT1/DCT2 custom). Il forward REDFT10 di FFTW
    // applica un fattore "2" per ogni dimensione (quindi "4" in 2D, da cui il vecchio
    // 0.25), e in piu' la convenzione ortonormale richiede alpha_k per riga/colonna:
    // il fattore corretto e' il prodotto di entrambi, non uno dei due da solo.
    double alpha[8];
    for (int i = 0; i < 8; i++) {
        alpha[i] = (i == 0) ? 1.0 / std::sqrt(8.0) : std::sqrt(2.0 / 8.0);
    }
    double baseScale = 0.25; // rimuove il fattore "4" del forward REDFT10 2D di FFTW
    for (int k = 0; k < 8; k++) {
        for (int l = 0; l < 8; l++) {
            out[k * 8 + l] *= baseScale * alpha[k] * alpha[l];
        }
    }

    Eigen::MatrixXd transformed = Eigen::Map<Eigen::Matrix<double,
        Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(out, 8, 8);    // copia ouptut in una matrice definita con Eigen

    fftw_destroy_plan(dtc2_test);
    fftw_free(in);
    fftw_free(out);

    std::cout << "Matrice 8x8 trasformata con DCT2 di FFTW (scalata):" << std::endl << transformed << std::endl;


    std::cout << "Matrice attesa (dalla traccia del progetto):" << std::endl << expectedMatrix << std::endl;
    std::cout << "Rapporti (calcolato/atteso, atteso ~1 ovunque se lo scaling e' corretto): " << std::endl;

    Eigen::MatrixXd ratios = transformed.array() / expectedMatrix.array();

    std::cout << "Rapporti (calcolato/atteso, atteso ~1 ovunque se lo scaling e' corretto): "
               << std::endl << ratios << std::endl;
}


/**
 * @brief test sulla scalabilità di DCT1 e DCT2 custom
 */
void customDCTScalingTest(){

    std::cout << "Test sullo scaling delle implementazioni custom di DCT1 e DCT2" << std::endl;

    std::cout << "Matrice 8x8:" << std::endl << scalingCheck << std::endl;
    std::cout << "-----------------" << std::endl;

    Eigen::MatrixXd transformed = DCT2(scalingCheck);
    std::cout << "Matrice 8x8 trasformata con DCT2 custom:" << transformed << std::endl << std::endl;
    std::cout << "-----------------" << std::endl;

    Eigen::VectorXd ratios(8 * 8);

    unsigned int index = 0;
    for(unsigned int i = 0; i < 8; ++i){
        for(unsigned int j = 0; j < 8; ++j){

            ratios(index) = transformed(i,j) / expectedMatrix(i,j);
            index++;
        }
    }

    std::cout << "Rapporti: " << ratios << std::endl;

    Eigen::VectorXd firstRow = scalingCheck.row(0);

    std::cout << "Prima riga della matrice originale:" << std::endl << firstRow << std::endl;
    std::cout << "-----------------" << std::endl;

    std::cout << "Riga transformata con DCT1 custom:" << std::endl << DCT1(firstRow) << std::endl;
    std::cout << "-----------------" << std::endl;

}


/**
 * @brief confronto tra implementazioni
 */
void implementationsComparison(){

    std::cout << "Confronto tra implementazioni custom e della libreria FFTW" << std::endl;
    std::vector<Misura> results;    // vettore dove raccogliere dati di esecuzione
    
    // Testa per matrici di dimensione N = 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
    for (int N = 2; N <= 1024; N *= 2) {
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
        fftw_plan p1 = fftw_plan_r2r_2d(N, N, in, out, FFTW_REDFT10, FFTW_REDFT10, FFTW_ESTIMATE);

        start = std::chrono::high_resolution_clock::now();
        fftw_execute(p);
        end = std::chrono::high_resolution_clock::now();
        double fftw_time = std::chrono::duration<double, std::milli>(end - start).count();

        // deallocazione
        fftw_destroy_plan(p);

        start = std::chrono::high_resolution_clock::now();
        fftw_execute(p1);
        end = std::chrono::high_resolution_clock::now();
        double fftw_time1 = std::chrono::duration<double, std::milli>(end - start).count();

        fftw_destroy_plan(p1);
        fftw_free(in);
        fftw_free(out);

        results.push_back({N, customDCT2_time, fftw_time, fftw_time1});
        
        std::cout << "N=" << N << " | Implementazione custom: " << customDCT2_time << "ms | Implementazione FFTW con flag MEASURE: " 
                  << fftw_time << "ms | Implementazione FFTW con flag ESTIMATE: " 
                  << fftw_time1 << "ms\n";;
    }

    generatePlot(results);
}


int main(){

    //computationTest();

    customDCTScalingTest();

    FFTW_ESTIMATEScalingTest();
    FFTW_MEASUREScalingTest();

    implementationsComparison();

    return 0;
}