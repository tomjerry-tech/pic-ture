#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QVector3D>
#include <QMatrix4x4>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateCube();

private:
    Ui::MainWindow *ui;
    
    // 立方体的顶点
    std::vector<QVector3D> vertices;
    // 立方体的面（每个面由4个顶点索引组成）
    std::vector<std::vector<int>> faces;
    
    // 旋转角度
    float angleX = 0.0f;
    float angleY = 0.0f;
    float angleZ = 0.0f;
    
    // 定时器用于动画
    QTimer *timer;
    
    // 初始化立方体数据
    void initCube();
    // 绘制立方体
    void drawCube(QPainter &painter);
    // 计算面的法向量
    QVector3D calculateNormal(const std::vector<QVector3D>& faceVertices);
    // 判断面是否可见（消隐）
    bool isFaceVisible(const std::vector<QVector3D>& faceVertices);
    
    // 计算面的可见性得分
    float calculateVisibility(const std::vector<QVector3D>& faceVertices);
    
    // 根据可见性得分计算透明度
    int calculateAlpha(float visibility);
};
#endif // MAINWINDOW_H
