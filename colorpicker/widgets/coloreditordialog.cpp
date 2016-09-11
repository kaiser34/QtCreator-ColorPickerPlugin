#include "coloreditordialog.h"

#include <QPainter>
#include <QBoxLayout>

#include "coloreditor.h"

namespace ColorPicker {
namespace Internal {


////////////////////////// ColorEditorDialog //////////////////////////

ColorEditorDialog::ColorEditorDialog(QWidget *parent) :
    QDialog(parent),
    m_colorWidget(new ColorEditor(this))
{
    setWindowTitle("ColorPicker");
    setWindowFlags((windowFlags() | Qt::NoDropShadowWindowHint)
                   & ~Qt::WindowContextHelpButtonHint);

    auto layout = new QVBoxLayout(this);
    const int margin = 3;
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addWidget(m_colorWidget);
}

ColorEditor *ColorEditorDialog::colorWidget() const
{
    return m_colorWidget;
}

} // namespace Internal
} // namespace ColorPicker
