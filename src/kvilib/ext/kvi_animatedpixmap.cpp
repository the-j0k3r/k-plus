/*
 * kvi_animatedpixmap.cpp
 *
 *  Created on: 30.07.2008
 *      Author: Alexey
 */

#include "kvi_animatedpixmap.h"
#include "kvi_settings.h"
#include <QImageReader>
#include <QHash>
#include <QMutexLocker>

KviAnimatedPixmap::KviAnimatedPixmap(QString fileName)
	:m_szFileName(fileName)
{
	m_bStarted            = 0;
	m_uCurrentFrameNumber = 0;

	connect(&m_timer,SIGNAL(timeout()),this,SLOT(animationTimerShot()));

	m_lFrames=KviAnimatedPixmapCache::load(fileName);
	start();
}

KviAnimatedPixmap::KviAnimatedPixmap(KviAnimatedPixmap* source)
	:m_szFileName(source->m_szFileName),
	m_uCurrentFrameNumber(source->m_uCurrentFrameNumber),
	m_bStarted(source->m_bStarted), //keep started state
	m_lFrames(source->m_lFrames)
{
	m_lFrames->refs++;
	connect(&m_timer,SIGNAL(timeout()),this,SLOT(animationTimerShot()));

	//restore started state
	if(isStarted() && (framesCount()>1))
	{
		m_timer.start(m_lFrames->at(m_uCurrentFrameNumber).delay);
	}
}

KviAnimatedPixmap::~KviAnimatedPixmap()
{
	// all pixmaps will be automagically deleted at
	// FrameInfo::~FrameInfo destructor:))

	KviAnimatedPixmapCache::free(m_lFrames);
}

void KviAnimatedPixmap::start()
{
	if(!isStarted() && (framesCount()>1))
	{
		m_timer.start(m_lFrames->at(m_uCurrentFrameNumber).delay);
		m_bStarted = true;
	}
}

void KviAnimatedPixmap::stop()
{
	if(isStarted())
	{
		m_timer.stop();
		m_bStarted = false;
	}
}

void KviAnimatedPixmap::animationTimerShot()
{
	m_uCurrentFrameNumber++;
	//Ensure, that we are not out of bounds
	m_uCurrentFrameNumber %= m_lFrames->count();

	emit(frameChanged());

	//run timer again
	m_timer.start(m_lFrames->at(m_uCurrentFrameNumber).delay);
}

void KviAnimatedPixmap::resize(QSize newSize,Qt::AspectRatioMode ratioMode)
{
	QSize curSize(size());
	curSize.scale(newSize,ratioMode);

	m_lFrames = KviAnimatedPixmapCache::resize(m_lFrames,curSize);
}
