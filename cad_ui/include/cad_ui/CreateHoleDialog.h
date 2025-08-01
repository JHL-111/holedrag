#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include "cad_core/Shape.h"
#include <TopoDS_Face.hxx>

// 前向声明
namespace cad_ui {
    class QtOccView;
}

namespace cad_ui {

class CreateHoleDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateHoleDialog(QWidget* parent = nullptr);
    ~CreateHoleDialog() = default;

    void onObjectSelected(const cad_core::ShapePtr& shape);
    void onFaceSelected(const TopoDS_Face& face);

signals:   
    void operationRequested(const cad_core::ShapePtr& targetShape, const TopoDS_Face& selectedFace, double diameter, double depth);
    void selectionModeChanged(bool enabled, const QString& prompt);

private slots:
    void onSelectFaceClicked();
    void onSelectionFinished();
    void onAccept();

private:
    void setupUI();
    void updateSelectionDisplay();
    void checkCanAccept();

    // 状态变量
    cad_core::ShapePtr m_targetShape;   // 存储被选中的实体
    TopoDS_Face m_selectedFace;         // 存储被选中的面
    bool m_isSelectingFace;             // 标记是否正处于“选择面”的状态

    // UI 控件
    QGroupBox* m_selectionGroup;
    QPushButton* m_selectFaceButton;
    QLabel* m_faceStatusLabel;

    QGroupBox* m_parametersGroup;
    QDoubleSpinBox* m_diameterSpinBox;
    QDoubleSpinBox* m_depthSpinBox;

    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};

} // namespace cad_ui#pragma once
