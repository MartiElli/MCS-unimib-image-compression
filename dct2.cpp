/**
 * =====================================================================================
 * METODI DEL CALCOLO SCIENTIFICO - PROGETTO 2
 * COMPRESSIONE DI IMMAGINI TRAMITE LA DCT2 (TIPO JPEG SEMPLIFICATO)
 * =====================================================================================
 * Standard di Compilazione: C++17
 * Dipendenze: Solo Standard Library (Header BMP nativo incluso)
 * =====================================================================================
 */


// blocco pragma push/pop per garantire che le struct siano allineate a 1 byte
// (push ,1 --> stack compilatore allinemento a 1 byte) e (pop --> ripristino allineamento precedente)
// i campi in memoria corrispondono esattamente ai dati nei file BMP 
#pragma pack(push, 1)

// queste 2 struct servono a mappare in memoria i metadati dei file BMP, 
// permettendo di leggere e scrivere correttamente le intestazioni dei file immagine.

// Strutture per la gestione dei file BMP (Header e Info)
struct BMPFileHeader {
    uint16_t fileType{0x4D42}; // "BM" in ASCII
    uint32_t fileSize{0};      // Dimensione totale del file in byte
    uint16_t reserved1{0};     // Riservato, deve essere 0
    uint16_t reserved2{0};     // Riservato, deve essere 0
    uint32_t offsetData{0};    // Offset in byte dall'inizio del file
};
// Struct per l'header informativo del BMP (DIB Header)
struct BMPInfoHeader {
    uint32_t size{0};            // Dimensione dell'header informativo (40 byte per BITMAPINFOHEADER)
    int32_t width{0};            // Larghezza dell'immagine in pixel
    uint16_t planes{1};          // Numero di piani (deve essere 1)
//    uint16_t bitCount{0};        // Numero di bit per pixel (es: 8 per toni di grigio, 24 per RGB)
//    uint32_t compression{0};     // Tipo di compressione (0 = nessuna, 1 = RLE 8-bit, 2 = RLE 4-bit, etc.)
    uint32_t sizeImage{0};       // Dimensione dell'immagine in byte (può essere 0 se non compressa)
    int32_t xPixelsPerMeter{0};
    int32_t yPixelsPerMeter{0};
    uint32_t colorsUsed{0};
    uint32_t colorsImportant{0};
};
#pragma pack(pop)

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>
// librerie di algebra 
#include <Eigen/Core>
#include <Eigen/Dense>
#include <fftw3.h>

// Definizione di una matrice di double per convenienza algoritmica
using Matrix = std::vector<std::vector<double>>;

// ─────────────────────────────────────────────────────────────────────────────────────
// SEZIONE 1: ALGORITMI DCT 1D E 2D (VERSIONE ORTOGONALE)
// ─────────────────────────────────────────────────────────────────────────────────────

/**
 * Calcola la DCT-II 1D ortogonale su un vettore di input di dimensione N.
 * Complessità temporale: O(N^2)
 */
void dct1D(const std::vector<double>& input, std::vector<double>& output) {
    size_t N = input.size();
    output.resize(N);
    double inv_sqrt_N = 1.0 / std::sqrt(static_cast<double>(N));
    double sqrt_2_N = std::sqrt(2.0 / static_cast<double>(N));

    for (size_t k = 0; k < N; ++k) {
        double sum = 0.0;
        for (size_t n = 0; n < N; ++n) {
            sum += input[n] * std::cos(M_PI * (2.0 * n + 1.0) * k / (2.0 * N));
        }
        double alpha = (k == 0) ? inv_sqrt_N : sqrt_2_N;
        output[k] = alpha * sum;
    }
}

/**
 * Calcola la IDCT-III 1D ortogonale su un vettore di input di dimensione N.
 * Complessità temporale: O(N^2)
 */
void idct1D(const std::vector<double>& input, std::vector<double>& output) {
    size_t N = input.size();
    output.resize(N);
    double inv_sqrt_N = 1.0 / std::sqrt(static_cast<double>(N));
    double sqrt_2_N = std::sqrt(2.0 / static_cast<double>(N));

    for (size_t n = 0; n < N; ++n) {
        double sum = 0.0;
        for (size_t k = 0; k < N; ++k) {
            double alpha = (k == 0) ? inv_sqrt_N : sqrt_2_N;
            sum += alpha * input[k] * std::cos(M_PI * (2.0 * n + 1.0) * k / (2.0 * N));
        }
        output[n] = sum;
    }
}

/**
 * Calcola la DCT2 2D applicando la DCT 1D separatamente su righe e colonne.
 * Complessità temporale per una matrice N x N: O(N^3)
 */
void dct2D_separable(const Matrix& input, Matrix& output) {
    size_t rows = input.size();
    size_t cols = input[0].size();
    output.assign(rows, std::vector<double>(cols, 0.0));

    // Trasformata lungo le righe
    Matrix temp(rows, std::vector<double>(cols, 0.0));
    for (size_t i = 0; i < rows; ++i) {
        dct1D(input[i], temp[i]);
    }

    // Trasformata lungo le colonne della matrice intermedia
    for (size_t j = 0; j < cols; ++j) {
        std::vector<double> col_in(rows), col_out(rows);
        for (size_t i = 0; i < rows; ++i) {
            col_in[i] = temp[i][j];
        }
        dct1D(col_in, col_out);
        for (size_t i = 0; i < rows; ++i) {
            output[i][j] = col_out[i];
        }
    }
}

/**
 * Calcola la IDCT2 2D applicando la IDCT 1D separatamente su colonne e righe.
 * Complessità temporale per una matrice N x N: O(N^3)
 */
void idct2D_separable(const Matrix& input, Matrix& output) {
    size_t rows = input.size();
    size_t cols = input[0].size();
    output.assign(rows, std::vector<double>(cols, 0.0));

    // Trasformata inversa lungo le colonne
    Matrix temp(rows, std::vector<double>(cols, 0.0));
    for (size_t j = 0; j < cols; ++j) {
        std::vector<double> col_in(rows), col_out(rows);
        for (size_t i = 0; i < rows; ++i) {
            col_in[i] = input[i][j];
        }
        idct1D(col_in, col_out);
        for (size_t i = 0; i < rows; ++i) {
            temp[i][j] = col_out[i];
        }
    }

    // Trasformata inversa lungo le righe
    for (size_t i = 0; i < rows; ++i) {
        idct1D(temp[i], output[i]);
    }
}

// ─────────────────────────────────────────────────────────────────────────────────────
// SEZIONE 2: I/O IMMAGINI IN FORMATO BMP (TONI DI GRIGIO 8-BIT)
// ─────────────────────────────────────────────────────────────────────────────────────

bool read_bmp_grayscale(const std::string& filename, std::vector<uint8_t>& data, int32_t& width, int32_t& height) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "[ERRORE] Impossibile aprire il file in lettura: " << filename << std::endl;
        return false;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    if (fileHeader.fileType != 0x4D42) {
        std::cerr << "[ERRORE] Il file non è un BMP valido." << std::endl;
        return false;
    }

    if (infoHeader.bitCount != 8) {
        std::cerr << "[ERRORE] Il programma supporta esclusivamente immagini BMP a 8-bit (toni di grigio)." << std::endl;
        return false;
    }

    width = infoHeader.width;
    height = std::abs(infoHeader.height); // Gestione altezza negativa (top-down)

    data.resize(width * height);
    file.seekg(fileHeader.offsetData, std::ios::beg);

    // Gestione del padding delle righe a 4 byte
    int rowPadding = (4 - (width * 1) % 4) % 4;
    
    if (infoHeader.height > 0) {
        // Bottom-up BMP
        for (int32_t i = height - 1; i >= 0; --i) {
            file.read(reinterpret_cast<char*>(&data[i * width]), width);
            file.seekg(rowPadding, std::ios::cur);
        }
    } else {
        // Top-down BMP
        for (int32_t i = 0; i < height; ++i) {
            file.read(reinterpret_cast<char*>(&data[i * width]), width);
            file.seekg(rowPadding, std::ios::cur);
        }
    }

    file.close();
    return true;
}

bool write_bmp_grayscale(const std::string& filename, const std::vector<uint8_t>& data, int32_t width, int32_t height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "[ERRORE] Impossibile aprire il file in scrittura: " << filename << std::endl;
        return false;
    }

    int rowPadding = (4 - (width * 1) % 4) % 4;
    uint32_t paletteSize = 256 * 4;
    uint32_t sizeImage = (width + rowPadding) * height;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + paletteSize;
    fileHeader.fileSize = fileHeader.offsetData + sizeImage;

    infoHeader.size = sizeof(BMPInfoHeader);
    infoHeader.width = width;
    infoHeader.height = height; // Salvataggio in modalità standard bottom-up
    infoHeader.bitCount = 8;
    infoHeader.sizeImage = sizeImage;
    infoHeader.colorsUsed = 256;

    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    // Scrittura della Palette dei colori (Toni di grigio R=G=B)
    for (int i = 0; i < 256; ++i) {
        uint8_t color[4] = { static_cast<uint8_t>(i), static_cast<uint8_t>(i), static_cast<uint8_t>(i), 0 };
        file.write(reinterpret_cast<const char*>(color), 4);
    }

    // Scrittura dati con padding bottom-up
    std::vector<uint8_t> paddingBytes(rowPadding, 0);
    for (int32_t i = height - 1; i >= 0; --i) {
        file.write(reinterpret_cast<const char*>(&data[i * width]), width);
        if (rowPadding > 0) {
            file.write(reinterpret_cast<const char*>(paddingBytes.data()), rowPadding);
        }
    }

    file.close();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────────────
// SEZIONE 3: CORE ENGINE DI COMPRESSIONE JPEG-LIKE
// ─────────────────────────────────────────────────────────────────────────────────────

void compress_image_pipeline(const std::vector<uint8_t>& src, std::vector<uint8_t>& dest, 
                             int32_t width, int32_t height, int F, int d) {
    
    int32_t blocks_x = width / F;
    int32_t blocks_y = height / F;
    
    // Inizializzazione destinazione con zeri (gli avanzi non mappati rimarranno neri o invariati)
    dest.assign(width * height, 0);

    // Ciclo sui macro-blocchi F x F
    for (int32_t by = 0; by < blocks_y; ++by) {
        for (int32_t bx = 0; bx < blocks_x; ++bx) {
            
            // 1. Estrazione del blocco f
            Matrix f(F, std::vector<double>(F, 0.0));
            for (int i = 0; i < F; ++i) {
                for (int j = 0; j < F; ++j) {
                    int32_t pixel_x = bx * F + j;
                    int32_t pixel_y = by * F + i;
                    f[i][j] = static_cast<double>(src[pixel_y * width + pixel_x]);
                }
            }

            // 2. Applicazione della DCT2
            Matrix c;
            dct2D_separable(f, c);

            // 3. Quantizzazione / Taglio frequenze (Soglia d)
            // Vincolo: k + l > d => coefficiente azzerato
            for (int k = 0; k < F; ++k) {
                for (int l = 0; l < F; ++l) {
                    if (k + l > d) {
                        c[k][l] = 0.0;
                    }
                }
            }

            // 4. Applicazione della IDCT2 Inversa
            Matrix ff;
            idct2D_separable(c, ff);

            // 5. Arrotondamento e Normalizzazione nel range [0, 255]
            for (int i = 0; i < F; ++i) {
                for (int j = 0; j < F; ++j) {
                    int32_t val = static_cast<int32_t>(std::round(ff[i][j]));
                    if (val < 0) val = 0;
                    if (val > 255) val = 255;

                    int32_t pixel_x = bx * F + j;
                    int32_t pixel_y = by * F + i;
                    dest[pixel_y * width + pixel_x] = static_cast<uint8_t>(val);
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────────────
// SEZIONE 4: UNIT TESTING E INTERFACCIA UTENTE UTENTE (MAIN)
// ─────────────────────────────────────────────────────────────────────────────────────

void execute_validation_test() {
    std::cout << "\n[TEST] Avvio Blocco Test di Validazione Specifica (8x8)..." << std::endl;
    std::vector<double> test_row = {231, 32, 233, 161, 24, 71, 140, 245};
    std::vector<double> row_out;
    dct1D(test_row, row_out);

    std::cout << "DCT1D Output riga (atteso confrontabile con specifica):" << std::endl;
    for (double val : row_out) {
        std::cout << std::scientific << std::setprecision(2) << val << " ";
    }
    std::cout << "\n[TEST] Completato.\n" << std::endl;
}

int main() {
    std::cout << "SISTEMA AUTOMATIZZATO COMPRESSIONE DCT2 - UNIMIB PROGETTO 2" << std::endl;

    // Esecuzione automatica del test di coerenza numerica richiesto
    execute_validation_test();

    std::string input_path;
    int F = 8;
    int d = 4;

    std::cout << "Inserire il percorso del file BMP (es: immagine.bmp): ";
    std::cin >> input_path;

    std::cout << "Inserire ampiezza della finestra F (es: 8, 16): ";
    std::cin >> F;

    std::cout << "Inserire soglia di taglio d (0 <= d <= " << (2 * F - 2) << "): ";
    std::cin >> d;

    if (d < 0 || d > (2 * F - 2)) {
        std::cerr << "[ERRORE] Parametro d fuori range applicabile. Termino." << std::endl;
        return 1;
    }

    std::vector<uint8_t> image_data;
    int32_t width = 0, height = 0;

    std::cout << "[INFO] Lettura file immagine..." << std::endl;
    if (!read_bmp_grayscale(input_path, image_data, width, height)) {
        return 1;
    }
    std::cout << "[INFO] Immagine caricata correttamente. Risoluzione: " << width << "x" << height << std::endl;

    std::vector<uint8_t> compressed_data;
    
    std::cout << "[INFO] Esecuzione pipeline di compressione..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    compress_image_pipeline(image_data, compressed_data, width, height, F, d);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "[INFO] Elaborazione completata in: " << diff.count() << " secondi." << std::endl;

    std::string output_path = "output_compressed_F" + std::to_string(F) + "_d" + std::to_string(d) + ".bmp";
    std::cout << "[INFO] Scrittura file compresso in: " << output_path << "..." << std::endl;
    
    if (!write_bmp_grayscale(output_path, compressed_data, width, height)) {
        return 1;
    }

    std::cout << "[SUCCESSO] Operazione conclusa senza anomalie strutturali." << std::endl;
    return 0;
}
