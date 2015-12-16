#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:34:33
#
#-------------------------------------------------

QT += core gui opengl widgets
QMAKE_CXXFLAGS += /WX
RESOURCES += Icons.qrc

CONFIG += console
TEMPLATE = app
DESTDIR = $$PWD/../../bin
UI_DIR = $$PWD/../../build/Editor
DEFINES += GLEW_STATIC

CONFIG(debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$PWD/../../build/Editor/debug
    MOC_DIR = $$PWD/../../build/Editor/debug
    RCC_DIR = $$PWD/../../build/Editor/debug
    TARGET = PrimeWorldEditor-debug

    # Debug Libs
    LIBS += -L$$PWD/../../build/Common/ -lCommond \
            -L$$PWD/../../build/Core/ -lCored \
            -L$$PWD/../../externals/assimp/lib/ -lassimp-vc120-mtd \
            -L$$PWD/../../externals/FileIO/lib/ -lFileIOd \
            -L$$PWD/../../externals/tinyxml2/lib/ -ltinyxml2d

    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/Common/Commond.lib \
                          $$PWD/../../build/Core/Cored.lib
    }
}

CONFIG(release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$PWD/../../build/Editor/release
    MOC_DIR = $$PWD/../../build/Editor/release
    RCC_DIR = $$PWD/../../build/Editor/release
    TARGET = PrimeWorldEditor

    # Release Libs
    LIBS += -L$$PWD/../../build/Common/ -lCommon \
            -L$$PWD/../../build/Core/ -lCore \
            -L$$PWD/../../externals/assimp/lib/ -lassimp-vc120-mt \
            -L$$PWD/../../externals/FileIO/lib/ -lFileIO \
            -L$$PWD/../../externals/tinyxml2/lib/ -ltinyxml2

    win32 {
        PRE_TARGETDEPS += -L$$PWD/../../build/Common/ -lCommon \
                          -L$$PWD/../../build/Core/ -lCore
    }
}

# Debug/Release Libs
LIBS += -L$$PWD/../../externals/glew-1.9.0/lib/ -lglew32s \
        -L$$PWD/../../externals/lzo-2.08/lib -llzo-2.08 \
        -L$$PWD/../../externals/zlib/lib -lzdll

# Include Paths
INCLUDEPATH += $$PWD/../ \
               $$PWD/../../externals/assimp/include \
               $$PWD/../../externals/boost_1_56_0 \
               $$PWD/../../externals/FileIO/include \
               $$PWD/../../externals/glew-1.9.0/include \
               $$PWD/../../externals/glm/glm \
               $$PWD/../../externals/lzo-2.08/include \
               $$PWD/../../externals/tinyxml2/include \
               $$PWD/../../externals/zlib/include

# Source Files
HEADERS += \
    ModelEditor/CModelEditorViewport.h \
    ModelEditor/CModelEditorWindow.h \
    Undo/CClearSelectionCommand.h \
    Undo/CDeselectNodeCommand.h \
    Undo/CRotateNodeCommand.h \
    Undo/CScaleNodeCommand.h \
    Undo/CSelectNodeCommand.h \
    Undo/CTranslateNodeCommand.h \
    Undo/EUndoCommand.h \
    Undo/UndoCommands.h \
    Widgets/IPreviewPanel.h \
    Widgets/WAnimParamsEditor.h \
    Widgets/WCollapsibleGroupBox.h \
    Widgets/WColorPicker.h \
    Widgets/WDraggableSpinBox.h \
    Widgets/WIntegralSpinBox.h \
    Widgets/WPropertyEditor.h \
    Widgets/WResourceSelector.h \
    Widgets/WRollout.h \
    Widgets/WScanPreviewPanel.h \
    Widgets/WStringPreviewPanel.h \
    Widgets/WTextureGLWidget.h \
    Widgets/WTexturePreviewPanel.h \
    Widgets/WVectorEditor.h \
    WorldEditor/CAboutDialog.h \
    WorldEditor/CLayerEditor.h \
    WorldEditor/CLayerModel.h \
    WorldEditor/CLayersInstanceModel.h \
    WorldEditor/CLinkModel.h \
    WorldEditor/CTypesInstanceModel.h \
    WorldEditor/CWorldEditor.h \
    WorldEditor/WCreateTab.h \
    WorldEditor/WInstancesTab.h \
    WorldEditor/WModifyTab.h \
    CBasicViewport.h \
    CDarkStyle.h \
    CGizmo.h \
    CNodeSelection.h \
    CSceneViewport.h \
    CStartWindow.h \
    INodeEditor.h \
    TestDialog.h \
    UICommon.h

SOURCES += \
    ModelEditor/CModelEditorViewport.cpp \
    ModelEditor/CModelEditorWindow.cpp \
    Undo/CClearSelectionCommand.cpp \
    Undo/CDeselectNodeCommand.cpp \
    Undo/CRotateNodeCommand.cpp \
    Undo/CScaleNodeCommand.cpp \
    Undo/CSelectNodeCommand.cpp \
    Undo/CTranslateNodeCommand.cpp \
    Widgets/IPreviewPanel.cpp \
    Widgets/WAnimParamsEditor.cpp \
    Widgets/WCollapsibleGroupBox.cpp \
    Widgets/WColorPicker.cpp \
    Widgets/WDraggableSpinBox.cpp \
    Widgets/WIntegralSpinBox.cpp \
    Widgets/WPropertyEditor.cpp \
    Widgets/WResourceSelector.cpp \
    Widgets/WRollout.cpp \
    Widgets/WScanPreviewPanel.cpp \
    Widgets/WStringPreviewPanel.cpp \
    Widgets/WTextureGLWidget.cpp \
    Widgets/WTexturePreviewPanel.cpp \
    Widgets/WVectorEditor.cpp \
    WorldEditor/CAboutDialog.cpp \
    WorldEditor/CLayerEditor.cpp \
    WorldEditor/CLayerModel.cpp \
    WorldEditor/CLayersInstanceModel.cpp \
    WorldEditor/CLinkModel.cpp \
    WorldEditor/CTypesInstanceModel.cpp \
    WorldEditor/CWorldEditor.cpp \
    WorldEditor/WCreateTab.cpp \
    WorldEditor/WInstancesTab.cpp \
    WorldEditor/WModifyTab.cpp \
    CBasicViewport.cpp \
    CDarkStyle.cpp \
    CGizmo.cpp \
    CNodeSelection.cpp \
    CSceneViewport.cpp \
    CStartWindow.cpp \
    INodeEditor.cpp \
    main.cpp \
    TestDialog.cpp \
    UICommon.cpp

FORMS += \
    CStartWindow.ui \
    TestDialog.ui \
    ModelEditor/CModelEditorWindow.ui \
    Widgets/WScanPreviewPanel.ui \
    Widgets/WTexturePreviewPanel.ui \
    WorldEditor/CAboutDialog.ui \
    WorldEditor/CLayerEditor.ui \
    WorldEditor/CWorldEditor.ui \
    WorldEditor/WCreateTab.ui \
    WorldEditor/WInstancesTab.ui \
    WorldEditor/WModifyTab.ui