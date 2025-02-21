#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpression>
#include <QByteArray>

#include <Windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void onSelectStartFolder();
    void onSelectOutFolder();

    void onStartProcessing();
    void runProcessing();

private:
    bool isFileOpen(const QString& filename) const;
    bool isValidHex(const QString& hex) const;
    QString generateOutFileName(const QString& inputFileName) const;
    void processFile(const QString& filename);

private:
    Ui::MainWindow *ui;

    QTimer * timer;

    QString startFolderPath;
    QString inputFileMask;
    QString outFolderPath;
    bool rewrite;
    QByteArray hexMask;
    bool repeat;
};
#endif // MAINWINDOW_H
