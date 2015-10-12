#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QFrame>

#include "../colorpicker_global.h"

class QAbstractButton;

namespace ColorPicker {
namespace Internal {

class ColorDialogImpl;
class ColorPickerWidget;
class HueSlider;
class OpacitySlider;

class ColorDialog : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int hue READ hue WRITE setHue NOTIFY hueChanged)
    Q_PROPERTY(int opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)

public:
    explicit ColorDialog(QWidget *parent = 0);
    ~ColorDialog();

    ColorFormat outputColorFormat() const;

    QColor color() const;
    int hue() const;
    int opacity() const;

public slots:
    void setOutputColorFormat(ColorFormat format);

    void setColor(const QColor &color);
    void setHue(int hue);
    void setOpacity(int opacity);

signals:
    void colorChanged(const QColor &, ColorFormat);
    void hueChanged(int);
    void opacityChanged(int);

private slots:
    void onFormatButtonChecked(QAbstractButton *checkedBtn);

private:
    QScopedPointer<ColorDialogImpl> d;
};

} // namespace Internal
} // namespace ColorPicker

#endif // COLORDIALOG_H
