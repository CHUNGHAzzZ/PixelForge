#include "PixelForgeApplication.h"

#include "Utils/Logger.h"

#include <QFileInfo>

namespace PixelForge {

PixelForgeApplication::PixelForgeApplication(QObject *parent)
    : QObject(parent)
    , m_documentName(QStringLiteral("Untitled Canvas"))
{
}

QString PixelForgeApplication::documentName() const
{
    return m_documentName;
}

void PixelForgeApplication::newDocument()
{
    Logger::info("Creating new document");
    setDocumentName(QStringLiteral("Untitled Canvas"));
}

void PixelForgeApplication::importFile(const QUrl &fileUrl)
{
    if (!fileUrl.isLocalFile()) {
        Logger::warning("Ignored non-local import URL");
        return;
    }

    const QFileInfo fileInfo(fileUrl.toLocalFile());
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        Logger::warning(formatLogMessage("Import file is not available: %s", fileUrl.toLocalFile().toUtf8().constData()));
        return;
    }

    Logger::info(formatLogMessage("Importing file: %s", fileInfo.absoluteFilePath().toUtf8().constData()));
    setDocumentName(fileInfo.fileName());
    emit fileImported(fileUrl);
}

void PixelForgeApplication::setDocumentName(const QString &name)
{
    if (m_documentName == name) {
        return;
    }

    m_documentName = name;
    Logger::debug(formatLogMessage("Document name changed: %s", name.toUtf8().constData()));
    emit documentNameChanged();
}

}
