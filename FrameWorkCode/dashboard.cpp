#include "dashboard.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "qfuturewatcher.h"
#include "ui_dashboard.h"
#include "crashlog.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QProcess>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QJsonParseError>
#include <QPushButton>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include "loadingspinner.h"
#include <Project.h>


/*!
 * \fn dashboard::dashboard
 * \brief This Class is used to preview the dashboard of Application.
 * \details
 * 1)Contains a Dialog Box which is a base class of dialog windows.
 * 2)Sets up the user interface for the specified widget.
 * 3)Includes a message handler which logs the crashlogs of application.
 * 4)Sets up window Title.
 * 5)Sets up html in text browser.
 * 6)Sets the range of SpinBox(The maximum elements textBrowser contains).
 * \param parent
 * \param s
 * \param max
 * \param repoMap
 */
dashboard::dashboard(QWidget *parent, QString s, int max, QMap<int, QString> repoMap, QString *p, bool * open) :
    QDialog(parent),
    ui(new Ui::dashboard)
{
    ui->setupUi(this);
    this->presentId.clear();
    this->btnMap.clear();

    ui->Import_btn->setStyleSheet("color : rgb(20,52,167); text-align : centre; background-color :rgb(229,228,226); border-radius :5px;");
    ui->Import_Open_btn->setStyleSheet("color : rgb(20,52,167); text-align : centre; background-color :rgb(229,228,226); border-radius :5px;");

    qInstallMessageHandler(crashlog::myMessageHandler);
    setWindowTitle("User Dashboard");
    this->repoMap = repoMap;

    ui->scrollArea->setWidget(ui->verticalLayout->widget());
    ui->verticalLayout->setAlignment(Qt::AlignLeft);

    int index = 0;
    QMapIterator<int,QString>i(repoMap);
    while(i.hasNext()){
        i.next();
        index = i.key();
        auto btn = new QPushButton();

        btn->setText(i.value());
        btn->setFixedHeight(50);
        btn->setMinimumWidth(ui->scrollArea->width());
        btn->setStyleSheet("color : black; text-align : left; padding : 10px;background-color :rgb(229,228,226); border-radius :5px;");
        connect(btn, &QPushButton::clicked , [this, index] {clicked(index);});
        this->btnMap[index] = btn;
        ui->verticalLayout->addWidget(btn);
    }

    this->p = p;
    this->toOpen = open;
}

/*!
 * \fn dashboard::~dashboard
 * \brief Deallocates the memory and cleans up class and it's object's.
 */
dashboard::~dashboard()
{
    delete ui;
}


/*!
 * \fn dashboard::on_pushButton_clicked
 * \inherits from dashboard, Executes when Clicked!
 * \brief
 * 1)Sets the value of spinBox to integer id.
 * 2)if there are no elemets in the textbrowser it returns or terminates the function.
 * 3)stores the filePath into String,if no filePath is Found,function terminates.
 * 4)Allows monitoring signals and slots by using QFuture and QFutureWatcher Classes.
 * 5)Conations Spinner which is displayed when importing is in process
 * 6)If the project is found already downloaded in same directory,throws an
 * error and terminates the function else the project is downloaded Successfully
 */
void dashboard::on_pushButton_clicked()
{
    QMapIterator<int,int>i(presentId);
    QString path, url_;
    bool ok;
    int ret;

    path = QFileDialog::getExistingDirectory(this, tr("Open Directory for importing all projects"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (path == "") {
        qDebug() << "User cancelled import #1";
        return;
    }


    int id = 0;
    while(i.hasNext()){
        i.next();
        if(i.value() == 1) id = i.key();
    }

    url_ = "https://github.com/UdaanContentForLogging/"+repoMap[id];
    if (url_.startsWith("git@github.com")) {
        qDebug() << "Not prepared to import using SSH. Please provide HTTPS URL";
        QMessageBox::information(this, "Use HTTPS URL", "This URL requires SSH key. Please provide HTTPS URL", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QFutureWatcher<int> watcher;
    connect(&watcher, &QFutureWatcher<int>::finished, this, &dashboard::stopSpinning);

    QFuture<int> t1 = QtConcurrent::run(Project::clone, QString(url_), QString(path));
    watcher.setFuture(t1);
    QString s = "Importing "+repoMap[id] + "...";

    spinner = new LoadingSpinner(this);
    spinner->SetMessage("Importing Set...", s);
    spinner->setModal(false);
    spinner->exec();

    if ((ret = t1.result()) != 0) {
        QMessageBox::information(this, "Error", ("The project is already downloaded\n" + repoMap[id]), QMessageBox::Ok, QMessageBox::Ok);
        qDebug()<<"Exited with return code"<<ret;
    } else {
        QMessageBox::information(this, "Successful", "Successfully imported the set", QMessageBox::Ok, QMessageBox::Ok);
        *p = path+"/"+repoMap[id]+"/project.xml";
    }

    dashboard::close();
}

/*!
 * \fn dashboard::stopSpinning
 * \brief This function stops the sppiner whenever called.
 */
void dashboard::stopSpinning()
{
    spinner->close();
    spinner->deleteLater();
}

/*!
 * \fn dashboard::clicked
 * \brief This function sets index for each button's onClick slot
 * \param integer index of button
*/
void dashboard::clicked(int index){
    QPushButton* btn = this->btnMap[index];
    if(this->presentId[index] == 0){
        this->presentId[index] = 1;
        this->totalClickedBooks++;
        btn->setStyleSheet("color : black; text-align : left; padding : 10px;background-color :rgb(137,207,240); border-radius :5px;");
    }
    else{
        this->presentId[index] = 0;
        this->totalClickedBooks--;
        btn->setStyleSheet("color : black; text-align : left; padding : 10px;background-color :rgb(229,228,226); border-radius :5px;");
    }
}

/*!
 * \fn dashboard::on_pushButton_2_clicked
 * \brief This function will give info while clicking 'i' button
*/
void dashboard::on_pushButton_2_clicked()
{
    QMessageBox * msg = new QMessageBox();

    msg->setText("* All your books will appear here.\n* You can save changes to cloud to these books only.\n* Click on book name to import it locally.");
    msg->exec();
}


void dashboard::on_Import_Open_btn_clicked()
{
    if(this->totalClickedBooks == 1){
        on_pushButton_clicked();
        * (this->toOpen) = true;
    }
    else{
        QMessageBox * errMsg = new QMessageBox();
        errMsg->setText("Please select any one book");
        errMsg->exec();
    }
}


void dashboard::on_Import_btn_clicked()
{
    if(this->totalClickedBooks == 1){
        on_pushButton_clicked();
    }
    else{
        QMessageBox * errMsg = new QMessageBox();
        errMsg->setText("Please select any one book");
        errMsg->exec();
    }
}

