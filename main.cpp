#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickPaintedItem>
#include <QPainter>
#include <QtMath>

class Grid : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QSize sizeInTiles READ sizeInTiles WRITE setSizeInTiles NOTIFY sizeInTilesChanged)
    Q_PROPERTY(int tileSize READ tileSize CONSTANT)

public:
    Grid() :
        mTileSize(32)
    {
        setSizeInTiles(QSize(10, 10));
    }

    void paint(QPainter *painter) override
    {
        for (int y = 0; y < mSizeInTiles.height(); ++y) {
            for (int x = 0; x < mSizeInTiles.width(); ++x) {
                const QPoint pos(x, y);
                Tile tile = mTiles.at(tilePosToIndex(pos));
                if (pos == mClosestAvailableTilePos)
                    painter->setBrush(QColor(QStringLiteral("lightgreen")));
                else if (tile.passable)
                    painter->setBrush(Qt::white);
                else
                    painter->setBrush(QColor(QStringLiteral("salmon")));

                painter->drawRect(x * mTileSize, y * mTileSize, mTileSize, mTileSize);
            }
        }

        painter->setBrush(QColor::fromRgb(0, 0, 71));
        painter->drawText(QRect(mStartPos.x() * mTileSize, mStartPos.y() * mTileSize, mTileSize, mTileSize), Qt::AlignCenter, QStringLiteral("x"));
    }

    Q_INVOKABLE void findClosestAvailableTile(const QPoint &pixelPos)
    {
        const QPoint tileStartPos = pixelPosToTilePos(pixelPos);
        if (!isWithinBounds(tileStartPos))
            return;

        mStartPos = clampPointWithinBounds(pixelPosToTilePos(pixelPos));
        if (mTiles[tilePosToIndex(mStartPos)].passable) {
            setClosestAvailableTilePos(mStartPos);
            return;
        }

        const int largestDimension = qMax(mSizeInTiles.width(), mSizeInTiles.height());

        // The most basic approach: go clockwise around the start position in a spiral.
        // It's dumb because it doesn't skip large portions of the search area that are not within the level bounds.
        for (int step = 0, size = 3; step < largestDimension; ++step, size += 2) {
            const QPoint topLeft = mStartPos - (QPoint(1, 1) * (step + 1));
            const QRect searchRect(topLeft, QSize(size, size));
            QPoint currentPos = topLeft;

            for (; currentPos.x() < searchRect.x() + searchRect.width(); ++currentPos.rx()) {
                if (setIfClosest(currentPos))
                    return;
            }

            --currentPos.rx();

            for (; currentPos.y() < searchRect.y() + searchRect.height(); ++currentPos.ry()) {
                if (setIfClosest(currentPos))
                    return;
            }

            --currentPos.ry();

            for (; currentPos.x() >= searchRect.x(); --currentPos.rx()) {
                if (setIfClosest(currentPos))
                    return;
            }

            ++currentPos.rx();


            for (; currentPos.y() > searchRect.y(); --currentPos.ry()) {
                if (setIfClosest(currentPos))
                    return;
            }

            // No available tile.
        }
    }

    Q_INVOKABLE void togglePassable(const QPoint &pixelPos)
    {
        auto tilePos = pixelPosToTilePos(pixelPos);
        if (!isWithinBounds(tilePos))
            return;

        if (tilePos == mStartPos)
            return;

        // TODO
        mTiles[tilePosToIndex(tilePos)].passable = !mTiles[tilePosToIndex(tilePos)].passable;

        update();
    }

    QSize sizeInTiles() const
    {
        return mSizeInTiles;
    }

    void setSizeInTiles(const QSize &sizeInTiles)
    {
        if (sizeInTiles.width() < 1 || sizeInTiles.height() < 1)
            return;

        if (sizeInTiles == mSizeInTiles)
            return;

        mSizeInTiles = sizeInTiles;
        mTiles.fill(Tile(), mSizeInTiles.width() * mSizeInTiles.height());
        updateSize();
        findClosestAvailableTile(mStartPos);
        update();
        emit sizeInTilesChanged();
    }

    int tileSize() const
    {
        return mTileSize;
    }

signals:
    void sizeInTilesChanged();

private:
    void componentComplete() override
    {
        QQuickPaintedItem::componentComplete();
        updateSize();
        update();
    }

    void updateSize()
    {
        setImplicitWidth(mSizeInTiles.width() * mTileSize);
        setImplicitHeight(mSizeInTiles.height() * mTileSize);
    }

    int tilePosToIndex(const QPoint &tilePos)
    {
        return tilePos.y() * mSizeInTiles.width() + tilePos.x();
    }

    QPoint pixelPosToTilePos(const QPoint &pixelPos)
    {
        return QPoint(qFloor(pixelPos.x() / mTileSize), qFloor(pixelPos.y() / mTileSize));
    }

    bool isWithinBounds(const QPoint &tilePos) const
    {
        return !(tilePos.x() < 0 || tilePos.x() >= mSizeInTiles.width()
                || tilePos.y() < 0 || tilePos.y() >= mSizeInTiles.height());
    }

    QPoint clampPointWithinBounds(QPoint tilePos) const
    {
        if (tilePos.x() < 0)
            tilePos.setX(0);
        if (tilePos.x() >= mSizeInTiles.width())
            tilePos.setX(mSizeInTiles.width() - 1);
        if (tilePos.y() < 0)
            tilePos.setY(0);
        if (tilePos.y() >= mSizeInTiles.height())
            tilePos.setY(mSizeInTiles.height() - 1);
        return tilePos;
    }

    void setClosestAvailableTilePos(const QPoint &tilePos)
    {
        mClosestAvailableTilePos = tilePos;
        update();
    }

    bool setIfClosest(const QPoint &tilePos)
    {
        qDebug() << "checking" << tilePos;
        if (isWithinBounds(tilePos) && mTiles[tilePosToIndex(tilePos)].passable) {
            setClosestAvailableTilePos(tilePos);
            return true;
        }
        return false;
    }

    QSize mSizeInTiles;
    int mTileSize;
    QPoint mStartPos;
    QPoint mEndPos;
    QPoint mClosestAvailableTilePos;

    struct Tile {
        Tile() :
            passable(true),
            sequenceIndex(-1)
        {
        }

        bool passable;
        int sequenceIndex;
    };

    QVector<Tile> mTiles;
};

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qmlRegisterType<Grid>("App", 1, 0, "Grid");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

#include "main.moc"
