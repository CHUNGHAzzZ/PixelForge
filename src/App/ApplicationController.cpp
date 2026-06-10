#include "ApplicationController.h"

#include <QFileInfo>

ApplicationController::ApplicationController(QObject *parent)
    : QObject(parent)
    , m_documentName(QStringLiteral("Untitled Canvas"))
{
}

QString ApplicationController::documentName() const
{
    return m_documentName;
}

void ApplicationController::newDocument()
{
    setDocumentName(QStringLiteral("Untitled Canvas"));
}

void ApplicationController::importFile(const QUrl &fileUrl)
{
    if (!fileUrl.isLocalFile()) {
        return;
    }

    const QFileInfo fileInfo(fileUrl.toLocalFile());
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return;
    }

    setDocumentName(fileInfo.fileName());
    emit fileImported(fileUrl);
}

void ApplicationController::setDocumentName(const QString &name)
{
    if (m_documentName == name) {
        return;
    }

    m_documentName = name;
    emit documentNameChanged();
}
