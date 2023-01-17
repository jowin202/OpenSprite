#ifndef FILEIMPORT_H
#define FILEIMPORT_H

#include <QObject>
#include <QFile>
#include <QMessageBox>
#include <QString>

class Sprite;
struct options;

class FileImport : public QObject
{
    Q_OBJECT
public:
    explicit FileImport(QString path, options *opt);



signals:

};

#endif // FILEIMPORT_H
