/*#include "fmutils.h"


fmutils::fmutils()
{

}

void fmutils::copytoclipboard(QStringList files)
{

}

void fmutils::pastefromclipboard(QString destdir)
{
    QClipboard *clip = QApplication::clipboard();
    if (clip->mimeData()->hasUrls())
    {
        QList<QUrl> urllist = clip->mimeData()->urls();
        foreach (QUrl url, urllist)
        {
            QString filepath;
            filepath = url.toString();
            filepath.remove("file://");

            QStringList pathlist = filepath.split("/");

            if (!destdir.endsWith("/")) destdir.append("/");

            QProcess cp;
            cp.start("cp -r " + filepath + " " + destdir + pathlist.last());
            cp.waitForFinished();
        }
    }
}
*/
