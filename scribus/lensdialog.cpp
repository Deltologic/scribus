/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/**************************************************************************
*   Copyright (C) 2007 by Franz Schmid                                    *
*   franz.schmid@altmuehlnet.de                                           *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "lensdialog.h"
#include <QRadialGradient>
#include "util_icon.h"

LensItem::LensItem(QRectF geom, LensDialog *parent) : QGraphicsEllipseItem(geom)
{
	dialog = parent;
	strength = 100.0;
	setPen(QPen(Qt::black));
	QRadialGradient radialGrad(QPointF(0.5, 0.5), 1.0);
	radialGrad.setColorAt(0.0, QColor(255, 0, 0, 127));
	radialGrad.setColorAt(0.1, QColor(255, 0, 0, 127));
	radialGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
#if QT_VERSION  >= 0x040301
	radialGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
#endif
	setBrush(radialGrad);
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}

void LensItem::setStrength(double s)
{
	strength = s;
}

QVariant LensItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	QPainterPath path;
	switch (change)
	{
		case ItemSelectedChange:
			dialog->lensSelected(this);
			break;
		case ItemPositionHasChanged:
			dialog->setLensPositionValues(mapToScene(rect().center()));
			updateEffect();
			break;
		default:
			break;
	}
	return QGraphicsItem::itemChange(change, value);
}

void LensItem::updateEffect()
{
	LensItem *item;
	QPainterPath path = dialog->origPath;
	for (int a = 0; a < dialog->lensList.count(); a++)
	{
		item = dialog->lensList[a];
		path = lensDeform(path, item->mapToScene(item->rect().center()), item->rect().width() / 2.0, item->strength / 100.0);
	}
	dialog->modifiedPath = path;
	dialog->origPathItem->setPath(path);
}

QPainterPath LensItem::lensDeform(const QPainterPath &source, const QPointF &offset, double m_radius, double s)
{
	QPainterPath path;
	path.addPath(source);
	for (int i = 0; i < path.elementCount(); ++i)
	{
		const QPainterPath::Element &e = path.elementAt(i);
		double dx = e.x - offset.x();
		double dy = e.y - offset.y();
		double len = m_radius - sqrt(dx * dx + dy * dy);
		if (len > 0)
			path.setElementPositionAt(i, e.x - s * dx * len / m_radius, e.y - s * dy * len / m_radius);
	}
	return path;
}

LensDialog::LensDialog(QWidget* parent, FPointArray &path) : QDialog(parent)
{
	setupUi(this);
	buttonRemove->setEnabled(false);
	setModal(true);
	origPath = path.toQPainterPath(true);
	buttonZoomOut->setIcon(QIcon(loadIcon("16/zoom-out.png")));
	buttonZoomI->setIcon(QIcon(loadIcon("16/zoom-in.png")));
	modifiedPath = origPath;
	origPathItem = scene.addPath(origPath);
	previewWidget->setRenderHint(QPainter::Antialiasing);
	previewWidget->setScene(&scene);
	previewWidget->show();
	connect(spinXPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensX(double)));
	connect(spinYPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensY(double)));
	connect(spinRadius, SIGNAL(valueChanged(double)), this, SLOT(setNewLensRadius(double)));
	connect(spinStrength, SIGNAL(valueChanged(double)), this, SLOT(setNewLensStrength(double)));
	connect(buttonAdd, SIGNAL(clicked()), this, SLOT(addLens()));
	connect(buttonRemove, SIGNAL(clicked()), this, SLOT(removeLens()));
	connect(buttonZoomI, SIGNAL(clicked()), this, SLOT(doZoomIn()));
	connect(buttonZoomOut, SIGNAL(clicked()), this, SLOT(doZoomOut()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	addLens();
}

void LensDialog::doZoomIn()
{
	previewWidget->scale(2.0, 2.0);
}

void LensDialog::doZoomOut()
{
	previewWidget->scale(0.5, 0.5);
}

void LensDialog::addLens()
{
	disconnect(spinXPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensX(double)));
	disconnect(spinYPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensY(double)));
	disconnect(spinRadius, SIGNAL(valueChanged(double)), this, SLOT(setNewLensRadius(double)));
	disconnect(spinStrength, SIGNAL(valueChanged(double)), this, SLOT(setNewLensStrength(double)));
	QRectF bBox = origPath.boundingRect();
	double r = qMin(bBox.width(), bBox.height());
	double x = (bBox.width() - r) / 2.0;
	double y = (bBox.height() - r) / 2.0;
	LensItem *item = new LensItem(QRectF(x, y, r, r), this);
	scene.addItem(item);
	lensList.append(item);
	currentLens = lensList.count() - 1;
	spinXPos->setValue(x + r / 2.0);
	spinYPos->setValue(y + r / 2.0);
	spinRadius->setValue(r / 2.0);
	spinStrength->setValue(100.0);
	lensList[currentLens]->updateEffect();
	if (lensList.count() > 1)
		buttonRemove->setEnabled(true);
	connect(spinXPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensX(double)));
	connect(spinYPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensY(double)));
	connect(spinRadius, SIGNAL(valueChanged(double)), this, SLOT(setNewLensRadius(double)));
	connect(spinStrength, SIGNAL(valueChanged(double)), this, SLOT(setNewLensStrength(double)));
}

void LensDialog::removeLens()
{
	LensItem *item = lensList.takeAt(currentLens);
	scene.removeItem(item);
	delete item;
	if (lensList.count() > 1)
		buttonRemove->setEnabled(true);
	else
		buttonRemove->setEnabled(false);
	currentLens = lensList.count() - 1;
	lensList[currentLens]->setSelected(true);
	lensList[currentLens]->updateEffect();
}

void LensDialog::lensSelected(LensItem *item)
{
	disconnect(spinXPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensX(double)));
	disconnect(spinYPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensY(double)));
	disconnect(spinRadius, SIGNAL(valueChanged(double)), this, SLOT(setNewLensRadius(double)));
	disconnect(spinStrength, SIGNAL(valueChanged(double)), this, SLOT(setNewLensStrength(double)));
	QPointF p = item->mapToScene(item->rect().center());
	spinXPos->setValue(p.x());
	spinYPos->setValue(p.y());
	spinRadius->setValue(item->rect().width() / 2.0);
	spinStrength->setValue(item->strength);
	currentLens = lensList.indexOf(item);
	if (currentLens < 0)
		currentLens = 0;
	connect(spinXPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensX(double)));
	connect(spinYPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensY(double)));
	connect(spinRadius, SIGNAL(valueChanged(double)), this, SLOT(setNewLensRadius(double)));
	connect(spinStrength, SIGNAL(valueChanged(double)), this, SLOT(setNewLensStrength(double)));
}

void LensDialog::setLensPositionValues(QPointF p)
{
	disconnect(spinXPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensX(double)));
	disconnect(spinYPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensY(double)));
	spinXPos->setValue(p.x());
	spinYPos->setValue(p.y());
	connect(spinXPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensX(double)));
	connect(spinYPos, SIGNAL(valueChanged(double)), this, SLOT(setNewLensY(double)));
}

void LensDialog::setNewLensX(double x)
{
	QRectF r = lensList[currentLens]->rect();
	r.moveCenter(QPointF(x, r.center().y()));
	lensList[currentLens]->setRect(r);
	lensList[currentLens]->updateEffect();
}

void LensDialog::setNewLensY(double y)
{
	QRectF r = lensList[currentLens]->rect();
	r.moveCenter(QPointF(r.center().x(), y));
	lensList[currentLens]->setRect(r);
	lensList[currentLens]->updateEffect();
}

void LensDialog::setNewLensRadius(double radius)
{
	QRectF r = lensList[currentLens]->rect();
	QPointF center = r.center();
	r.setWidth(radius * 2.0);
	r.setHeight(radius * 2.0);
	QPointF centerN = r.center();
	r.translate(center.x() - centerN.x(), center.y() - centerN.y());
	setLensPositionValues(lensList[currentLens]->mapToScene(r.center()));
	lensList[currentLens]->setRect(r);
	lensList[currentLens]->updateEffect();
}

void LensDialog::setNewLensStrength(double s)
{
	lensList[currentLens]->setStrength(s);
	lensList[currentLens]->updateEffect();
}