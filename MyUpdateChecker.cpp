#include "MyUpdateChecker.h"

//------------------------------------------------------------------------------

const QString MyUpdateChecker::kVersionUrl =
    "http://www.spellbyte.com/software/memoque/memoque_ver.txt";

const QString MyUpdateChecker::kUpdateUrl =
    "http://www.spellbyte.com/software/memoque/Update_Memoque.exe";


//------------------------------------------------------------------------------

MyUpdateChecker::MyUpdateChecker(QObject *parent) :
    QObject(parent)
{
    mNamChecker = new QNetworkAccessManager(this);

    connect(
        mNamChecker,
        SIGNAL(finished(QNetworkReply *)),
        this,
        SLOT(on_NetworkReply(QNetworkReply *))
    );
}

//------------------------------------------------------------------------------

MyUpdateChecker::~MyUpdateChecker()
{
    delete mNamChecker;
}

//------------------------------------------------------------------------------

void MyUpdateChecker::checkForUpdates()
{
    QUrl tUrl(kVersionUrl);

    mNamChecker->get(QNetworkRequest(tUrl));
}

//------------------------------------------------------------------------------

void MyUpdateChecker::checkVersion(QString inVersion)
{
    if (inVersion.length() >= 5)
    {
        int tVersionMajor =
            inVersion.mid(0, inVersion.length() - 4).toInt();
        int tVersionMimor =
            inVersion.mid(inVersion.length() - 4, 2).toInt();
        int tVersionBuild =
            inVersion.mid(inVersion.length() - 2, 2).toInt();

        if (inVersion.toInt() > MyQtUtility::instance().versionNumber())
        {
            QMessageBox msgBox;
            msgBox.setWindowModality(Qt::ApplicationModal);
            msgBox.setWindowTitle("Attenzione: Nuova versione disponibile.");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(
                QString
                (
                    "E' disponibile la nuova versione %1.%2.%3, "
                    "vuoi scaricarla?"
                )
                .arg(tVersionMajor)
                .arg(tVersionMimor)
                .arg(tVersionBuild)
            );
            msgBox.setInformativeText(
                QString
                (
                    "E' essenziale aggiornare il programma perchè la nuova "
                    "versione conterrà importanti migliorie e/o correzioni!"
                )
            );
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setButtonText(QMessageBox::Yes, "Si");
            msgBox.setButtonText(QMessageBox::No, "Non adesso");
            int tRet = msgBox.exec();

            if (tRet == QMessageBox::Yes)
            {
                downloadFile();
            }
        }
    }
}

//------------------------------------------------------------------------------

void MyUpdateChecker::downloadFile()
{
    QFileInfo tServerFileInfo(kUpdateUrl);
    QString tServerFileName = tServerFileInfo.fileName();

    if (tServerFileName.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Attenzione!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(
            tr(
                "Il file %1 non è presente sul server."
            ).arg(tServerFileName));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        return;
    }

    QString tLocalFileName =
        QDir::toNativeSeparators(
            QCoreApplication::applicationDirPath() + "/" + tServerFileName);

    if (QFile::exists(tLocalFileName))
    {
        QFile::remove(tLocalFileName);
    }

    mFile = new QFile(tLocalFileName);
    if (!mFile->open(QIODevice::WriteOnly))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Attenzione!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(
            tr(
                "Impossibile salvare il file %1: %2."
            ).arg(tLocalFileName).arg(mFile->errorString())
        );
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        delete mFile;

        return;
    }

    mProgressDialog = new QProgressDialog();
    mProgressDialog->setWindowModality(Qt::ApplicationModal);
    connect(mProgressDialog, SIGNAL(canceled()),
            this, SLOT(on_CancelDownload()));
    mProgressDialog->setWindowTitle(tr("Memoque: download aggiornamento."));
    mProgressDialog->setLabelText(tr("Download di %1.").arg(tServerFileName));

    mHttpRequestAborted = false;
    startRequest(kUpdateUrl);
}

//------------------------------------------------------------------------------

void MyUpdateChecker::startRequest(QUrl inUrl)
{
    mNamDownloader = new QNetworkAccessManager(this);

    mDownloaderReply = mNamDownloader->get(QNetworkRequest(inUrl));

    connect(mDownloaderReply, SIGNAL(finished()),
            this, SLOT(on_HttpFinished()));
    connect(mDownloaderReply, SIGNAL(readyRead()),
            this, SLOT(on_HttpDataRead()));
    connect(mDownloaderReply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(on_UpdateDataReadProgress(qint64, qint64)));
}

//------------------------------------------------------------------------------

void MyUpdateChecker::on_CancelDownload()
{
    qDebug() << "on_CancelDownload";

    mHttpRequestAborted = true;
    mDownloaderReply->abort();
}

//------------------------------------------------------------------------------

void MyUpdateChecker::on_HttpFinished()
{
    qDebug() << "on_HttpFinished";

    if (mHttpRequestAborted)
    {
        if (mFile)
        {
            mFile->close();
            mFile->remove();

            delete mFile;
        }

        mDownloaderReply->deleteLater();
        mProgressDialog->hide();

        return;
    }

    mProgressDialog->hide();
    mFile->flush();
    mFile->close();

    QVariant tRedirectionTarget =
        mDownloaderReply->attribute(
            QNetworkRequest::RedirectionTargetAttribute);

    if (mDownloaderReply->error())
    {
        mFile->remove();

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Attenzione!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(
            tr(
                "Download fallito: %1."
            ).arg(mDownloaderReply->errorString()));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
    else if (!tRedirectionTarget.isNull())
    {
        QUrl tUrl = QUrl(kUpdateUrl);
        QUrl tNewUrl = tUrl.resolved(tRedirectionTarget.toUrl());
        tUrl = tNewUrl;

        mFile->open(QIODevice::WriteOnly);
        mFile->resize(0);

        startRequest(tUrl);

        return;
    }
    else
    {
        QString tLocalFileName =
            QDir::toNativeSeparators(
                QCoreApplication::applicationDirPath() + "/" + QFileInfo(
                    QUrl(kUpdateUrl).path()).fileName());

        QDesktopServices::openUrl(QUrl::fromLocalFile(tLocalFileName));

        QApplication::quit();
    }

    mDownloaderReply->deleteLater();

    delete mFile;
}

//------------------------------------------------------------------------------

void MyUpdateChecker::on_HttpDataRead()
{
    if (mFile)
    {
        mFile->write(mDownloaderReply->readAll());
    }
}

//------------------------------------------------------------------------------

void MyUpdateChecker::on_UpdateDataReadProgress(
    qint64 inBytesRead,
    qint64 inTotalBytes)
{
    if (mHttpRequestAborted)
    {
        return;
    }

    mProgressDialog->setMaximum(inTotalBytes);
    mProgressDialog->setValue(inBytesRead);
}

//------------------------------------------------------------------------------

void MyUpdateChecker::on_NetworkReply(QNetworkReply *inReply)
{
    if (inReply->error() == QNetworkReply::NoError)
    {
        int tHttpStatusCode =
            inReply->attribute(
                QNetworkRequest::HttpStatusCodeAttribute).toUInt();

        if (tHttpStatusCode >= 200 && tHttpStatusCode < 300)
        {
            if (inReply->isReadable())
            {
                QString tReplyString =
                    QString::fromUtf8(inReply->readAll().data());

                checkVersion(tReplyString);
            }
        }
        else if (tHttpStatusCode >= 300 && tHttpStatusCode < 400)
        {
            // Get the redirection url
            QUrl tNewUrl =
                inReply->attribute(
                    QNetworkRequest::RedirectionTargetAttribute).toUrl();
            // Because the redirection url can be relative,
            // we have to use the previous one to resolve it
            tNewUrl = inReply->url().resolved(tNewUrl);

            QNetworkAccessManager *tManager = inReply->manager();
            QNetworkRequest tRedirection(tNewUrl);
            //            QNetworkReply *tNewReply = tManager->get(tRedirection);
            tManager->get(tRedirection);

            return; // to keep the manager for the next request
        }
        else
        {
            qDebug() << "Errore!";
        }
    }

    inReply->deleteLater();
}
