#include "FileLoader.h"

#include <QString>
#include <QStringList>

#include <pqLoadDataReaction.h>
#include <pqPipelineSource.h>

pqPipelineSource* FileLoader::loadDataFile(const QString & filePath)
{
  QStringList file;
  file << filePath;
  QList<QStringList> files;
  files << file;

  pqPipelineSource* source = pqLoadDataReaction::loadData(files);

  return source;
}


