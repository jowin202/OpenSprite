#ifndef FILEIO_H
#define FILEIO_H

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QObject>

class FileIO : public QObject
{
    Q_OBJECT
public:
    explicit FileIO(QObject *parent = nullptr);

    QJsonObject read_spd(QString path);
    void write_spd(QString path, QJsonObject file_obj);

    void write_prg(QString path, QJsonObject file_obj, int address, int attribute);
    QJsonObject read_prg_bin(QString path, int *address);


    QJsonObject read_spm(QString path);

signals:
};

#endif // FILEIO_H
