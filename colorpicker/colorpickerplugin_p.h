#ifndef COLORPICKERPLUGIN_P_H
#define COLORPICKERPLUGIN_P_H

#include "generalsettings.h"

namespace ColorPicker {
namespace Internal {

class ColorEditorDialog;
class ColorModifier;
class ColorWatcher;

////////////////////////// ColorPickerPluginImpl //////////////////////////

class ColorPickerPluginImpl
{
public:
    ColorPickerPluginImpl(ColorPickerPlugin *qq);
    ~ColorPickerPluginImpl();

    /* functions */
    ColorCategory colorCategoryForEditor(Core::IEditor *editor) const;
    QPoint clampColorEditorPosition(const QPoint &cursorPos,
                                    const QRect &rect) const;


    void setInsertOnChange(bool enable);

    void editorSensitiveSettingChanged(bool isSensitive);
    void insertOnChangeSettingChanged(bool insertOnChange);

    /* variables */
    ColorPickerPlugin *q;

    QMap<Core::IEditor *, ColorWatcher *> watchers;
    ColorModifier *colorModifier;
    ColorEditorDialog *colorEditorDialog;

    GeneralSettings generalSettings;
};

} // namespace Internal
} // namespace ColorPicker

#endif // COLORPICKERPLUGIN_P_H
