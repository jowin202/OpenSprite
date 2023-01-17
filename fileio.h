#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMessageBox>

class FileIO : public QObject
{
    Q_OBJECT
public:
    explicit FileIO(QObject *parent = nullptr);

    QJsonObject read_spd(QString path);
    void write_spd(QString path, QJsonObject file_obj);

signals:

};

#endif // FILEIO_H
