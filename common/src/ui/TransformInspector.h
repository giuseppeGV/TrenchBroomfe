#pragma once

#include "NotifierConnection.h"
#include "ui/TabBook.h"

#include <memory>

class QDoubleSpinBox;
class QPushButton;
class QCheckBox;
class QComboBox;

namespace tb::ui
{
class MapDocument;

class TransformInspector : public TabBookPage
{
    Q_OBJECT

private:
    MapDocument& m_document;
    NotifierConnection m_selectionConnection;
    NotifierConnection m_symmetryConnection;

    // Transform
    QDoubleSpinBox* m_xInput = nullptr;
    QDoubleSpinBox* m_yInput = nullptr;
    QDoubleSpinBox* m_zInput = nullptr;
    QPushButton* m_applyButton = nullptr;

    // Symmetry
    QCheckBox* m_symmetryEnabled = nullptr;
    QComboBox* m_symmetryAxis = nullptr;

    // Repair
    QPushButton* m_repairButton = nullptr;

    bool m_updatingUi = false;

public:
    explicit TransformInspector(MapDocument& document, QWidget* parent = nullptr);
    ~TransformInspector() override;

private:
    void createGui();
    void connectSignals();
    void updateUi();
    void applyTransform();
    void repairConvexity();
};

} // namespace tb::ui
