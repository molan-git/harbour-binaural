#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QtQml>
#include <sailfishapp.h>
#include "audioengine.h"

int main(int argc, char *argv[])
{
    qmlRegisterType<AudioEngine>("Binaural", 1, 0, "AudioEngine");

    return SailfishApp::main(argc, argv);
}
