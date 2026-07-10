#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow> // finestra principale con strutture principali
#include <QLabel>      // label che mostra testo/immagini
#include <QSpinBox>    // selettore x (F, d) --> setRange/Value(), value()
#include <QPushButton> // button x "CaricaBMP"/"Comprimi" --> clicked() --> slots
#include <QPainter>    // x disegnare su QImage
#include <QImage>      // immagine come griglia di pixel in memoria --> rw su pixels
                       // x visualizzarla convertita in QPixmap e messa in QLabel

/**
 * @file mainwindow.h
 * @brief Interfaccia grafica per la compressione di immagini in scala di grigi tramite DCT2.
 */

/**
 * @brief Finestra principale dell'applicazione: permette di caricare un'immagine .bmp
 * in scala di grigi, impostare i parametri F (dimensione blocco) e d (soglia di taglio
 * delle frequenze), ed eseguire la compressione tramite DCT2 a blocchi, visualizzando
 * affiancate l'immagine originale e quella compressa.
 */
class MainWindow : public QMainWindow {
    // macro x qualsiasi classe con slot/signals
    Q_OBJECT

public:

    /**
     * @brief Costruisce la finestra principale e inizializza l'interfaccia (bottoni, spin box, aree immagine).
     *  1 parametro --> explicit x evitare conversioni implicite
     * @param parent widget genitore (Qt), nullptr se finestra radice
     */
    explicit MainWindow(QWidget *parent = nullptr);

// slot che rispondono agli eventi
private slots:
    /**
     * @brief Apre un file dialog per scegliere un'immagine .bmp, la converte in scala di grigi
     * (Format_Grayscale8) e aggiorna la visualizzazione.
     */
    void loadImage();

    /**
     * @brief Esegue la compressione DCT2 a blocchi sull'immagine caricata: 
     * per ogni blocco F×F applica DCT2 (FFTW), azzera le frequenze con k+l >= d, applica l'IDCT2, arrotonda
     * e limita i valori a [0,255], ricompone l'immagine e la salva in compressed/.
     */
    void compress();

private:
    /**
     * @brief Aggiorna il range ammissibile di d (0 ... 2F-2) quando F cambia,
     * clampando il valore corrente di d se supera il nuovo massimo.
     */
    void updateDRange();

    /**
     * @brief Ridisegna su schermo l'immagine originale e quella compressa (se presenti),
     * scalandole mantenendo le proporzioni.
     */
    void displayImages();

    /**
     * @brief Estrae i pixel di un blocco quadrato di immagine (Format_Grayscale8) in una matrice.
     * @param block reference al blocco, non modificabile
     * @param blockSize dimensione del blocco
     * @param matrix puntatore alla matrice dove inserire i coefficienti estratti
     */
    void extractPixels(const QImage& block, int blockSize, double* matrix);

    QLabel *originalLabel;        ///< label con l'immagine originale
    QLabel *compressedLabel;      ///< label con l'immagine compressa
    QPushButton *loadButton;      ///< avvia loadImage()
    QPushButton *compressButton;  ///< avvia compress()
    QSpinBox *fSpinBox;           ///< campo numerico F (dimensione blocco)
    QSpinBox *dSpinBox;           ///< campo numerico d (soglia taglio frequenze)
    QLabel *statusLabel;          ///< barra di stato in basso

    QImage originalImage;         ///< copia dell'immagine originale caricata
    QImage compressedImage;       ///< copia dell'immagine dopo la compressione
    QString imagePath;            ///< percorso del file caricato
};

#endif
