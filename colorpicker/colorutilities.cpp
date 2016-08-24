#include "colorutilities.h"

#include <QDebug> // REMOVEME
#include <QRegularExpression>

namespace {


////////////////////// Parsing Helpers //////////////////////

QString colorDoubleToQString(double n)
{
    QString ret = QString::number(n, 'f', 2);

    QString toReplace = QLatin1String(".00");
    if (ret.contains(toReplace))
        ret.replace(toReplace, QLatin1String(".0"));

    return ret;
}

void parseQCssRgbUChar(const QRegularExpressionMatch &match, QColor &result)
{
    int r = match.captured(1).toInt();
    int g = match.captured(2).toInt();
    int b = match.captured(3).toInt();

    result.setRgb(r, g, b);

    QString possibleAlpha = match.captured(4);
    if (!possibleAlpha.isNull()) {
        qreal a = possibleAlpha.toDouble();

        result.setAlphaF(a);
    }
}

void parseCssRgbPercent(const QRegularExpressionMatch &match, QColor &result)
{
    qreal r = match.captured(1).remove(QChar::fromLatin1('%')).toDouble() / 100;
    qreal g = match.captured(2).remove(QChar::fromLatin1('%')).toDouble() / 100;
    qreal b = match.captured(3).remove(QChar::fromLatin1('%')).toDouble() / 100;

    result.setRgbF(r, g, b);

    QString possibleAlpha = match.captured(4);
    if (!possibleAlpha.isNull()) {
        qreal a = possibleAlpha.toDouble();

        result.setAlphaF(a);
    }
}

void parseQssHsv(const QRegularExpressionMatch &match, QColor &result)
{
    int h = match.captured(1).toInt();
    int s = match.captured(2).toInt();
    int v = match.captured(3).toInt();

    result.setHsv(h, s, v);

    QString possibleAlpha = match.captured(4);

    if (!possibleAlpha.isNull()) {
        qreal a = possibleAlpha.remove(QChar::fromLatin1('%')).toDouble() / 100;

        result.setAlphaF(a);
    }
}

void parseCssHsl(const QRegularExpressionMatch &match, QColor &result)
{
    int h = match.captured(1).toInt();
    int s = match.captured(2).remove(QChar::fromLatin1('%')).toInt() * 255 / 100;
    int l = match.captured(3).remove(QChar::fromLatin1('%')).toInt() * 255 / 100;

    result.setHsl(h, s, l);

    QString possibleAlpha = match.captured(4);
    if (!possibleAlpha.isNull()) {
        qreal a = possibleAlpha.toDouble();

        result.setAlphaF(a);
    }
}

void parseQmlRgba(const QRegularExpressionMatch &match, QColor &result)
{
    qreal r = match.captured(1).toDouble();
    qreal g = match.captured(2).toDouble();
    qreal b = match.captured(3).toDouble();
    qreal a = match.captured(4).toDouble();

    result.setRgbF(r, g, b, a);
}

void parseQmlHsla(const QRegularExpressionMatch &match, QColor &result)
{
    qreal h = match.captured(1).toDouble();
    qreal s = match.captured(2).toDouble();
    qreal l = match.captured(3).toDouble();
    qreal a = match.captured(4).toDouble();

    result.setHslF(h, s, l, a);
}

void parseGlslColor(const QRegularExpressionMatch &match, QColor &result)
{
    qreal r = match.captured(1).toDouble();
    qreal g = match.captured(2).toDouble();
    qreal b = match.captured(3).toDouble();

    result.setRgbF(r, g, b);

    QString possibleAlpha = match.captured(4);
    if (!possibleAlpha.isNull()) {
        qreal a = possibleAlpha.toDouble();

        result.setAlphaF(a);
    }
}

void parseHexColor(const QRegularExpressionMatch &match, QColor &result)
{
    result.setNamedColor(match.captured());
}


////////////////////// ColorToString Helpers //////////////////////

void cssRgbUCharToQString(const QColor &color, QString &prefix,
                          QString &parts)
{
    prefix = QLatin1String("rgb(");
    parts = QString::number(color.red()) + QLatin1String(", ")
            + QString::number(color.green()) + QLatin1String(", ")
            + QString::number(color.blue());

    qreal alpha = color.alphaF();
    if (alpha < 1.0) {
        prefix.insert(3, QLatin1Char('a'));
        parts += QLatin1String(", ") + colorDoubleToQString(color.alphaF());
    }
}

void cssRgbPercentToQString(const QColor &color, QString &prefix,
                            QString &parts)
{
    int rP = qRound(color.redF() * 100);
    int gP = qRound(color.greenF() * 100);
    int bP = qRound(color.blueF() * 100);

    prefix = QLatin1String("rgb(");
    parts = QString::number(rP) + QChar::fromLatin1('%') + QLatin1String(", ")
            + QString::number(gP)
            + QChar::fromLatin1('%') + QLatin1String(", ")
            + QString::number(bP) + QChar::fromLatin1('%');

    qreal alpha = color.alphaF();

    if (alpha < 1.0) {
        prefix.insert(3, QLatin1Char('a'));
        parts += QLatin1String(", ") + colorDoubleToQString(alpha);
    }

}

void qssHsvToQString(const QColor &color, QString &prefix, QString &parts)
{
    prefix = QLatin1String("hsv(");
    parts = QString::number(color.hsvHue()) + QLatin1String(", ")
            + QString::number(color.hsvSaturation()) + QLatin1String(", ")
            + QString::number(color.value());

    int aP = qRound(color.alphaF() * 100);

    if (aP < 100) {
        prefix.insert(3, QLatin1Char('a'));
        parts += QLatin1String(", ") + QString::number(aP)
                + QChar::fromLatin1('%');
    }
}

void cssHslToQString(const QColor &color, QString &prefix, QString &parts)
{
    prefix = QLatin1String("hsl(");

    int sP = qRound(color.hslSaturationF() * 100);
    int lP = qRound(color.lightnessF() * 100);

    parts = QString::number(color.hslHue()) + QLatin1String(", ")
            + QString::number(sP) + QChar::fromLatin1('%')
            + QLatin1String(", ")
            + QString::number(lP) + QChar::fromLatin1('%');

    qreal alpha = color.alphaF();
    if (alpha < 1.0) {
        prefix.insert(3, QLatin1Char('a'));
        parts += QLatin1String(", ") + colorDoubleToQString(color.alphaF());
    }
}

void qmlRgbaToQString(const QColor &color, QString &prefix, QString &parts)
{
    prefix = QLatin1String("Qt.rgba(");
    parts = colorDoubleToQString(color.redF()) + QLatin1String(", ")
            + colorDoubleToQString(color.greenF()) + QLatin1String(", ")
            + colorDoubleToQString(color.blueF()) + QLatin1String(", ")
            + colorDoubleToQString(color.alphaF());
}

void qmlHslaToQString(const QColor &color, QString &prefix, QString &parts)
{
    prefix = QLatin1String("Qt.hsla(");
    parts = colorDoubleToQString(color.hueF()) + QLatin1String(", ")
            + colorDoubleToQString(color.saturationF()) + QLatin1String(", ")
            + colorDoubleToQString(color.lightnessF()) + QLatin1String(", ")
            + colorDoubleToQString(color.alphaF());

}

void glslColorToQString(const QColor &color, QString &prefix, QString &parts)
{
    prefix = QLatin1String("vec");

    parts = colorDoubleToQString(color.redF()) + QLatin1String(", ")
            + colorDoubleToQString(color.greenF()) + QLatin1String(", ")
            + colorDoubleToQString(color.blueF());

    qreal alpha = color.alphaF();
    if (alpha < 1.0) {
        prefix.append(QLatin1Char('4'));
        parts += QLatin1String(", ") + colorDoubleToQString(color.alphaF());
    } else {
        prefix.append(QLatin1Char('3'));
    }

    prefix.append(QLatin1Char('('));

}

void hexColorToQString(const QColor &color, QString &prefix, QString &parts)
{
    prefix = QLatin1String("#");

    int alpha = color.alpha();

    if (alpha < 255)
        parts.sprintf("%02x%02x%02x%02x",
                      alpha, color.red(), color.green(), color.blue());
    else
        parts.sprintf("%02x%02x%02x",
                      color.red(), color.green(), color.blue());

    parts = parts.toUpper();
}

} // anon namespace

namespace ColorPicker {
namespace Internal {

ColorFormatSet formatsFromCategory(ColorCategory category)
{
    ColorFormatSet ret;

    switch (category) {
    case ColorCategory::AnyCategory:
        ret << QCssRgbUCharFormat << QCssRgbPercentFormat
            << QssHsvFormat
            << CssHslFormat
            << QmlRgbaFormat << QmlHslaFormat
            << GlslFormat
            << HexFormat;
        break;
    case ColorCategory::QssCategory:
        ret << QCssRgbUCharFormat << QCssRgbPercentFormat
            << QssHsvFormat
            << HexFormat;
        break;
    case ColorCategory::CssCategory:
        ret << QCssRgbUCharFormat << QCssRgbPercentFormat
            << CssHslFormat
            << HexFormat;
        break;
    case ColorCategory::QmlCategory:
        ret << QmlRgbaFormat << QmlHslaFormat;
        break;
    case ColorCategory::GlslCategory:
        ret << GlslFormat;
        break;
    default:
        break;
    }

    Q_ASSERT(ret.size());

    return ret;
}

QColor parseColor(ColorFormat format, const QRegularExpressionMatch &match)
{
    QColor ret;

    if (format == ColorFormat::QCssRgbUCharFormat) {
        parseQCssRgbUChar(match, ret);
    }
    if (format == ColorFormat::QCssRgbPercentFormat) {
        parseCssRgbPercent(match, ret);
    }
    else if (format == ColorFormat::QssHsvFormat) {
        parseQssHsv(match, ret);
    }
    else if (format == ColorFormat::CssHslFormat) {
        parseCssHsl(match, ret);
    }
    else if (format == ColorFormat::QmlRgbaFormat) {
        parseQmlRgba(match, ret);
    }
    else if (format == ColorFormat::QmlHslaFormat) {
        parseQmlHsla(match, ret);
    }
    else if (format == ColorFormat::GlslFormat) {
        parseGlslColor(match, ret);
    }
    else if (format == ColorFormat::HexFormat) {
        parseHexColor(match, ret);
    }

    Q_ASSERT_X(ret.isValid(), Q_FUNC_INFO, "The color cannot be invalid.");

    return ret;
}

QString colorToString(const QColor &color, ColorFormat format)
{
    QString ret;

    QString prefix;
    QString colorParts;

    if (format == ColorFormat::QCssRgbUCharFormat) {
        cssRgbUCharToQString(color, prefix, colorParts);
    }
    if (format == ColorFormat::QCssRgbPercentFormat) {
        cssRgbPercentToQString(color, prefix, colorParts);
    }
    else if (format == ColorFormat::QssHsvFormat) {
        qssHsvToQString(color, prefix, colorParts);
    }
    else if (format == ColorFormat::CssHslFormat) {
        cssHslToQString(color, prefix, colorParts);
    }
    else if (format == ColorFormat::QmlRgbaFormat) {
        qmlRgbaToQString(color, prefix, colorParts);
    }
    else if (format == ColorFormat::QmlHslaFormat) {
        qmlHslaToQString(color, prefix, colorParts);
    }
    else if (format == ColorFormat::GlslFormat) {
        glslColorToQString(color, prefix, colorParts);
    }
    else if (format == ColorFormat::HexFormat) {
        hexColorToQString(color, prefix, colorParts);
    }

    Q_ASSERT(!prefix.isNull());
    Q_ASSERT(!colorParts.isNull());

    ret = prefix + colorParts;

    if (format != ColorFormat::HexFormat)
        ret += QChar::fromLatin1(')');

    Q_ASSERT_X(!ret.isNull(), Q_FUNC_INFO, "The string version of the color is invalid");

    return ret;
}

} // namespace Internal
} // namespace ColorPicker
