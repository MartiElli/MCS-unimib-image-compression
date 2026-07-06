#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Compressione DCT");                              // titolo MainWindow
    resize(1200, 700);                                          // dimensione MainWindow

    auto *central = new QWidget(this);            // crea QWidget generico, parent MainWindow
    setCentralWidget(central);                               // imposta widget al centro

    // button per caricare/comprimere immagine
    loadButton = new QPushButton("Carica BMP");
    compressButton = new QPushButton("Comprimi");
    compressButton->setEnabled(false);                              // disabled finché non carico immagine

    // F: dimensione blocco 
    auto *fLabel = new QLabel("F (dim. blocco):");     // label
    fSpinBox = new QSpinBox();                                        // campo numerico
    fSpinBox->setRange(2, 256);                              // range 2 - 256
    fSpinBox->setValue(8);                                       // val iniziale 8

    // d: soglia taglio
    auto *dLabel = new QLabel("d (soglia taglio):");   // label
    dSpinBox = new QSpinBox();                                        // campo numerico
    dSpinBox->setRange(0, 14);                               // 2F-2 = 14 per F=8
    dSpinBox->setValue(7);                                       // inizialmente metà range

    // status label iniziale
    statusLabel = new QLabel("Nessuna immagine caricata");    

    // layout orizzontale x widget
    // i widget elencati sono in fila da sx a dx
    auto *controls = new QHBoxLayout();
    controls->addWidget(loadButton);
    controls->addWidget(fLabel);
    controls->addWidget(fSpinBox);
    controls->addWidget(dLabel);
    controls->addWidget(dSpinBox);
    controls->addWidget(compressButton);
    // spazio finale per disporli bene
    controls->addStretch();

    // area immagine originale
    originalLabel = new QLabel("Originale");
    originalLabel->setAlignment(Qt::AlignCenter);
    //originalLabel->setMinimumSize(400, 400);
    originalLabel->setStyleSheet("border: 1px solid gray;");

    // area immagine compressa
    compressedLabel = new QLabel("Compressa");
    compressedLabel->setAlignment(Qt::AlignCenter);
    //compressedLabel->setMinimumSize(400, 400);
    compressedLabel->setStyleSheet("border: 1px solid gray;");

    // altro layout orizzontale x label widget: originale - compressa
    auto *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(originalLabel);
    imageLayout->addWidget(compressedLabel);

    // layout verticale principale x central widget
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(controls);                // buttons + spinbox
    mainLayout->addLayout(imageLayout, 1); // immagini 
    mainLayout->addWidget(statusLabel);                    // status label in basso

    // connette button -> signal -> slot
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(compressButton, &QPushButton::clicked, this, &MainWindow::compress);
    connect(fSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        updateDRange();
    });
}

// update 'd' range based on new 'F'
void MainWindow::updateDRange() {
    int F = fSpinBox->value();
    int maxD = 2 * F - 2;
    dSpinBox->setMaximum(maxD);
    if (dSpinBox->value() > maxD)
        dSpinBox->setValue(maxD);
}

// carica immagine, prende solo .bmp
void MainWindow::loadImage() {
    QString path = QFileDialog::getOpenFileName(this, "Scegli immagine BMP",
                                                 QString(), "BMP (*.bmp)");
    if (path.isEmpty()) return; // se annullo
    
    QImage img(path); // carica .bmp in QImage
    if (img.isNull()) {        // se fallisce caricamento
        QMessageBox::warning(this, "Errore", "Impossibile caricare l'immagine.");
        return;
    }
    // converte formato dei pixel, 
    // anche se accetta solo formato .bmp sanifica immagine
    // scala grigi 8 bit
    originalImage = img.convertToFormat(QImage::Format_Grayscale8);
    imagePath = path;
    // immagine caricata --> enable compress
    compressButton->setEnabled(true);
    // status: path, dimensioni
    statusLabel->setText(QString("Caricata: %1 (%2x%3)")
                             .arg(path)
                             .arg(originalImage.width())
                             .arg(originalImage.height()));
    // update display immagini                          
    displayImages();
}


void extractPixels(const QImage& block, int blockSize, double* matrix){

    for (int y = 0; y < block.height(); y++) {
        for (int x = 0; x < block.width(); x++) {
            unsigned char pixel = block.pixelIndex(x, y);
            matrix[y * blockSize + x] = (double)pixel;
        }
    }
}


// comprime immagine
void MainWindow::compress() {
    // guardia presenza immagine
    if (originalImage.isNull()) return;

    int F = fSpinBox->value();      // legge F
    int d = dSpinBox->value();      // legge d
    int w = originalImage.width();  // legge larghezza imm
    int h = originalImage.height(); // legge altezza imm
    // calcolo # blocchi FxF ci stanno
    // divisione intera scarta pixel avanzati
    // --> nested for x scorrere blocchi su bX e bY
    int blocksX = w / F;    // # blocchi in orizzontale
    int blocksY = h / F;    // # blocchi in verticale
    // copia troncata su pixel avanzo da bX (a destra) e da bY (in basso)
    compressedImage = originalImage.copy(0, 0, blocksX * F, blocksY * F);

    // creazione array contenente i blocchi in cui l'immagine viene suddivisa
    QVector<QImage> blocks(blocksX * blocksY);
    // partendo da (0,0) itero una riga alla volta -> (x + F, y)
    // poi passo alla riga sopra (incremento y di F) e itero nuovamente
    for(unsigned int y = 0; y < compressedImage.height(); y += F){   // spostamento sulle righe dal basso verso l'alto

        for(unsigned int x = 0; x < compressedImage.width(); x += F){   // spostamento dentro la riga da sx a dx
            int blockWidth = qMin(F, compressedImage.width() - (int)x);
            int blockHeight = qMin(F, compressedImage.height() - (int)y);
        
            QRect blockRect(x, y, blockWidth, blockWidth);  // ritaglio la forma del blocco
            QImage block = compressedImage.copy(blockRect); // estraggo il blocco dall'immagine come copia
            blocks.append(block);
        }
    }

    // allocazione spazio per input a FFTW
    double* inputMatrix = (double*)fftw_malloc(F * F * sizeof(double));

    // per ogni blocco F×F:
    for(const QImage &block : blocks){

        //memset(inputMatrix, 0, F * F * sizeof(double)); // resetto il contenuto dello spazio di memoria (secondo me è inutile)

        extractPixels(block, F, inputMatrix);   // 1. estraggo i pixel in una matrice
        // 2. applicare DCT2 (FFTW)
        // 3. azzerare frequenze con k+l >= d
        // 4. applicare IDCT2
        // 5. round + clamp [0,255]
        // 6. riscrivere i pixel nel blocco
    }

    // ricomposizione immagine
    //QImage recomposed(originalImage.width(), originalImage.height(), QImage::Format_RGB32);
    QPainter painter(&compressedImage);

    int blockIndex = 0;
    for (int y = 0; y < originalImage.height(); y += F) {
        for (int x = 0; x < originalImage.width(); x += F) {
            if (blockIndex < blocks.size()) {
                painter.drawImage(x, y, blocks[blockIndex]);
                blockIndex++;
            }
        }
    }
    painter.end();

    // aggiorna status
    statusLabel->setText(QString("Compressione: F=%1, d=%2, %3x%4 blocchi (TODO: DCT)")
                             .arg(F).arg(d).arg(blocksX).arg(blocksY));
    // update display imm
    displayImages();
}

//--------- display immagini: --------------------
// QImage --> QPixmap: gestito da QLabel a schermo
// scala x dimensione label,
// KeepAspectRatio mantiene proporzioni,
// SmoothTransformation sfuma i pixel
void MainWindow::displayImages() {
    // display imm originale
    if (!originalImage.isNull()) {
        QPixmap orig = QPixmap::fromImage(originalImage).scaled(
            originalLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        originalLabel->setPixmap(orig);
    }
    // display imm compressa
    if (!compressedImage.isNull()) {
        QPixmap comp = QPixmap::fromImage(compressedImage).scaled(
            compressedLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        compressedLabel->setPixmap(comp);
    }
}
