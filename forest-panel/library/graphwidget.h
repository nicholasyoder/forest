/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QDebug>


typedef  QHash<int, int> inthash;
typedef  QHash<int, qreal> qrealhash;

class graphwidget : public QWidget
{
    Q_OBJECT

public:
    graphwidget(){}
    ~graphwidget(){}

    int numofgraphs = 0;
    QList<inthash> valueslist;
    QList<qrealhash> alphaslist;

    QList<QColor> colors;
    QList<qreal> opacitys;

    QColor backcolor;
    qreal backopacity;

public slots:
    void setupgraphs(int numberofgraphs, QList<QColor> gcolors, QList<qreal> gopacitys, QColor gbackcolor, qreal gbackopacity)
    {
        numofgraphs = numberofgraphs;
        valueslist.clear();
        for (int i = 0; i < numofgraphs; i++){
            inthash hash;
            valueslist.append(hash);
            qrealhash qrhash;
            alphaslist.append(qrhash);
        }

        colors = gcolors;
        opacitys = gopacitys;
        backcolor = gbackcolor;
        backopacity = gbackopacity;
    }

    void updategraph(QList<qreal> newvalues)
    {
        if (newvalues.count() != numofgraphs) return;

        for (int i = 0; i < numofgraphs; i++)
        {
            inthash currenthash = valueslist[i];
            qrealhash currentalphahash = alphaslist[i];
            qreal currentvalue = newvalues[i];

            //move values in hash up
            int a = 0, b = 1;
            while (a < width())
            {
                currenthash[a] = currenthash[b];
                currentalphahash[a] = currentalphahash[b];
                a++;
                b++;
            }

            //value is a percentage i.e. 0.25
            //height * value = num of pixels high the bar should be
            int barheight = int(currentvalue * height());
            currentalphahash[a-1] = currentvalue * height() - barheight;//just get decimal
            currenthash[a-1] = int(currentvalue * height());

            valueslist[i] = currenthash;
            alphaslist[i] = currentalphahash;
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        //Background~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        QPainter painter(this);
        painter.setOpacity(backopacity);
        painter.fillRect(0,0, width(), height(), backcolor);
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        for (int i = 0; i < numofgraphs; i++)//for each graph
        {
            inthash currenthash = valueslist[i]; //get value hash for current graph
            qrealhash currentalphahash = alphaslist[i];

            int b = 0;
            while(b < width())
            {
                QPainter painter(this);
                painter.setOpacity(opacitys[i]);
                int y = height() - currenthash[b];
                painter.fillRect(b, y, 1, currenthash[b], colors[i]);

                int alphapixY = y-1;
                if (alphapixY > 0){
                    QColor color = colors[i];
                    color.setAlpha(255 * currentalphahash[b]);
                    painter.fillRect(b, alphapixY, 1, currenthash[b], color);
                }

                b++;
            }
        }
    }
};

#endif // GRAPHWIDGET_H
