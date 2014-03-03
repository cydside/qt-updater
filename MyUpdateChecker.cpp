#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QtWidgets>
#include <QtNetwork>

class MyUpdateChecker : public QObject
{
        Q_OBJECT

    public:
        static const QString kVersionUrl;
        static const QString kUpdateUrl;

        explicit MyUpdateChecker(QObject *parent = 0);
        ~MyUpdateChecker();

        void checkForUpdates();

    public slots:
        void on_NetworkReply(QNetworkReply *inReply);
        void on_CancelDownload();
        void on_HttpFinished();
        void on_HttpDataRead();
        void on_UpdateDataReadProgress(qint64 inBytesRead, qint64 inTotalBytes);

    private:
        void checkVersion(QString inVersion);
        void startRequest(QUrl inUrl);
        void downloadFile();

        QProgressDialog *mProgressDialog;
        QNetworkAccessManager *mNamChecker;
        QNetworkAccessManager *mNamDownloader;
        QNetworkReply *mDownloaderReply;
        QFile *mFile;
        bool mHttpRequestAborted;

};

#endif // UPDATECHECKER_H
