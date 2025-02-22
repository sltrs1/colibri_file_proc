#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , startFolderPath("")
    , inputFileMask("*")
    , outFolderPath("")
    , rewrite(false)
    , repeat(false)
    , deleteAfterProc(false)
    , hexMask()
{
    ui->setupUi(this);

    timer = new QTimer(this);

    connect(ui->pushButtonSelectStartFolder, &QPushButton::clicked, this, &MainWindow::onSelectStartFolder);
    connect(ui->pushButtonSelectOutFilePath, &QPushButton::clicked, this, &MainWindow::onSelectOutFolder);

    connect(ui->pushButtonStartProcessing, &QPushButton::clicked, this, &MainWindow::onStartProcessing);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
}

void MainWindow::onSelectStartFolder()
{
    startFolderPath = QFileDialog::getExistingDirectory(this, "Select Start Folder").toStdString().c_str();
    ui->labelSelectedStartFolder->setText(startFolderPath);
}

void MainWindow::onSelectOutFolder()
{
    outFolderPath = QFileDialog::getExistingDirectory(this, "Select Output Folder").toStdString().c_str();
    ui->labelSelectedOutFolder->setText(outFolderPath);
}


void MainWindow::onStartProcessing()
{
    if (startFolderPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "No starting folder provided!");
        return;
    }

    inputFileMask = ui->lineEditInputFileMask->text();

    if (outFolderPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "No output folder provided!");
        return;
    }

    rewrite = ui->checkBoxRewrite->isChecked();
    deleteAfterProc = ui->checkBoxDelete->isChecked();

    QString hexInput = ui->lineEditHexMask->text();
    if (isValidHex(hexInput))
    {
        hexMask = QByteArray::fromHex(hexInput.toUtf8());
    }
    else
    {
        QMessageBox::warning(this, "Error", "Invalid mask!");
    }

    repeat = ui->checkBoxRepeat->isChecked();
    int interval = ui->spinBoxTimer->value();

    timer->stop();
    disconnect(timer, &QTimer::timeout, this, &MainWindow::runProcessing);

    if (repeat)
    {
        connect(timer, &QTimer::timeout, this, &MainWindow::runProcessing);
        timer->start(interval);
    }
    else
    {
        runProcessing();
    }

}

void MainWindow::runProcessing()
{
    QDir dir(startFolderPath);
    QStringList files = dir.entryList(QStringList() << inputFileMask, QDir::Files);

    for (const QString &file : files)
    {
        processFile(dir.filePath(file));
    }
}


void MainWindow::processFile(const QString& filename)
{
    size_t byteCnt = 0;

    QString outputFilePath = generateOutFileName(filename);

    QFile inputFile(filename);
    QFile outputFile(outputFilePath);

    if (isFileOpen(filename))
    {
        qWarning() << "File " << filename << " is already opened by someone - skipping it\n";
        return;
    }
    else
    {
        if (!inputFile.open(QIODevice::ReadOnly))
        {
            qWarning() << "Failed to open input file:" << inputFile.errorString();
            return;
        }

        if (!outputFile.open(QIODevice::WriteOnly))
        {
            qWarning() << "Failed to open output file:" << outputFile.errorString();
            inputFile.close();
            return;
        }

        while (!inputFile.atEnd())
        {
            uchar byte;
            inputFile.getChar(reinterpret_cast<char*>(&byte)); // Read one byte

            uchar maskedByte = byte & hexMask[(byteCnt++) / 8];
            outputFile.putChar(static_cast<char>(maskedByte));

        }

        inputFile.close();
        outputFile.close();

        if (deleteAfterProc)
        {
            if (!inputFile.remove())
            {
                qWarning() << "Failed to delete input file:" << inputFile.errorString();
            }
        }

    }

}

bool MainWindow::isFileOpen(const QString& filename) const
{
    HANDLE file = CreateFileW(filename.toStdWString().c_str(), GENERIC_READ, 0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_SHARING_VIOLATION) {
            return true;
        }
        return false;
    }
    CloseHandle(file);
    return false;
}

bool MainWindow::isValidHex(const QString &hex) const
{
    return hex.length() == 16 && QRegularExpression("^[0-9A-Fa-f]*$").match(hex).hasMatch();
}

QString MainWindow::generateOutFileName(const QString& inputFileName) const
{
    QFileInfo fileInfo(inputFileName);
    QString directory = outFolderPath;
    QString baseName = fileInfo.baseName();
    QString extension = fileInfo.suffix();

    QString uniqueFileName = baseName + (extension.isEmpty() ? "" : "." + extension);
    QString fullFilePath = QDir(directory).filePath(uniqueFileName);
    int counter = 1;

    if (!rewrite)
    {
        while (QFile::exists(fullFilePath))
        {
            uniqueFileName = baseName + "_" + QString::number(counter) + (extension.isEmpty() ? "" : "." + extension);
            fullFilePath = QDir(directory).filePath(uniqueFileName);
            counter++;
        }
    }

    return fullFilePath;
}
