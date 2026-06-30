#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow> // finestra principale con strutture principali
#include <QLabel>      // label che mostra testo/immagini
#include <QSpinBox>    // selettore x (F, d) --> setRange/Value(), value()
#include <QPushButton> // button x "CaricaBMP"/"Comprimi" --> clicked() --> slots
#include <QImage>      // immagine come griglia di pixel in memoria --> rw su pixels
                       // x visualizzarla convertita in QPixmap e messa in QLabel

class MainWindow : public QMainWindow {
    // macro x qualsiasi classe con slot/signals
    Q_OBJECT

public:
    // costruttore con 1 parametro 
    // --> explicit x evitare conversioni implicite
    // parent nullptr, finestra root
    explicit MainWindow(QWidget *parent = nullptr);

// slot che rispondono agli eventi
private slots:
    void loadImage();     // apre file dialog x caricare BMP
    void compress();      // esegue compressione DCT

private:
    void updateDRange();  // update 'd' quando cambio 'F' (0 ... 2F-2)
    void displayImages(); // ridisegna le immagini su schermo

    QLabel *originalLabel;        // label con bmp originale
    QLabel *compressedLabel;      // label con bmp compresso
    QPushButton *loadButton;      // --> loadImage()
    QPushButton *compressButton;  // --> compress()
    QSpinBox *fSpinBox;           // campo numerico F
    QSpinBox *dSpinBox;           // campo numerico d
    QLabel *statusLabel;          // barra di stato in basso

    QImage originalImage;         // copia immagine originale
    QImage compressedImage;       // copia immagine compressa
    QString imagePath;            // filepath caricato
};

#endif
