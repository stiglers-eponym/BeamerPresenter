#include <src/pdfmaster.h>

PdfMaster::PdfMaster(const QString &filename)
{
    // Load the document
    {
        // Check if the file exists.
        QFileInfo fileinfo(filename);
        if (!fileinfo.exists() || !fileinfo.isFile()) {
            qFatal("Error: given filename is not a file.");
        }
        // Load the document.
        document = Poppler::Document::load(filename);
        // Save the modification time.
        lastModified = fileinfo.lastModified();
        // Save the file path.
        pdfpath = filename;
    }
}

PdfMaster::~PdfMaster()
{
    qDeleteAll(paths);
    paths.clear();
    delete document;
}
