#ifndef FILELOADER_H
#define FILELOADER_H

#include <QObject>

class pqPipelineSource;

class FileLoader : public QObject
{
public:
    static pqPipelineSource* loadDataFile(const QString & filePath);
};

#endif // FILELOADER_H
