#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QUrl>
#include <QtNetwork>
#include <QXmlQuery>

#include<QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QAbstractButton::clicked,this,&MainWindow::fetch);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fetch()
{
    ui->pushButton->setEnabled(false);
    const QString urlSpec = "https://diablo.fandom.com/wiki/Season";
    if (urlSpec.isEmpty())
        return;

    const QUrl newUrl = QUrl::fromUserInput(urlSpec);

    url = newUrl;

    reply = qnam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &MainWindow::fetchResult);

}
void MainWindow::fetchResult(){

    if (reply->error()) {
        reply->deleteLater();
        resultString = reply->errorString();
        ui->textEdit->setText(resultString);
        return;
    }

    QString result = reply->readAll();
    reply->deleteLater();
    reply = nullptr;

    stripData(result);
}
void MainWindow::stripData(QString htmlData)
{

    QString result;

    result = htmlData.mid(htmlData.indexOf("<table"), htmlData.indexOf("</tr></table>") - htmlData.indexOf("<table"));

    QXmlStreamReader xml(result);
    QString textString;
    bool fistLine = true; // we dont need the columns

    while (!xml.atEnd()) {
        if ( xml.readNext() == QXmlStreamReader::Characters && !fistLine) {
            textString += xml.text();
        }
        fistLine = false;
    }

    QStringList numberStringList;
    numberStringList = textString.split(QRegExp("[\n]"),QString::SkipEmptyParts);

    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    ui->tableWidget->setColumnCount(2);

    QVector<QDate> dateVector;
    QString format = "dd MMM yyyy";
    QDate timeParse;

    int i = 5;
    int col = 0;

    QStringList parsedStringList;

    while(i < numberStringList.length())
    {
        if(numberStringList[i].trimmed() != nullptr && i %4 !=0){

            QStringRef strippedData(&numberStringList[i],0,numberStringList[i].indexOf('['));
            QString strippedStringData = strippedData.toString().simplified();

            //timeParse = QDate::fromString(strippedStringData,format); ONLY WORKS IF SYSTEM'S LOCAL LANGUAGE IS ENGLISH

            QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates)); //LOCALISATION INDEPENDENT SOLUTION
            timeParse = QLocale().toDate(strippedStringData,format);

            if(timeParse.isValid())
                dateVector.append(timeParse);

            parsedStringList.append(strippedStringData);

        }

        if(i % 2 == 0 )
        {
            i+=3;
        }
        else
            i++;
    }

    // create table row/col and fill content
    col = 0;
    i = 0;

    while(i < parsedStringList.length())
    {
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,
                                 col,
                                 new QTableWidgetItem(parsedStringList[i]));

        col++;
        i++;

        if(col > 1 && i < parsedStringList.length()){
            col = 0;
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        }

    }

    QString outputString;
    if(parsedStringList[parsedStringList.length()-1].trimmed() != "TBD")
    { // the season did not start yet, we dont calculate end date

        outputString.append("Season end date with the addition of 90 days: " + dateVector[dateVector.length()-1].addDays(90).toString());

        qDebug() << " this :    " << dateVector[dateVector.length()-1].toString();

    }else
    {
        outputString.append("Season did not start yet!");

        // calculate estimated start date
        outputString.append("\nSeason start date estimated: " + calculateSeasonStartDate(dateVector).toString());

    }
    ui->textEdit->setText(outputString);
    ui->pushButton->setEnabled(true);

}

QDate MainWindow::calculateSeasonStartDate(QVector<QDate> dateVector){

    // calculate average days between season starts
    // add calculated day to the result

    QDate result = dateVector[dateVector.length()-1]; // get the last season's end date

    //calculate average days between each season end - next season start
    int j = 0;
    qint64 sumDays = 0;

    for(int i = 1; i< dateVector.length()-1; i+=2){
        sumDays += dateVector[i].daysTo(dateVector[i+1]);
        j++;
    }
    // add average days to the last season's end date to get the estimated next season start date
    return result.addDays(sumDays/j);
}

