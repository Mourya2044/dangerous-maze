#include "mainmenuscene.h"
#include <QFont>
#include <QBrush>
#include <QColor>
#include <QGraphicsSceneMouseEvent>

MainMenuScene::MainMenuScene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
    : QGraphicsScene(x, y, width, height, parent)
{
    // Set background
    setBackgroundBrush(QBrush(Qt::black));

    // Add Title
    titleText = new QGraphicsTextItem("MAZE GAME");
    QFont titleFont = font();
    titleFont.setPointSize(48);
    titleFont.setBold(true);
    titleText->setFont(titleFont);
    titleText->setDefaultTextColor(Qt::yellow);
    titleText->setPos((width - titleText->boundingRect().width()) / 2, height / 2 - 100);
    addItem(titleText);

    // Add Start Button
    startButton = new QGraphicsTextItem("START");
    QFont buttonFont = font();
    buttonFont.setPointSize(24);
    startButton->setFont(buttonFont);
    startButton->setDefaultTextColor(Qt::white);
    startButton->setPos((width - startButton->boundingRect().width()) / 2, height / 2 + 20);
    addItem(startButton);
}

void MainMenuScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Check if the "START" button was clicked
    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
    if (item == startButton)
    {
        emit startGameClicked();
    }
    QGraphicsScene::mousePressEvent(event);
}
