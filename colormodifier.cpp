#include "colormodifier.h"

// Qt includes
#include <QTextBlock>
#include <QTextCursor>

// QtCreator includes
#include <coreplugin/editormanager/editormanager.h>

#include <texteditor/texteditor.h>


using namespace Core;
using namespace TextEditor;

namespace ColorPicker {
namespace Internal {


////////////////////////// ColorModifierImpl //////////////////////////

class ColorModifierImpl
{
public:
    ColorModifierImpl() :
        currentEdited()
    {

    }

    /* functions */
    QString colorTypePrefix(ColorType type) const
    {
        QString ret;

        switch (type) {
        case ColorType::RgbType:
            ret = QLatin1String("rgb(");
            break;
        case ColorType::RgbaType:
            ret = QLatin1String("rgba(");
            break;
        case ColorType::HslType:
            ret = QLatin1String("hsl(");
            break;
        case ColorType::HslaType:
            ret = QLatin1String("hsla(");
            break;
        case ColorType::HsvType:
            ret = QLatin1String("hsv(");
            break;
        case ColorType::HsvaType:
            ret = QLatin1String("hsva(");
            break;
        // No ColorType::HexType because of the QColor::name() function
        case ColorType::Vec3Type:
            ret = QLatin1String("vec3(");
            break;
        case ColorType::Vec4Type:
            ret = QLatin1String("vec4(");
            break;
        case ColorType::QColorInlineCtorRgbType:
            ret = QLatin1String("QColor(");
            break;
        default:
            break;
        }

        Q_ASSERT_X(!ret.isNull(), Q_FUNC_INFO, "The string version of the color cannot be invalid.");

        return ret;
    }

    QString colorComponents(const QColor &color, ColorType type) const
    {
        QString ret;

        if (type == ColorType::RgbType) {
            ret = QString::number(color.red()) + QLatin1String(", ")
                    + QString::number(color.green()) + QLatin1String(", ")
                    + QString::number(color.blue());
        }
        else if (type == ColorType::RgbaType) {
            ret = QString::number(color.red()) + QLatin1String(", ")
                    + QString::number(color.green()) + QLatin1String(", ")
                    + QString::number(color.blue()) + QLatin1String(", ")
                    + QString::number(color.alphaF());
        }
        else if (type == ColorType::HslType) {
            ret = QString::number(color.hue()) + QLatin1String(", ")
                    + QString::number(color.saturation()) + QLatin1String(", ")
                    + QString::number(color.lightness());
        }
        else if (type == ColorType::HslaType) {
            ret = QString::number(color.hue()) + QLatin1String(", ")
                    + QString::number(color.saturation()) + QLatin1String(", ")
                    + QString::number(color.lightness()) + QLatin1String(", ")
                    + QString::number(color.alphaF());
        }
        else if (type == ColorType::HsvType) {
            ret = QString::number(color.hue()) + QLatin1String(", ")
                    + QString::number(color.saturation()) + QLatin1String(", ")
                    + QString::number(color.value());
        }
        else if (type == ColorType::HsvaType) {
            ret = QString::number(color.hue()) + QLatin1String(", ")
                    + QString::number(color.saturation()) + QLatin1String(", ")
                    + QString::number(color.value()) + QLatin1String(", ")
                    + QString::number(color.alphaF());
        }
        else if (type == ColorType::HexType) {
            ret = color.name();
        }
        else if (type == ColorType::Vec3Type) {
            ret = QString::number(color.redF()) + QLatin1String(", ")
                    + QString::number(color.greenF()) + QLatin1String(", ")
                    + QString::number(color.blueF());
        }
        else if (type == ColorType::Vec4Type) {
            ret = QString::number(color.redF()) + QLatin1String(", ")
                    + QString::number(color.greenF()) + QLatin1String(", ")
                    + QString::number(color.blueF()) + QLatin1String(", ")
                    + QString::number(color.alphaF());
        }
        else if (type == ColorType::QColorInlineCtorRgbType) {
            ret = QString::number(color.red()) + QLatin1String(", ")
                    + QString::number(color.green()) + QLatin1String(", ")
                    + QString::number(color.blue());

            int cAlpha = color.alpha();

            if (cAlpha != 255)
                ret += QLatin1String(", ") + QString::number(cAlpha);
        }

        Q_ASSERT_X(!ret.isNull(), Q_FUNC_INFO, "The string version of the color cannot be invalid.");

        return ret;
    }

    /* attributes */
    ColorExpr currentEdited;
};


////////////////////////// ColorModifier //////////////////////////

ColorModifier::ColorModifier(QObject *parent) :
    QObject(parent),
    d(new ColorModifierImpl)
{

}

ColorModifier::~ColorModifier()
{}

void ColorModifier::insertColor(const QColor &newValue, ColorType asType)
{
    IEditor *currentEditor = EditorManager::instance()->currentEditor();
    if (!currentEditor)
        return;

    TextEditorWidget *editorWidget = qobject_cast<TextEditorWidget *>(currentEditor->widget());

    QTextBlock block = editorWidget->document()->findBlockByLineNumber(d->currentEdited.line);
    Q_ASSERT_X(block.isValid(), Q_FUNC_INFO, "The line number is invalid.");

    QTextCursor cursor(block);

    // If the dest type is the same as the source type, we just modify the color components
}

void ColorModifier::setTarget(const ColorExpr &colorExpr)
{
    if (d->currentEdited != colorExpr)
        d->currentEdited = colorExpr;
}

} // namespace Internal
} // namespace ColorPicker