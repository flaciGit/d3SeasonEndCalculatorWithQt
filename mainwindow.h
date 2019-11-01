#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//#include "fetchhtml.h"

#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QProgressDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString resultString;
    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;

    void fetch();
    void fetchResult();
    void stripData(QString htmlData);
    QDate calculateSeasonStartDate(QVector<QDate> dateVector);

};

#endif // MAINWINDOW_H
