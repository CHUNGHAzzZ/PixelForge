#include "ApplicationController.h"

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

void ApplicationController::setDocumentName(const QString &name)
{
    if (m_documentName == name) {
        return;
    }

    m_documentName = name;
    emit documentNameChanged();
}
