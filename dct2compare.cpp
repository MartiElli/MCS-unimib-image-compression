#include <iostream>
#include <Eigen/Core>
#include <fftw3.h>
#include <cmath>


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
    std::cout << "---------------" << std::endl;
    std::cout << "vettore da trasformare: " << std::endl << f;
    std::cout << "Matrice D: " << std::endl << D << std::endl;
    std::cout << "Vettore trasformato con DCT1:" << std::endl << D * f << std::endl;
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

    std::cout << "DCT1 SULLE RIGHE" << std::endl;   // stampa di test
    for(unsigned int i = 0; i < c.rows(); ++i){    // loop righe
        c.row(i) = DCT1(c.row(i));
    }

    std::cout << "DCT1 SULLE COLONNE" << std::endl;   // stampa di test
    for(unsigned int j = 0; j < c.cols(); ++j){    // loop colonne
        c.col(j) = DCT1(c.col(j));
    }

    return c;
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
    // RISULTATO: il primo elemento viene cambiato, gli altri diventano tutti 0. Troppo sospetto per essere semplicemente opera di D*f...
}


int main(){

    computationTest();
    return 0;
}
