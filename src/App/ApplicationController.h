#pragma once

#include <QObject>
#include <QString>

class ApplicationController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString documentName READ documentName NOTIFY documentNameChanged)

public:
    explicit ApplicationController(QObject *parent = nullptr);

    QString documentName() const;

    Q_INVOKABLE void newDocument();

signals:
    void documentNameChanged();

private:
    void setDocumentName(const QString &name);

    QString m_documentName;
};
