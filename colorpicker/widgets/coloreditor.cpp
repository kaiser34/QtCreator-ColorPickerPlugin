#include "coloreditor.h"

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QToolButton>
#include <QStyleOption>

// QtCreator includes
#include <coreplugin/coreicons.h>

#include <utils/theme/theme.h>

// Plugin includes
#include "colorframe.h"
#include "colorpicker.h"
#include "hueslider.h"
#include "opacityslider.h"
#include "saturationslider.h"
#include "valueslider.h"

namespace ColorPicker {
namespace Internal {


////////////////////////// ColorEditorImpl //////////////////////////

class ColorEditorImpl
{
public:
    enum UpdateReason
    {
        UpdateFromColorPicker = 1 << 0,
        UpdateFromHueSlider = 1 << 1,
        UpdateFromSaturationSlider = 1 << 2,
        UpdateFromValueSlider = 1 << 3,
        UpdateFromOpacitySlider = 1 << 4,
        UpdateProgrammatically = 1 << 5,

        UpdateAll = UpdateFromColorPicker | UpdateFromHueSlider | UpdateFromSaturationSlider
        | UpdateFromValueSlider | UpdateFromOpacitySlider | UpdateProgrammatically
    };
    Q_DECLARE_FLAGS(UpdateReasons, UpdateReason)

    ColorEditorImpl(ColorEditor *qq);

    /* functions */
    void updateColorWidgets(const QColor &cl, UpdateReasons whichUpdate);
    void updateFormatsLayout();

    void replaceAvailableFormats(const ColorFormatSet &formats);

    void setCurrentFormat(ColorFormat f);
    void setCurrentFormat_atomic(ColorFormat f);
    void setCurrentColor(const QColor &cl);

    QAbstractButton *colorFormatToButton(ColorFormat format) const;

    void onPickerColorChanged(const QColor &color);
    void onHueChanged(int hue);
    void onSaturationChanged(int saturation);
    void onValueChanged(int value);
    void onOpacityChanged(int opacity);

    /* variables */
    ColorEditor *q;

    ColorCategory category;
    ColorFormatSet availableFormats;
    QMap<QAbstractButton *, ColorFormat> buttonToColorFormat;

    ColorFormat outputFormat;
    QColor color;

    ColorPickerWidget *colorPicker;
    HueSlider *hueSlider;
    SaturationSlider *saturationSlider;
    ValueSlider *valueSlider;
    OpacitySlider *opacitySlider;

    ColorFrame *colorFrame;
    QHBoxLayout *formatsLayout;
    QButtonGroup *btnGroup;
    QToolButton *rgbBtn;
    QToolButton *hsvBtn;
    QToolButton *hslBtn;
    QToolButton *qmlRgbaBtn;
    QToolButton *qmlHslaBtn;
    QToolButton *vecBtn;
    QToolButton *hexBtn;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ColorEditorImpl::UpdateReasons)

ColorEditorImpl::ColorEditorImpl(ColorEditor *qq) :
    q(qq),
    category(ColorCategory::CssCategory), // trick
    availableFormats(),
    buttonToColorFormat(),
    outputFormat(),
    color(QColor::Hsv),
    colorPicker(new ColorPickerWidget(qq)),
    hueSlider(new HueSlider(qq)),
    saturationSlider(new SaturationSlider(qq)),
    valueSlider(new ValueSlider(qq)),
    opacitySlider(new OpacitySlider(qq)),
    colorFrame(new ColorFrame()),
    formatsLayout(new QHBoxLayout),
    btnGroup(new QButtonGroup(qq)),
    rgbBtn(new QToolButton(qq)),
    hsvBtn(new QToolButton(qq)),
    hslBtn(new QToolButton(qq)),
    qmlRgbaBtn(new QToolButton(qq)),
    qmlHslaBtn(new QToolButton(qq)),
    vecBtn(new QToolButton(qq)),
    hexBtn(new QToolButton(qq))
{}

void ColorEditorImpl::updateColorWidgets(const QColor &cl, UpdateReasons whichUpdate)
{
    if (whichUpdate & ColorEditorImpl::UpdateFromColorPicker) {
        const QSignalBlocker blocker(colorPicker);

        saturationSlider->setValueAtomic(cl.hsvSaturation());
        valueSlider->setValueAtomic(cl.value());
        opacitySlider->setHsv(cl.hsvHue(), cl.hsvSaturation(), cl.value());

        colorFrame->setColor(cl);
    }

    if (whichUpdate & ColorEditorImpl::UpdateFromHueSlider) {
        const QSignalBlocker blocker(colorPicker);

        int newHue = cl.hsvHue();

        colorPicker->setColor(cl);

        saturationSlider->setHue(newHue);
        valueSlider->setHue(newHue);
        opacitySlider->setHsv(newHue, cl.hsvSaturation(), cl.value());

        colorFrame->setColor(cl);
    }

    if (whichUpdate & ColorEditorImpl::UpdateFromSaturationSlider
            || whichUpdate & ColorEditorImpl::UpdateFromValueSlider) {
        const QSignalBlocker blocker(colorPicker);

        colorPicker->setColor(cl);
        colorFrame->setColor(cl);
    }

    if (whichUpdate & ColorEditorImpl::UpdateFromOpacitySlider) {
        const QSignalBlocker blocker(colorPicker);
        colorFrame->setColor(cl);
    }

    if (whichUpdate & ColorEditorImpl::UpdateProgrammatically) {
        hueSlider->setValueAtomic(cl.hsvHue());
        saturationSlider->setValueAtomic(cl.hsvSaturation());
        valueSlider->setValueAtomic(cl.value());
        opacitySlider->setValueAtomic(cl.alpha());

        colorFrame->setColor(cl);
    }
}

void ColorEditorImpl::updateFormatsLayout()
{
    // Clear the layout
    QLayoutItem *child;
    while ( (child = formatsLayout->takeAt(0)) != 0 ) {
        delete child;
    }

    // Populate with right buttons
    if (availableFormats.contains(ColorFormat::QCssRgbUCharFormat)) {
        rgbBtn->setVisible(true);
        formatsLayout->addWidget(rgbBtn);
    }
    else {
        rgbBtn->setVisible(false);
    }

    if (availableFormats.contains(ColorFormat::QssHsvFormat)) {
        hsvBtn->setVisible(true);
        formatsLayout->addWidget(hsvBtn);
    }
    else {
        hsvBtn->setVisible(false);
    }

    if (availableFormats.contains(ColorFormat::CssHslFormat)) {
        hslBtn->setVisible(true);
        formatsLayout->addWidget(hslBtn);
    }
    else {
        hslBtn->setVisible(false);
    }

    if (availableFormats.contains(ColorFormat::QmlRgbaFormat)) {
        qmlRgbaBtn->setVisible(true);
        formatsLayout->addWidget(qmlRgbaBtn);
    }
    else {
        qmlRgbaBtn->setVisible(false);
    }

    if (availableFormats.contains(ColorFormat::QmlHslaFormat)) {
        qmlHslaBtn->setVisible(true);
        formatsLayout->addWidget(qmlHslaBtn);
    }
    else {
        qmlHslaBtn->setVisible(false);
    }

    if (availableFormats.contains(ColorFormat::GlslFormat)) {
        vecBtn->setVisible(true);
        formatsLayout->addWidget(vecBtn);
    }
    else {
        vecBtn->setVisible(false);
    }

    if (availableFormats.contains(ColorFormat::HexFormat)) {
        hexBtn->setVisible(true);
        formatsLayout->addWidget(hexBtn);
    }
    else {
        hexBtn->setVisible(false);
    }

    formatsLayout->addStretch();
}

void ColorEditorImpl::replaceAvailableFormats(const ColorFormatSet &formats)
{
    availableFormats = formats;

    updateFormatsLayout();
}

void ColorEditorImpl::setCurrentFormat(ColorFormat f)
{
    outputFormat = f;

    emit q->outputFormatChanged(f);
}

void ColorEditorImpl::setCurrentFormat_atomic(ColorFormat f)
{
    outputFormat = f;
}

void ColorEditorImpl::setCurrentColor(const QColor &cl)
{
    color = cl;

    emit q->colorChanged(cl);
}

QAbstractButton *ColorEditorImpl::colorFormatToButton(ColorFormat format) const
{
    QAbstractButton *ret = nullptr;

    switch (format) {
    case QCssRgbUCharFormat:
    case QCssRgbPercentFormat:
        ret = rgbBtn;
        break;
    case QssHsvFormat:
        ret = hsvBtn;
        break;
    case CssHslFormat:
        ret = hslBtn;
        break;
    case QmlRgbaFormat:
        ret = qmlRgbaBtn;
        break;
    case QmlHslaFormat:
        ret = qmlHslaBtn;
        break;
    case GlslFormat:
        ret = vecBtn;
        break;
    case HexFormat:
        ret = hexBtn;
        break;
    default:
        break;
    }

    Q_ASSERT(ret);
    return ret;
}

void ColorEditorImpl::onPickerColorChanged(const QColor &color)
{
    QColor newColor(color);
    newColor.setAlpha(opacitySlider->value());

    updateColorWidgets(newColor, ColorEditorImpl::UpdateFromColorPicker);
    setCurrentColor(newColor);
}

void ColorEditorImpl::onHueChanged(int hue)
{
    QColor newColor = QColor::fromHsv(hue,
                                      color.hsvSaturation(),
                                      color.value(),
                                      opacitySlider->value());

    updateColorWidgets(newColor, ColorEditorImpl::UpdateFromHueSlider);
    setCurrentColor(newColor);
}

void ColorEditorImpl::onSaturationChanged(int saturation)
{
    QColor newColor = QColor::fromHsv(color.hsvHue(),
                                      saturation,
                                      color.value(),
                                      opacitySlider->value());

    updateColorWidgets(newColor, ColorEditorImpl::UpdateFromSaturationSlider);
    setCurrentColor(newColor);
}

void ColorEditorImpl::onValueChanged(int value)
{
    QColor newColor = QColor::fromHsv(color.hsvHue(),
                                      color.hsvSaturation(),
                                      value,
                                      opacitySlider->value());

    updateColorWidgets(newColor, ColorEditorImpl::UpdateFromValueSlider);
    setCurrentColor(newColor);
}

void ColorEditorImpl::onOpacityChanged(int opacity)
{
    QColor newColor = QColor::fromHsv(hueSlider->value(),
                                      color.hsvSaturation(),
                                      color.value(),
                                      opacity);

    updateColorWidgets(newColor, ColorEditorImpl::UpdateFromOpacitySlider);
    setCurrentColor(newColor);
}


////////////////////////// ColorEditor //////////////////////////

ColorEditor::ColorEditor(QWidget *parent) :
    QFrame(parent),
    d(new ColorEditorImpl(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setCursor(QCursor(Qt::ArrowCursor));

    // Build UI
    // Color format selection
    d->rgbBtn->setText(QLatin1String("rgb"));
    d->hsvBtn->setText(QLatin1String("hsv"));
    d->hslBtn->setText(QLatin1String("hsl"));
    d->qmlRgbaBtn->setText(QLatin1String("Qt.rgba"));
    d->qmlHslaBtn->setText(QLatin1String("Qt.hsla"));
    d->vecBtn->setText(QLatin1String("vec"));
    d->hexBtn->setText(QLatin1String("hex"));

    d->rgbBtn->setCheckable(true);
    d->hsvBtn->setCheckable(true);
    d->hslBtn->setCheckable(true);
    d->qmlRgbaBtn->setCheckable(true);
    d->qmlHslaBtn->setCheckable(true);
    d->vecBtn->setCheckable(true);
    d->hexBtn->setCheckable(true);

    // Build layouts
    d->formatsLayout->setSpacing(0);

    d->colorFrame->setMinimumSize(QSize(30, 35));

    auto rightLayout = new QVBoxLayout;
    rightLayout->addWidget(d->colorFrame);
    rightLayout->addStretch();

    auto leftPanelLayout = new QVBoxLayout;
    leftPanelLayout->addStretch();

    auto colorWidgetsLayout = new QHBoxLayout;
    colorWidgetsLayout->addWidget(d->colorPicker);
    colorWidgetsLayout->addWidget(d->hueSlider);
    colorWidgetsLayout->addWidget(d->saturationSlider);
    colorWidgetsLayout->addWidget(d->valueSlider);
    colorWidgetsLayout->addWidget(d->opacitySlider);
    colorWidgetsLayout->addLayout(rightLayout);

    auto centerLayout = new QVBoxLayout;
    centerLayout->addLayout(colorWidgetsLayout);
    centerLayout->addLayout(d->formatsLayout);

    auto mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(leftPanelLayout);
    mainLayout->addSpacing(0);
    mainLayout->addLayout(centerLayout);

    // Color format selection logic
    d->btnGroup->addButton(d->rgbBtn);
    d->btnGroup->addButton(d->hsvBtn);
    d->btnGroup->addButton(d->hslBtn);
    d->btnGroup->addButton(d->qmlRgbaBtn);
    d->btnGroup->addButton(d->qmlHslaBtn);
    d->btnGroup->addButton(d->vecBtn);
    d->btnGroup->addButton(d->hexBtn);

    d->buttonToColorFormat.insert(d->rgbBtn, ColorFormat::QCssRgbUCharFormat);
    d->buttonToColorFormat.insert(d->hsvBtn, ColorFormat::QssHsvFormat);
    d->buttonToColorFormat.insert(d->hslBtn, ColorFormat::CssHslFormat);
    d->buttonToColorFormat.insert(d->qmlRgbaBtn, ColorFormat::QmlRgbaFormat);
    d->buttonToColorFormat.insert(d->qmlHslaBtn, ColorFormat::QmlHslaFormat);
    d->buttonToColorFormat.insert(d->vecBtn, ColorFormat::GlslFormat);
    d->buttonToColorFormat.insert(d->hexBtn, ColorFormat::HexFormat);

    connect(d->btnGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked),
            [=] (QAbstractButton *btn) {
        ColorFormat format = d->buttonToColorFormat.value(btn);

        d->setCurrentFormat(format);
    });

    // Color changes logic
    connect(d->colorPicker, &ColorPickerWidget::colorChanged,
            [=](const QColor &color) { d->onPickerColorChanged(color); });

    connect(d->hueSlider, &HueSlider::valueChanged,
            [=](int hue) { d->onHueChanged(hue); });

    connect(d->saturationSlider, &SaturationSlider::valueChanged,
            [=](int sat) { d->onSaturationChanged(sat); });

    connect(d->valueSlider, &ValueSlider::valueChanged,
            [=](int val) { d->onValueChanged(val); });

    connect(d->opacitySlider, &OpacitySlider::valueChanged,
            [=](int opacity) { d->onOpacityChanged(opacity); });

    setColorCategory(ColorCategory::AnyCategory);
    setOutputFormat(ColorFormat::QCssRgbUCharFormat);
    setColor(Qt::red);
}

ColorEditor::~ColorEditor()
{}

ColorCategory ColorEditor::colorCategory() const
{
    return d->category;
}

void ColorEditor::setColorCategory(ColorCategory category)
{
    if (d->category != category) {
        d->category = category;

        ColorFormatSet formats = formatsFromCategory(category);
        d->replaceAvailableFormats(formats);

        // Set the first format in the set and check the right button
        ColorFormat firstFormat = (*formats.begin());

        const QSignalBlocker blocker(d->btnGroup);
        QAbstractButton *btn = d->colorFormatToButton(firstFormat);
        btn->click();

        d->setCurrentFormat_atomic(firstFormat);
    }
}

ColorFormatSet ColorEditor::availableFormats() const
{
    return d->availableFormats;
}

ColorFormat ColorEditor::outputFormat() const
{
    return d->outputFormat;
}

void ColorEditor::setOutputFormat(ColorFormat format)
{
    if (d->outputFormat != format) {
        const QSignalBlocker blocker(d->btnGroup);

        QAbstractButton *btn = d->colorFormatToButton(format);
        btn->click();

        d->setCurrentFormat(format);
    }
}

QColor ColorEditor::color() const
{
    return d->color;
}

void ColorEditor::setColor(const QColor &color)
{
    if (d->color != color) {
        d->updateColorWidgets(color, ColorEditorImpl::UpdateAll);
        d->setCurrentColor(color);
    }
}

int ColorEditor::hue() const
{
    return d->hueSlider->value();
}

void ColorEditor::setHue(int hue)
{
    Q_ASSERT(hue >= 0 && hue <= 359);

    if (d->hueSlider->value() != hue) {
        d->hueSlider->setValue(hue);

        emit hueChanged(hue);
    }
}

int ColorEditor::opacity() const
{
    return d->opacitySlider->value();
}

void ColorEditor::setOpacity(int opacity)
{
    Q_ASSERT(opacity >= 0 && opacity <= 255);

    if (d->opacitySlider->value() != opacity) {
        d->opacitySlider->setValue(opacity);

        emit opacityChanged(opacity);
    }
}

void ColorEditor::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();

    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        emit colorSelected(d->color, d->outputFormat);
    }
}

} // namespace Internal
} // namespace ColorPicker
