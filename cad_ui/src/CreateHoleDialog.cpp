#include "cad_ui/CreateHoleDialog.h"
#include "cad_ui/QtOccView.h"
#include <QMessageBox>
#include <QFormLayout>
#pragma execution_character_set("utf-8")

namespace cad_ui {

    CreateHoleDialog::CreateHoleDialog(QtOccView* viewer, QWidget* parent)
        : QDialog(parent), m_isSelectingFace(false), m_viewer(viewer) {
        setupUI();
        setModal(false);
        setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
        resize(380, 500);
        
    }

    // 实现析构函数，确保对话框关闭时恢复实体外观
    CreateHoleDialog::~CreateHoleDialog() {
        if (m_viewer && m_transparentShape) {
            m_viewer->ResetShapeDisplay(m_transparentShape);
        }
    }

void CreateHoleDialog::setupUI() {
    setWindowTitle("挖孔操作");
    // 使用明确的类型替换 auto*
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

	// face selection group
    m_selectionGroup = new QGroupBox("选择", this);
    auto* selectionLayout = new QVBoxLayout(m_selectionGroup); 

    m_selectFaceButton = new QPushButton("选择要挖孔的面", this);
    m_selectionList = new QListWidget(this);
    m_selectionList->setMaximumHeight(60); // 限制列表高度
    m_selectionList->addItem("尚未选择面...");
    m_selectionList->setStyleSheet("QListWidget::item { color: #888; }");

    selectionLayout->addWidget(m_selectFaceButton);
    selectionLayout->addWidget(m_selectionList);

	// parameters group
    m_parametersGroup = new QGroupBox("参数", this);
    QFormLayout* parametersLayout = new QFormLayout(m_parametersGroup);

    m_diameterSpinBox = new QDoubleSpinBox(this);
    m_diameterSpinBox->setRange(0.1, 1000.0);
    m_diameterSpinBox->setValue(5.0);
    m_diameterSpinBox->setSuffix(" mm");
    parametersLayout->addRow("直径:", m_diameterSpinBox);

    m_depthSpinBox = new QDoubleSpinBox(this);
    m_depthSpinBox->setRange(0.1, 1000.0);
    m_depthSpinBox->setValue(5.0);
    m_depthSpinBox->setSuffix(" mm");
    parametersLayout->addRow("深度:", m_depthSpinBox);

    parametersLayout->addRow(new QLabel("--- 孔中心坐标 ---")); 
    m_xCoordSpinBox = new QDoubleSpinBox(this);
    m_xCoordSpinBox->setRange(-1000.0, 1000.0);
    m_xCoordSpinBox->setValue(0.0);
    parametersLayout->addRow("坐标 X:", m_xCoordSpinBox);

    m_yCoordSpinBox = new QDoubleSpinBox(this);
    m_yCoordSpinBox->setRange(-1000.0, 1000.0);
    m_yCoordSpinBox->setValue(0.0);
    parametersLayout->addRow("坐标 Y:", m_yCoordSpinBox);

    m_zCoordSpinBox = new QDoubleSpinBox(this);
    m_zCoordSpinBox->setRange(-1000.0, 1000.0);
    m_zCoordSpinBox->setValue(0.0);
    parametersLayout->addRow("坐标 Z:", m_zCoordSpinBox);

	// botton layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    m_okButton->setEnabled(false); // 默认不可用，直到选择了面

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);

    mainLayout->addWidget(m_selectionGroup);
    mainLayout->addWidget(m_parametersGroup);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

	// connect signals and slots
    connect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectFaceClicked);
    connect(m_okButton, &QPushButton::clicked, this, &CreateHoleDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void CreateHoleDialog::onSelectFaceClicked() {
    m_isSelectingFace = true;
    m_selectFaceButton->setText("完成");

    emit selectionModeChanged(true, "请选择一个面用于挖孔");

	// switch to selection mode
    disconnect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectFaceClicked);
    connect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectionFinished);
}

void CreateHoleDialog::cleanupAndRestoreView() {
    if (m_viewer && m_transparentShape) {
        m_viewer->ResetShapeDisplay(m_transparentShape);
    }
    // 关键：清空指针，切断与旧实体的任何联系
    m_transparentShape = nullptr;
}

void CreateHoleDialog::onObjectSelected(const cad_core::ShapePtr& shape) {
    if (m_isSelectingFace) {
        m_targetShape = shape; 
    }
}


void CreateHoleDialog::onFaceSelected(const TopoDS_Face& face) {
    if (m_isSelectingFace) {
        m_selectedFace = face;

        // 关键逻辑：
        // 此时 m_targetShape 应该已经被 onObjectSelected 设置为当前面的父实体。
        if (m_viewer && m_targetShape) {

            // 1. 如果之前有别的实体是半透明的，先把它恢复原状
            if (m_transparentShape && m_transparentShape != m_targetShape) {
                m_viewer->ResetShapeDisplay(m_transparentShape);
            }

            // 2. 将当前面所属的实体设置为半透明
            m_viewer->SetShapeTransparency(m_targetShape, 0.5); // 设置为50%透明度

            // 3. 记录下哪个实体被我们变透明了，以便之后恢复
            m_transparentShape = m_targetShape;
        }

        updateSelectionDisplay();
        checkCanAccept();
    }
}

void CreateHoleDialog::onSelectionFinished() {
    m_isSelectingFace = false;
    m_selectFaceButton->setText("选择");

    emit selectionModeChanged(false, "");

	// reset target shape and selected face
    disconnect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectionFinished);
    connect(m_selectFaceButton, &QPushButton::clicked, this, &CreateHoleDialog::onSelectFaceClicked);
}

void CreateHoleDialog::updateSelectionDisplay() {
    m_selectionList->clear();
    if (!m_selectedFace.IsNull()) {
        m_selectionList->addItem("已选择 1 个面");
        m_selectionList->setStyleSheet("QListWidget::item { color: green; }");
    }
    else {
        m_selectionList->addItem("尚未选择面...");
        m_selectionList->setStyleSheet("QListWidget::item { color: #888; }");
    }
}

void CreateHoleDialog::checkCanAccept() {
    bool canAccept = m_targetShape && !m_selectedFace.IsNull();
    m_okButton->setEnabled(canAccept);
}

void CreateHoleDialog::onAccept() {
    if (!m_targetShape || m_selectedFace.IsNull()) {
        QMessageBox::warning(this, "错误", "请先选择一个有效的面。");
        return;
    }
    double diameter = m_diameterSpinBox->value();
    double depth = m_depthSpinBox->value();

    // 读取坐标值
    double x = m_xCoordSpinBox->value();
    double y = m_yCoordSpinBox->value();
    double z = m_zCoordSpinBox->value();

    // 发射带有坐标信息的新信号
    emit operationRequested(m_targetShape, m_selectedFace, diameter, depth, x, y, z);
    accept();
}

void CreateHoleDialog::updateCenterCoords(double x, double y, double z) {
    m_xCoordSpinBox->setValue(x);
    m_yCoordSpinBox->setValue(y);
    m_zCoordSpinBox->setValue(z);
}

} // namespace cad_ui