#include "cad_ui/CreateHoleDialog.h"
#include <QMessageBox>
#include <QFormLayout>
#pragma execution_character_set("utf-8")

namespace cad_ui {

CreateHoleDialog::CreateHoleDialog(QWidget* parent)
    : QDialog(parent), m_isSelectingFace(false) {
    setupUI();
    setModal(false); 
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    resize(380, 500);
}

void CreateHoleDialog::setupUI() {
    setWindowTitle("挖孔操作");
    // 使用明确的类型替换 auto*
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

	// face selection group
    m_selectionGroup = new QGroupBox("选择", this);
    QGridLayout* selectionLayout = new QGridLayout(m_selectionGroup);

    QLabel* faceLabel = new QLabel("定位面:", this);
    m_faceStatusLabel = new QLabel("未选择", this);
    m_faceStatusLabel->setStyleSheet("color: #888;");
    m_selectFaceButton = new QPushButton("选择", this);

    selectionLayout->addWidget(faceLabel, 0, 0);
    selectionLayout->addWidget(m_faceStatusLabel, 0, 1);
    selectionLayout->addWidget(m_selectFaceButton, 0, 2);

	// parameters group
    m_parametersGroup = new QGroupBox("参数", this);
    QFormLayout* parametersLayout = new QFormLayout(m_parametersGroup);

    m_diameterSpinBox = new QDoubleSpinBox(this);
    m_diameterSpinBox->setRange(0.1, 1000.0);
    m_diameterSpinBox->setValue(10.0);
    m_diameterSpinBox->setSuffix(" mm");
    parametersLayout->addRow("直径:", m_diameterSpinBox);

    m_depthSpinBox = new QDoubleSpinBox(this);
    m_depthSpinBox->setRange(0.1, 1000.0);
    m_depthSpinBox->setValue(20.0);
    m_depthSpinBox->setSuffix(" mm");
    parametersLayout->addRow("深度:", m_depthSpinBox);

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


void CreateHoleDialog::onObjectSelected(const cad_core::ShapePtr& shape) {
    if (m_isSelectingFace) {
        m_targetShape = shape; 
    }
}


void CreateHoleDialog::onFaceSelected(const TopoDS_Face& face) {
    if (m_isSelectingFace) {
        m_selectedFace = face;
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
    if (!m_selectedFace.IsNull()) {
        m_faceStatusLabel->setText("已选择 1 个面");
        m_faceStatusLabel->setStyleSheet("color: green;");
    }
    else {
        m_faceStatusLabel->setText("未选择");
        m_faceStatusLabel->setStyleSheet("color: #888;");
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
    emit operationRequested(m_targetShape, m_selectedFace, diameter, depth);
    accept();
}

} // namespace cad_ui