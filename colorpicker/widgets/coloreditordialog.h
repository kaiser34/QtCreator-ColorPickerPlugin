#ifndef VALUESLIDER_H
#define VALUESLIDER_H

#include <QDialog>

namespace ColorPicker {
namespace Internal {

class ColorEditor;

class ColorEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ColorEditorDialog(QWidget *parent = nullptr);

    ColorEditor *colorWidget() const;

private:
    ColorEditor *m_colorWidget;
};

} // namespace Internal
} // namespace ColorPicker

#endif // VALUESLIDER_H
