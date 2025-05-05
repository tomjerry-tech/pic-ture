#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QKeyEvent>
#include <QDebug>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 设置窗口标题
    setWindowTitle("立方体消隐");
    
    // 初始化立方体数据
    initCube();
    
    // 设置定时器，用于旋转动画
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateCube);
    timer->start(30); // 30毫秒更新一次
    
    // 设置窗口大小
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCube()
{
    // 清空之前的数据
    vertices.clear();
    faces.clear();
    
    // 定义立方体的8个顶点（以原点为中心）
    float size = 100.0f;
    vertices = {
        QVector3D(-size, -size, -size), // 0: 左下后
        QVector3D(size, -size, -size),  // 1: 右下后
        QVector3D(size, size, -size),   // 2: 右上后
        QVector3D(-size, size, -size),  // 3: 左上后
        QVector3D(-size, -size, size),  // 4: 左下前
        QVector3D(size, -size, size),   // 5: 右下前
        QVector3D(size, size, size),    // 6: 右上前
        QVector3D(-size, size, size)    // 7: 左上前
    };
    
    // 定义立方体的6个面（每个面由4个顶点索引组成，顺序为顺时针）
    faces = {
        {0, 3, 2, 1}, // 后面 (-z)
        {4, 5, 6, 7}, // 前面 (+z)
        {0, 4, 7, 3}, // 左面 (-x)
        {1, 2, 6, 5}, // 右面 (+x)
        {0, 1, 5, 4}, // 下面 (-y)
        {3, 7, 6, 2}  // 上面 (+y)
    };
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 设置背景色
    painter.fillRect(rect(), Qt::white);
    
    // 绘制立方体
    drawCube(painter);
}

void MainWindow::drawCube(QPainter &painter)
{
    // 创建变换矩阵
    QMatrix4x4 matrix;
    
    // 将坐标原点移到窗口中心
    matrix.translate(width() / 2, height() / 2, 0);
    
    // 应用旋转
    matrix.rotate(angleX, 1, 0, 0);
    matrix.rotate(angleY, 0, 1, 0);
    matrix.rotate(angleZ, 0, 0, 1);
    
    // 变换后的顶点
    std::vector<QVector3D> transformedVertices;
    for (const auto& vertex : vertices) {
        transformedVertices.push_back(matrix.map(vertex));
    }
    
    // 计算每个面的可见性和透明度
    std::vector<std::pair<float, int>> faceVisibility;
    for (size_t i = 0; i < faces.size(); ++i) {
        const auto& face = faces[i];
        // 获取面的顶点
        std::vector<QVector3D> faceVertices;
        for (int idx : face) {
            faceVertices.push_back(transformedVertices[idx]);
        }
        
        // 计算可见性得分（点积值）
        float visibility = calculateVisibility(faceVertices);
        faceVisibility.push_back(std::make_pair(visibility, i));
    }
    
    // 按可见性排序（从后向前绘制，确保半透明效果正确）
    std::sort(faceVisibility.begin(), faceVisibility.end());
    
    // 绘制每个面
    for (const auto& [visibility, faceIndex] : faceVisibility) {
        const auto& face = faces[faceIndex];
        
        // 获取面的顶点
        std::vector<QVector3D> faceVertices;
        for (int idx : face) {
            faceVertices.push_back(transformedVertices[idx]);
        }
        
        // 只有当可见性大于阈值时才绘制
        if (visibility > -0.3f) {  // 允许稍微背向视点的面也可见
            // 创建多边形路径
            QPainterPath path;
            path.moveTo(faceVertices[0].x(), faceVertices[0].y());
            for (size_t i = 1; i < faceVertices.size(); ++i) {
                path.lineTo(faceVertices[i].x(), faceVertices[i].y());
            }
            path.closeSubpath();
            
            // 设置面的颜色（根据面的索引设置不同颜色）
            QColor faceColor;
            switch (faceIndex) {
                case 0: faceColor = QColor(255, 0, 0); break;    // 红色
                case 1: faceColor = QColor(0, 255, 0); break;    // 绿色
                case 2: faceColor = QColor(0, 0, 255); break;    // 蓝色
                case 3: faceColor = QColor(255, 255, 0); break;  // 黄色
                case 4: faceColor = QColor(0, 255, 255); break;  // 青色
                case 5: faceColor = QColor(255, 0, 255); break;  // 紫色
                default: faceColor = QColor(128, 128, 128); break; // 灰色
            }
            
            // 根据可见性调整透明度
            int alpha = calculateAlpha(visibility);
            faceColor.setAlpha(alpha);
            
            // 填充面
            painter.setBrush(faceColor);
            painter.setPen(Qt::black);
            painter.drawPath(path);
        }
    }
}

// 计算面的可见性得分（点积值）
float MainWindow::calculateVisibility(const std::vector<QVector3D>& faceVertices)
{
    // 计算面的法向量
    QVector3D normal = calculateNormal(faceVertices);
    
    // 视点位置（在观察者位置，与旋转无关）
    QVector3D viewPoint(0, 0, 1000);
    
    // 面的中心点
    QVector3D center(0, 0, 0);
    for (const auto& v : faceVertices) {
        center += v;
    }
    center /= faceVertices.size();
    
    // 从面中心指向视点的向量
    QVector3D viewVector = viewPoint - center;
    viewVector.normalize();
    
    // 返回点积值作为可见性得分
    return QVector3D::dotProduct(normal, viewVector);
}

// 根据可见性得分计算透明度
int MainWindow::calculateAlpha(float visibility)
{
    // 设置过渡区间
    const float minVisibility = -0.3f;  // 最小可见性（完全透明）
    const float maxVisibility = 0.3f;   // 最大可见性（完全不透明）
    
    if (visibility <= minVisibility) {
        return 0;   // 完全透明
    } else if (visibility >= maxVisibility) {
        return 255; // 完全不透明
    } else {
        // 线性插值计算透明度
        float t = (visibility - minVisibility) / (maxVisibility - minVisibility);
        return static_cast<int>(t * 255);
    }
}

QVector3D MainWindow::calculateNormal(const std::vector<QVector3D>& faceVertices)
{
    // 计算面的法向量（使用前三个点）
    QVector3D v1 = faceVertices[1] - faceVertices[0];
    QVector3D v2 = faceVertices[2] - faceVertices[0];
    
    // 叉乘计算法向量
    return QVector3D::crossProduct(v1, v2).normalized();
}

bool MainWindow::isFaceVisible(const std::vector<QVector3D>& faceVertices)
{
    // 计算可见性得分
    float visibility = calculateVisibility(faceVertices);
    
    // 可见性大于阈值时认为面可见
    return visibility > 0.0f;
}

void MainWindow::updateCube()
{
    // 更新旋转角度
    angleY += 1.0f;
    if (angleY >= 360.0f) {
        angleY = 0.0f;
    }
    
    // 重绘
    update();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 键盘控制旋转
    switch (event->key()) {
        case Qt::Key_W:
            angleX -= 5.0f;
            break;
        case Qt::Key_S:
            angleX += 5.0f;
            break;
        case Qt::Key_A:
            angleY -= 5.0f;
            break;
        case Qt::Key_D:
            angleY += 5.0f;
            break;
        case Qt::Key_Q:
            angleZ -= 5.0f;
            break;
        case Qt::Key_E:
            angleZ += 5.0f;
            break;
        case Qt::Key_Space:
            // 空格键暂停/继续旋转
            if (timer->isActive()) {
                timer->stop();
            } else {
                timer->start(30);
            }
            break;
        case Qt::Key_R:
            // 重置旋转角度
            angleX = 0.0f;
            angleY = 0.0f;
            angleZ = 0.0f;
            break;
        default:
            QMainWindow::keyPressEvent(event);
    }
    
    // 重绘
    update();
}
