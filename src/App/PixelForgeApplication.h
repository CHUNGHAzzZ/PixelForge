#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

namespace PixelForge {

class PixelForgeApplication final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString documentName READ documentName NOTIFY documentNameChanged)

public:
    explicit PixelForgeApplication(QObject *parent = nullptr);

    QString documentName() const;

    Q_INVOKABLE void newDocument();
    Q_INVOKABLE void importFile(const QUrl &fileUrl);

signals:
    void documentNameChanged();
    void fileImported(const QUrl &fileUrl);

private:
    void setDocumentName(const QString &name);

    QString m_documentName;
};

}
