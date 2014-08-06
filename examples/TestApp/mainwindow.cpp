#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QsrMailTransport>
#include <QsrMailTransaction>
#include <QsrMailMessage>
#include <QsrMailMimePart>
#include <QsrMailMimeMultipart>
#include <QsrMailAddress>

#include <QSslConfiguration>
#include <QSslCipher>
#include <QDateTime>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    attachments = new FileListModel(this);

    ui->setupUi(this);
    ui->lstAttachment->setModel(attachments);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::transactionComplete()
{
    QsrMailTransaction *o = qobject_cast<QsrMailTransaction *>(QObject::sender());

    QString message;

    message += "messageComplete:\n";
    message += "  messageId:     "
            + o->message().messageId()
            + "\n";
    message += "  error:         "
            + QString::number(o->error())
            + "\n";
    message += "  errorText:     "
            + o->errorText()
            + "\n";
    message += "  status:        "
            + QString::number(o->status())
            + "\n";
    message += "  statusText:    "
            + o->statusText()
            + "\n";
    message += "  encrypted:     "
            + QByteArray(o->isEncrypted() ? "Yes\n" : "No\n");

    if (o->isEncrypted()) {
        QSslConfiguration cfg(o->sslConfiguration());
        message += "  local cert:    "
                + cfg.localCertificate().subjectInfo(QSslCertificate::CommonName).value(0)
                + "\n";
        message += "  peer cert:     "
                + cfg.peerCertificate().subjectInfo(QSslCertificate::CommonName).value(0)
                + "\n";
        message += "  cipher:        "
                + cfg.sessionCipher().name()
                + "\n";
    }

    message += "  authenticated: "
            + QByteArray(o->isAuthenticated() ? "Yes\n" : "No\n");

    switch (o->authMech()) {
    case QsrMailTransport::DisabledMech:
        message += "  authMech:      disabled\n";
        break;
    case QsrMailTransport::CramMd5Mech:
        message += "  authMech:      CRAM-MD5\n";
        break;
    case QsrMailTransport::LoginMech:
        message += "  authMech:      LOGIN\n";
        break;
    case QsrMailTransport::PlainMech:
        message += "  authMech:      PLAIN\n";
        break;
    case QsrMailTransport::AutoSelectMech:
        /* should not be seen */
        message += "  authMech:      (autoselect)\n";
        break;
    }

    message += "  username:      "
            + o->username()
            + "\n";

    message += "\n";

    ui->txtLog->appendPlainText(message);

    o->deleteLater();
}

void MainWindow::on_btnAdd_clicked()
{
    QFileDialog dialog(this, "Select attachment");
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setViewMode(QFileDialog::Detail);
    if (dialog.exec() && !dialog.selectedFiles().isEmpty()) {
        attachments->insertRows(0, dialog.selectedFiles().size());
        for (int i=0, size=dialog.selectedFiles().size(); i<size; i++) {
            attachments->setData(attachments->index(i),
                                 dialog.selectedFiles().at(i));
        }
    }
}

void MainWindow::on_btnDelete_clicked()
{
    QModelIndexList rows = ui->lstAttachment->selectionModel()->selectedRows();
    if (!rows.isEmpty())
        attachments->removeRow(rows.first().row());
}

void MainWindow::on_btnSend_clicked()
{
    /* create a transport */
    QsrMailTransport *transport = new QsrMailTransport(this);
    connect(transport, &QsrMailTransport::progressUpdate,
            ui->totalProgress , &QProgressBar::setValue);

    /* set tls */
    switch (ui->cbxTLS->currentIndex()) {
    case 0:
        transport->setTlsLevel(QsrMailTransport::TlsDisabled);
        break;
    case 1:
        transport->setTlsLevel(QsrMailTransport::TlsOptional);
        break;
    case 2:
        transport->setTlsLevel(QsrMailTransport::TlsRequired);
        break;
    }

    /* set auth */
    switch (ui->cbxAuth->currentIndex()) {
    case 0:
        transport->setAuthMech(QsrMailTransport::DisabledMech);
        break;
    case 1:
        transport->setAuthMech(QsrMailTransport::AutoSelectMech);
        break;
    case 2:
        transport->setAuthMech(QsrMailTransport::CramMd5Mech);
        break;
    case 3:
        transport->setAuthMech(QsrMailTransport::LoginMech);
        break;
    case 4:
        transport->setAuthMech(QsrMailTransport::PlainMech);
        break;
    }

    /* set user credentials */
    transport->setUser(ui->txtUsername->text());
    transport->setPassword(ui->txtPassword->text());

    /* disable ssl verification */
    QSslConfiguration cfg(QSslConfiguration::defaultConfiguration());
    cfg.setPeerVerifyMode(QSslSocket::VerifyNone);
    transport->setSslConfiguration(cfg);

    /* build the message */
    QsrMailMessage message;
    message.setFrom(QsrMailAddress(ui->txtFrom->text()));
    message.setTo(QsrMailAddress(ui->txtTo->text()));
    message.setDate(QDateTime::currentDateTime());
    message.setSubject(ui->txtSubject->text());

    QsrMailMimeMultipart mp;

    /* main body */
    QsrMailMimePart body;
    body.setContentType("text/html; charset=UTF-8");
    body.setBody(ui->txtBody->document()->toHtml().toUtf8());
    mp.append(body);

    /* attachments */
    foreach (const QFileInfo &file, attachments->fileList())
        mp.append(QsrMailMimePart::fromFile(new QFile(file.absoluteFilePath())));

    /* set the multipart body */
    message.setBody(mp);

    /* queue the message */
    QsrMailTransaction *transaction = transport->queueMessage(message);
    connect(transaction, SIGNAL(finished()), this, SLOT(transactionComplete()));

    /* send it */
    transport->sendMessages(ui->txtServerName->text(),
                            ui->txtServerPort->text().toInt(),
                            QAbstractSocket::IPv4Protocol);
}
