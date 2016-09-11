#include "colorpickerplugin.h"
#include "colorpickerplugin_p.h"

// Qt includes
#include <QMenu>

// QtCreator includes
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/editormanager/ieditor.h>

#include <cppeditor/cppeditorconstants.h>
#include <extensionsystem/pluginmanager.h>
#include <glsleditor/glsleditorconstants.h>
#include <qmljseditor/qmljseditorconstants.h>
#include <texteditor/texteditor.h>
#include <utils/theme/theme.h>

// Plugin includes
#include "colormodifier.h"
#include "colorpickeroptionspage.h"
#include "colorpickerconstants.h"
#include "colorwatcher.h"

#include "widgets/coloreditor.h"
#include "widgets/coloreditordialog.h"

using namespace Core;
using namespace TextEditor;

namespace ColorPicker {
namespace Internal {


////////////////////////// ColorPickerPluginImpl //////////////////////////

ColorPickerPluginImpl::ColorPickerPluginImpl(ColorPickerPlugin *qq) :
    q(qq),
    watchers(),
    colorModifier(new ColorModifier(qq)),
    colorEditorDialog(nullptr),
    generalSettings()
{}

ColorPickerPluginImpl::~ColorPickerPluginImpl()
{}

ColorCategory ColorPickerPluginImpl::colorCategoryForEditor(IEditor *editor) const
{
    ColorCategory ret = ColorCategory::AnyCategory;

    Id lastId = *(--editor->context().end());

    if (lastId == QmlJSEditor::Constants::C_QMLJSEDITOR_ID)
        ret = ColorCategory::QmlCategory;
    else if (lastId == GlslEditor::Constants::C_GLSLEDITOR_ID)
        ret = ColorCategory::GlslCategory;

    return ret;
}

QPoint ColorPickerPluginImpl::clampColorEditorPosition(const QPoint &cursorPos,
                                                       const QRect &rect) const
{
    int colorEditorWidth = colorEditorDialog->width();
    int colorEditorHalfWidth = (colorEditorWidth / 2);

    int posX = cursorPos.x() - colorEditorHalfWidth;
    int widgetRight = rect.right();

    if (posX < 0) {
        posX = 0;
    }
    else if ( (cursorPos.x() + colorEditorHalfWidth) > (widgetRight) ) {
        posX = widgetRight - colorEditorWidth;
    }

    QPoint ret(posX, cursorPos.y());

    return ret;
}

void ColorPickerPluginImpl::setInsertOnChange(bool enable)
{
    ColorEditor *colorEditor = colorEditorDialog->colorWidget();

    if (enable) {
        QObject::connect(colorEditor, &ColorEditor::colorChanged,
                         q, &ColorPickerPlugin::onColorChanged);

        QObject::connect(colorEditor, &ColorEditor::outputFormatChanged,
                         q, &ColorPickerPlugin::onOutputFormatChanged);
    }
    else {
        QObject::disconnect(colorEditor, &ColorEditor::colorChanged,
                            q, &ColorPickerPlugin::onColorChanged);

        QObject::disconnect(colorEditor, &ColorEditor::outputFormatChanged,
                            q, &ColorPickerPlugin::onOutputFormatChanged);
    }
}

void ColorPickerPluginImpl::editorSensitiveSettingChanged(bool isSensitive)
{
    for (auto it = watchers.begin(); it != watchers.end(); ++it) {
        Core::IEditor *editor = it.key();

        ColorCategory newCat = (isSensitive) ? colorCategoryForEditor(editor)
                                             : ColorCategory::AnyCategory;

        ColorWatcher *watcher = it.value();
        watcher->setColorCategory(newCat);

        // Update the color editor
        auto editorWidget = qobject_cast<TextEditorWidget *>(editor->widget());

        Q_ASSERT(editorWidget);

        if (colorEditorDialog) {
            colorEditorDialog->colorWidget()->setColorCategory(newCat);
        }
    }
}

void ColorPickerPluginImpl::insertOnChangeSettingChanged(bool insertOnChange)
{
    setInsertOnChange(insertOnChange);
}


////////////////////////// ColorPickerPlugin //////////////////////////

ColorPickerPlugin::ColorPickerPlugin() :
    d(new ColorPickerPluginImpl(this))
{
}

ColorPickerPlugin::~ColorPickerPlugin()
{}

bool ColorPickerPlugin::initialize(const QStringList &arguments,
                                   QString *errorMessage)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorMessage);

    auto optionsPage = new ColorPickerOptionsPage;
    d->generalSettings = optionsPage->generalSettings();

    connect(optionsPage, &ColorPickerOptionsPage::generalSettingsChanged,
            this, &ColorPickerPlugin::onGeneralSettingsChanged);

    // Register the plugin actions
    ActionContainer *toolsContainer = ActionManager::actionContainer(Core::Constants::M_TOOLS);

    ActionContainer *myContainer = ActionManager::createMenu("ColorPicker");
    QMenu *myMenu = myContainer->menu();
    myMenu->setTitle(tr("&ColorPicker"));
    myMenu->setEnabled(true);

    auto triggerColorEditAction = new QAction(tr(Constants::ACTION_NAME_TRIGGER_COLOR_EDIT), this);
    Command *command = ActionManager::registerAction(triggerColorEditAction,
                                                     Constants::TRIGGER_COLOR_EDIT);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+C")));

    myContainer->addAction(command);

    connect(triggerColorEditAction, &QAction::triggered,
            this, &ColorPickerPlugin::onColorEditTriggered);

    toolsContainer->addMenu(myContainer);

    // Register objects
    addAutoReleasedObject(optionsPage);

    return true;
}

void ColorPickerPlugin::extensionsInitialized()
{}

void ColorPickerPlugin::onColorEditTriggered()
{
    IEditor *currentEditor = EditorManager::instance()->currentEditor();
    if (!currentEditor)
        return;

    auto editorWidget = qobject_cast<TextEditorWidget *>(currentEditor->widget());

    if (editorWidget) {
        ColorCategory cat = (d->generalSettings.m_editorSensitive)
                ? d->colorCategoryForEditor(currentEditor)
                : ColorCategory::AnyCategory;

        // Create a watcher or apply an existing one to the current texteditor
        ColorWatcher *watcher = nullptr;

        if (!d->watchers.contains(currentEditor)) {
            watcher = new ColorWatcher(editorWidget);
            watcher->setColorCategory(cat);

            d->watchers.insert(currentEditor, watcher);
        }
        else {
            watcher = d->watchers.value(currentEditor);
        }

        Q_ASSERT(watcher);

        // If necessary, instanciate the color editor dialog
        if (!d->colorEditorDialog) {
            d->colorEditorDialog = new ColorEditorDialog(Core::ICore::mainWindow());

            connect(d->colorEditorDialog, &QDialog::finished,
                    this, &ColorPickerPlugin::destroyColorEditorDialog);

            connect(d->colorEditorDialog->colorWidget(), &ColorEditor::colorSelected,
                    this, &ColorPickerPlugin::onColorSelected);

            connect(EditorManager::instance(), &EditorManager::editorAboutToClose,
                    this, &ColorPickerPlugin::destroyColorEditorDialog);

            d->setInsertOnChange(d->generalSettings.m_insertOnChange);
        }

        // Process the color under cursor
        ColorExpr toEdit = watcher->process();

        // Move and show the dialog
        QWidget *editorViewport = editorWidget->viewport();

        QPoint newPos = d->clampColorEditorPosition(toEdit.pos,
                                                    editorViewport->rect());

        d->colorEditorDialog->move(editorViewport->mapToGlobal(newPos));
        d->colorEditorDialog->show();

        // Update the color dialog to reflect the processed color
        ColorEditor *colorEditor = d->colorEditorDialog->colorWidget();
        colorEditor->setColorCategory(cat);

        QColor newColor;

        if (toEdit.value.isValid()) {
            newColor = toEdit.value;

            const QSignalBlocker blocker(colorEditor);
            colorEditor->setOutputFormat(toEdit.format);
        } else {
            newColor = colorEditor->color();
        }

        colorEditor->setColor(newColor);
    }
}

void ColorPickerPlugin::onGeneralSettingsChanged(const GeneralSettings &gs)
{
    // Setting : editor sensitive
    bool editorSensitiveOldVal = d->generalSettings.m_editorSensitive;
    bool editorSensitiveNewVal = gs.m_editorSensitive;

    if (editorSensitiveNewVal != editorSensitiveOldVal) {
        d->editorSensitiveSettingChanged(editorSensitiveNewVal);
    }

    // Setting : insert on change
    bool insertOnChangeOldVal = d->generalSettings.m_insertOnChange;
    bool insertOnChangeNewVal = gs.m_insertOnChange;

    if (insertOnChangeNewVal != insertOnChangeOldVal) {
        d->insertOnChangeSettingChanged(insertOnChangeNewVal);
    }

    d->generalSettings = gs;
}

void ColorPickerPlugin::onColorSelected(const QColor &color,
                                        ColorFormat format)
{
    d->colorModifier->insertColor(color, format);
}

void ColorPickerPlugin::onColorChanged(const QColor &color)
{
    ColorEditor *colorEditor = d->colorEditorDialog->colorWidget();

    d->colorModifier->insertColor(color, colorEditor->outputFormat());
}

void ColorPickerPlugin::onOutputFormatChanged(ColorFormat format)
{
    ColorEditor *colorEditor = d->colorEditorDialog->colorWidget();

    d->colorModifier->insertColor(colorEditor->color(), format);
}

void ColorPickerPlugin::destroyColorEditorDialog()
{
    if (d->colorEditorDialog) {
        d->colorEditorDialog->deleteLater();

        d->colorEditorDialog = nullptr;
    }
}

} // namespace Internal
} // namespace ColorPicker
