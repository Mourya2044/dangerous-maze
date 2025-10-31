#ifndef MAINMENUSCENE_H
#define MAINMENUSCENE_H

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>

class MainMenuScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit MainMenuScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);

signals:
    void startGameClicked();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsTextItem *titleText;
    QGraphicsTextItem *startButton;
};

#endif // MAINMENUSCENE_H