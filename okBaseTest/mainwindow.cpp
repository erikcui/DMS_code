#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hhconstants.h"
#include "hhsliderwidget.h"
#include <iostream>
#include "celexsensordll.h"
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QPainter>
#include <QComboBox>
#include "doubleslider.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QDesktopWidget>

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
//#define _LOG_TIME_CONSUMING_
#endif

#define PLAYBACK_TIMER  20

using namespace std;

SensorDataObserver::SensorDataObserver(SensorData *sensorData, QWidget *parent)
    : QWidget(parent)
    , m_imageFullPic(PIXELS_PER_COL, PIXELS_PER_ROW, QImage::Format_RGB888)
    , m_imageEventPic(PIXELS_PER_COL, PIXELS_PER_ROW, QImage::Format_RGB888)
    , m_pFrameBuffer1(NULL)
    , m_pFrameBuffer2(NULL)
    , m_emSensorMode(EventMode)
    , m_lFrameCount(0)
    , m_uiDisplayMode1(0)
    , m_uiDisplayMode2(1)
    , m_iDisplayFrameTime(30)
    , m_iRotateType(0)
    , m_iImage1X(10)
    , m_iImage2X(0)
    , m_iImageWidth(768)
    , m_iImageHeight(640)
    , m_bRealtimeDisplay(true)
    , m_bSaveBmps(false)
    , m_bPlayback(false)
    , m_bCreatingFramePaused(false)
{
    m_pSensorData = sensorData;
    m_pSensorData->registerData(this, emPixelData);
    m_pSensorData->registerData(this, emPicFrameBufferData);
    m_pSensorData->registerData(this, emSpecialEvent);

    m_pFullPicQueue = new CirDataQueue(100);
    m_pEventPicQueue = new CirDataQueue(100);

    m_pPlayTimer = new QTimer(this);
    connect(m_pPlayTimer, SIGNAL(timeout()), this, SLOT(onUpdateDisplay()));
    if (!m_bRealtimeDisplay)
        m_pPlayTimer->start(m_iDisplayFrameTime);

    m_pLabelBg = new QLabel(this);
    m_pLabelBg->setGeometry(810, 0, 768, 640);
    m_pLabelBg->hide();
    m_pLabelBg->setStyleSheet("background: black;");

    m_imageFullPic.fill(Qt::black);
    m_imageEventPic.fill(Qt::black);

	connect(this, SIGNAL(sgupdateImg(const QImage& , const QImage& )), &mDetectThread, SLOT(handleImageSlot(const QImage&, const QImage&)));
	
	connect(this, SIGNAL(sgupdateBuffer(const unsigned char* , const unsigned char* , const unsigned char*)),
		&mDetectThread, SLOT(handleBufferSlot(const unsigned char*, const unsigned char*, const unsigned char*)));

	connect(&mDetectThread, SIGNAL(rectReady(QRect,int)),
		this, SLOT(onRectReady(QRect,int)));
	mDetectThread.start();
}

SensorDataObserver::~SensorDataObserver()
{
    m_pSensorData->unregisterData(this, emPixelData);
    m_pSensorData->unregisterData(this, emPicFrameBufferData);
    m_pSensorData->unregisterData(this, emSpecialEvent);

    delete[] m_pFrameBuffer1;
    delete[] m_pFrameBuffer2;
    delete m_pFullPicQueue;
    delete m_pEventPicQueue;
}

void SensorDataObserver::updateData(emDataType type)
{
    if (NULL == m_pSensorData)
        return;
    if (type == emPixelData)
    {
        ;
    }
    else if (type == emSpecialEvent)
    {
        //cout << GetTickCount() << endl;
        unsigned char* pBuffer1 = NULL;
        unsigned char* pBuffer2 = NULL;
        if (m_emSensorMode == FullPictureMode)
        {
            pBuffer1 = m_pSensorData->getFullPicBuffer();
        }
        else
        {
            if (m_uiDisplayMode1 == 0)
                pBuffer1 = m_pSensorData->getFullPicBuffer();
            else if (m_uiDisplayMode1 == 1)
                pBuffer1 = m_pSensorData->getEventPicBuffer();
            else if (m_uiDisplayMode1 == 2)
                pBuffer1 = m_pSensorData->getEventPicBufferGray();
            else if (m_uiDisplayMode1 == 3)
                pBuffer1 = m_pSensorData->getEventPicBufferSuperimposed();
            else if (m_uiDisplayMode1 == 4)
                pBuffer1 = m_pSensorData->getEventPicBufferDenoisedBinary();
            else if (m_uiDisplayMode1 == 5)
                pBuffer1 = m_pSensorData->getEventPicBufferDenoisedGray();
            else if (m_uiDisplayMode1 == 6)
                pBuffer1 = m_pSensorData->getEventPicMultiSlice();
            else if (m_uiDisplayMode1 == 7)
                pBuffer1 = m_pSensorData->getEventPicMultiSliceDirection();
            else if (m_uiDisplayMode1 == 8)
                pBuffer1 = m_pSensorData->getEventPicMultiSliceSpeed();

            if (m_uiDisplayMode2 == 0)
                pBuffer2 = m_pSensorData->getFullPicBuffer();
            else if (m_uiDisplayMode2 == 1)
                pBuffer2 = m_pSensorData->getEventPicBuffer();
            else if (m_uiDisplayMode2 == 2)
                pBuffer2 = m_pSensorData->getEventPicBufferGray();
            else if (m_uiDisplayMode2 == 3)
                pBuffer2 = m_pSensorData->getEventPicBufferSuperimposed();
            else if (m_uiDisplayMode2 == 4)
                pBuffer2 = m_pSensorData->getEventPicBufferDenoisedBinary();
            else if (m_uiDisplayMode2 == 5)
                pBuffer2 = m_pSensorData->getEventPicBufferDenoisedGray();
            else if (m_uiDisplayMode2 == 6)
                pBuffer2 = m_pSensorData->getEventPicMultiSlice();
            else if (m_uiDisplayMode2 == 7)
                pBuffer2 = m_pSensorData->getEventPicMultiSliceDirection();
            else if (m_uiDisplayMode2 == 8)
                pBuffer2 = m_pSensorData->getEventPicMultiSliceSpeed();
        }

        if (m_bRealtimeDisplay)
        {
            updateImage(pBuffer1, pBuffer2);
			emit sgupdateBuffer(m_pSensorData->getFullPicBuffer(),
				m_pSensorData->getEventPicBuffer(), 
				m_pSensorData->getEventPicMultiSlice());
        }
        else
        {
            //cout << "m_pFullPicQueue->getLength = " << m_pFullPicQueue->getLength() << endl;
            if (m_pFullPicQueue->getLength() != 0 &&
                m_pFullPicQueue->getLength() >= (m_pFullPicQueue->getCapacity()-10))
            {
                m_bCreatingFramePaused = true;
                m_pWindow->pauseThread(true);
            }
            m_pFullPicQueue->enqueue(pBuffer1);
            if (m_emSensorMode != FullPictureMode)
                m_pEventPicQueue->enqueue(pBuffer2);
        }
    }
    else if (type == emPicFrameBufferData)
    {
        ;
    }
}

void SensorDataObserver::setPlayState(bool state)
{
    if (state)
    {
        m_pPlayTimer->start(m_iDisplayFrameTime);
        onUpdateDisplay();
    }
    else
    {
        m_pPlayTimer->stop();
    }
    m_bPlayback = true;
}

void SensorDataObserver::setDisplayTime(int time)
{
    m_iDisplayFrameTime = time;
    m_pPlayTimer->start(time);
}

int SensorDataObserver::getDisplayTime()
{
    return m_iDisplayFrameTime;
}

int SensorDataObserver::getRotateType()
{
    return m_iRotateType;
}

void SensorDataObserver::setRotateType(int type)
{
    m_iRotateType += type;
}

void SensorDataObserver::setWindow(MainWindow *pWindow)
{
    m_pWindow = pWindow;
}

void SensorDataObserver::clearFrames()
{
    m_pFullPicQueue->clear();
    m_pEventPicQueue->clear();
    m_imageEventPic.fill(Qt::black);
    m_imageFullPic.fill(Qt::black);
    update();
}

void SensorDataObserver::setPlayback(bool bPlayback)
{
    m_bRealtimeDisplay = !bPlayback;
    if (bPlayback)
    {
        if (!m_pPlayTimer->isActive())
            m_pPlayTimer->start(m_iDisplayFrameTime);
        m_emSensorMode = EventMode;
    }
    else
    {
        m_pPlayTimer->stop();
    }
}

void SensorDataObserver::setSensorMode(emSensorMode mode)
{
    if (mode == FullPictureMode)
    {
        m_imageEventPic.fill(Qt::black);
        m_pLabelBg->show();
    }
    else
    {
        m_pLabelBg->hide();
    }
    m_emSensorMode = mode;
}

void SensorDataObserver::setSaveBmp(bool save)
{
    m_bSaveBmps = save;
}

bool SensorDataObserver::isSavingBmp()
{
    return m_bSaveBmps;
}

void SensorDataObserver::saveBmps()
{  
    QDir dir;
    dir.cd(QCoreApplication::applicationDirPath());
    if (!dir.exists("image1"))
    {
        dir.mkdir("image1");
    }
    if (!dir.exists("image2"))
    {
        dir.mkdir("image2");
    }
    //
    QString name = QCoreApplication::applicationDirPath() + "/image1/Pic";
    name += QString::number(m_lFrameCount);
    name += ".jpg";

    char nn[256] = {0};
    memcpy(nn, name.toStdString().c_str(), name.size());
    m_imageFullPic.save(nn, "JPG");

    if (m_emSensorMode != FullPictureMode)
    {
        QString name1 = QCoreApplication::applicationDirPath() + "/image2/Pic";
        name1 += QString::number(m_lFrameCount);
        name1 += ".jpg";
        memcpy(nn, name1.toStdString().c_str(), name1.size());
        m_imageEventPic.save(nn, "JPG");
    }
    ++m_lFrameCount;
}

void SensorDataObserver::updateImage(unsigned char *pBuffer1, unsigned char *pBuffer2)
{
#ifdef _LOG_TIME_CONSUMING_
    struct timeval tv_begin, tv_end;
    gettimeofday(&tv_begin, NULL);
#endif

    int row = 0, col = 0, index = 0, value = 0;
    uchar* pp1 = m_imageFullPic.bits();
    uchar* pp2 = m_imageEventPic.bits();
    for (int i = 0; i < PIXELS_PER_ROW; ++i)
    {
        for (int j = 0; j < PIXELS_PER_COL; ++j)
        {
            if (0 == m_iRotateType)
            {
                col = j;
                row = i;
            }
            else if (1 == m_iRotateType) //left right rotate
            {
                col = PIXELS_PER_COL-j-1;
                row = i;
            }
            else if (2 == m_iRotateType) //up down rotate
            {
                col = j;
                row = PIXELS_PER_ROW-i-1;
            }
            else if (3 == m_iRotateType) //left right up down rotate
            {
                col = PIXELS_PER_COL-j-1;
                row = PIXELS_PER_ROW-i-1;
            }
            index = row*PIXELS_PER_COL+col;
            if (pBuffer1)
            {
                value = pBuffer1[index];
                if (m_uiDisplayMode1 <= 5)
                {
                    *pp1 = value;
                    *(pp1+1) = value;
                    *(pp1+2) = value;
                }
                else if (m_uiDisplayMode1 == 7) //multi-slice direction
                {
                    if (0 == value)
                    {
                        *pp1 = 0;
                        *(pp1+1) = 0;
                        *(pp1+2) = 0;
                    }
                    else if (value < 21 || value > 210) //30 300 red
                    {
                        *pp1 = 255;
                        *(pp1+1) = 0;
                        *(pp1+2) = 0;
                    }
                    else if (value > 32 && value <= 96) //45 135 blue
                    {
                        *pp1 = 0;
                        *(pp1+1) = 0;
                        *(pp1+2) = 255;
                    }
                    else if (value > 96 && value <= 159) //135 225 green
                    {
                        *pp1 = 0;
                        *(pp1+1) = 255;
                        *(pp1+2) = 0;
                    }
                    else if (value > 159 && value < 223) //225 315 yellow
                    {
                        *pp1 = 255;
                        *(pp1+1) = 255;
                        *(pp1+2) = 0;
                    }
                    else
                    {
                        *pp1 = 0;
                        *(pp1+1) = 0;
                        *(pp1+2) = 0;
                    }
                }
                else //multi-slice frame and speed
                {
                    if (0 == value)
                    {
                        *pp1 = 0;
                        *(pp1+1) = 0;
                        *(pp1+2) = 0;
                    }
                    else if (value < 50) //blue
                    {
                        *pp1 = 0;
                        *(pp1+1) = 0;
                        *(pp1+2) = 255;
                    }
                    else if (value < 100)
                    {
                        *pp1 = 0;
                        *(pp1+1) = 255;
                        *(pp1+2) = 255;
                    }
                    else if (value < 150) //green
                    {
                        *pp1 = 0;
                        *(pp1+1) = 255;
                        *(pp1+2) = 0;
                    }
                    else if (value < 200) //yellow
                    {
                        *pp1 = 255;
                        *(pp1+1) = 255;
                        *(pp1+2) = 0;
                    }
                    else //red
                    {
                        *pp1 = 255;
                        *(pp1+1) = 0;
                        *(pp1+2) = 0;
                    }
                }
                pp1+= 3;
            }
            if (pBuffer2)
            {
                value = pBuffer2[index];
                if (m_uiDisplayMode2 <= 5)
                {
                    *pp2 = value;
                    *(pp2+1) = value;
                    *(pp2+2) = value;
                }
                else if (m_uiDisplayMode2 == 7) //multi-slice direction
                {
                    if (0 == value)
                    {
                        *pp2 = 0;
                        *(pp2+1) = 0;
                        *(pp2+2) = 0;
                    }
                    else if (value < 21 || value > 210) //30 300 red
                    {
                        *pp2 = 255;
                        *(pp2+1) = 0;
                        *(pp2+2) = 0;
                    }
                    else if (value > 32 && value <= 96) //45 135 blue
                    {
                        *pp2 = 0;
                        *(pp2+1) = 0;
                        *(pp2+2) = 255;
                    }
                    else if (value > 96 && value <= 159) //135 225 green
                    {
                        *pp2 = 0;
                        *(pp2+1) = 255;
                        *(pp2+2) = 0;
                    }
                    else if (value > 159 && value < 223) //225 315 yellow
                    {
                        *pp2 = 255;
                        *(pp2+1) = 255;
                        *(pp2+2) = 0;
                    }
                    else
                    {
                        *pp2 = 0;
                        *(pp2+1) = 0;
                        *(pp2+2) = 0;
                    }
                }
                else //multi-slice frame and speed
                {
                    if (0 == value)
                    {
                        *pp2 = 0;
                        *(pp2+1) = 0;
                        *(pp2+2) = 0;
                    }
                    else if (value < 2) //blue 50
                    {
                        *pp2 = 0;
                        *(pp2+1) = 0;
                        *(pp2+2) = 255;
                    }
                    else if (value < 3) //100
                    {
                        *pp2 = 0;
                        *(pp2+1) = 255;
                        *(pp2+2) = 255;
                    }
                    else if (value < 4) //green 150
                    {
                        *pp2 = 0;
                        *(pp2+1) = 255;
                        *(pp2+2) = 0;
                    }
                    else if (value < 5) //yellow 200
                    {
                        *pp2 = 255;
                        *(pp2+1) = 255;
                        *(pp2+2) = 0;
                    }
                    else //red
                    {
                        *pp2 = 255;
                        *(pp2+1) = 0;
                        *(pp2+2) = 0;
                    }
                }
                pp2+= 3;
            }
        }
    }

#ifdef _LOG_TIME_CONSUMING_
    gettimeofday(&tv_end, NULL);
    cout << "updateImage time = " << tv_end.tv_usec - tv_begin.tv_usec << endl;
#endif
    if (m_bSaveBmps)
    {
        saveBmps();
    }
    update();
	//emit sgupdateImg(m_imageFullPic, m_imageEventPic);
}

void SensorDataObserver::setDisplayMode1(unsigned int mode)
{
    m_uiDisplayMode1 = mode;
    m_pFullPicQueue->clear();
    m_pEventPicQueue->clear();
}

void SensorDataObserver::setDisplayMode2(unsigned int mode)
{
    m_uiDisplayMode2 = mode;
    m_pFullPicQueue->clear();
    m_pEventPicQueue->clear();
}

void SensorDataObserver::setImagePos(int x1, int x2)
{
    m_iImage1X = x1;
    m_iImage2X = x2;
    int diffX = (x2 - x1);
    if (diffX >= 768)
    {
        m_iImageWidth = 768;
        m_iImageHeight = 640;
    }
    else
    {
        m_iImageWidth = diffX - 10;
        m_iImageHeight = m_iImageWidth * 640 / 768;
    }
    m_pLabelBg->setGeometry(m_iImage2X, 0, m_iImageWidth, m_iImageHeight);
}

void SensorDataObserver::paintEvent(QPaintEvent *event)
{
#ifdef _LOG_TIME_CONSUMING_
    struct timeval tv_begin, tv_end;
    gettimeofday(&tv_begin, NULL);
#endif

    Q_UNUSED(event);
    QPainter painter(this);
    if (m_iImageWidth == 768)
    {
        painter.drawPixmap(m_iImage1X, 0, QPixmap::fromImage(m_imageFullPic));
        if (m_emSensorMode != FullPictureMode)
            painter.drawPixmap(m_iImage2X, 0, QPixmap::fromImage(m_imageEventPic));
    }
    else
    {
        painter.drawPixmap(m_iImage1X, 0, QPixmap::fromImage(m_imageFullPic).scaled(m_iImageWidth, m_iImageHeight));
        if (m_emSensorMode != FullPictureMode)
            painter.drawPixmap(m_iImage2X, 0, QPixmap::fromImage(m_imageEventPic).scaled(m_iImageWidth, m_iImageHeight));
    }
	//QRect face_rect(100, 100, 100, 100);
	painter.drawRect(m_rect);
	if (m_dete_name_index == 1) {
		int size = sizeof("帅哥");
		int x = m_rect.x() + (m_rect.width() - m_rect.x() - size) / 2;
		int y = m_rect.y() - 10;
		int w = size;
		int h = 10;
		QString name = "帅哥";
		painter.drawText(x, y, name);
}
	

#ifdef _LOG_TIME_CONSUMING_
    gettimeofday(&tv_end, NULL);
    cout << "paintEvent time = " << tv_end.tv_usec - tv_begin.tv_usec << endl;
#endif
}

void SensorDataObserver::onUpdateDisplay()
{
    //cout << "SensorDataObserver::onUpdateDisplay: " << GetTickCount() << endl;
    if (m_pFullPicQueue->getLength() <= 10 && m_bCreatingFramePaused)
    {
        m_pWindow->pauseThread(false);
        m_bCreatingFramePaused = false;
    }
    unsigned char* pData1;
    if (!m_pFullPicQueue->dequeue(pData1))
    {
        pData1 = NULL;
    }
    unsigned char* pData2;
    if (!m_pEventPicQueue->dequeue(pData2))
    {
        pData2 = NULL;
    }
    //cout << "SensorDataObserver::onUpdateDisplay: " << m_pFullPicQueue->getLength() << endl;
    updateImage(pData1, pData2);
}

void SensorDataObserver::onRectReady(QRect rect,int index) {
	m_rect = rect;
	m_dete_name_index = index;
}

//-------- MainWindow --------
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
    , m_bFirstShowPBBoard(true)
    , m_iCreateFrameTime(60)
    , m_uiDisplayMode1(0)
    , m_uiDisplayMode2(1)
    , m_ulPlaybackStartPos(8)
    , m_ulPlaybackEndPos(8)
    , m_bPipeoutAllowed(true)
    , m_bUseEnglishName(true)
    , m_bRecording(false)
    , m_bADCEnabled(true)
    , m_bPaused(false)
    , m_bRepeatPlay(false)
    , m_pAdvancedSliderBg(NULL)
    , m_bGlobalLengthSet(false)
{
    ui->setupUi(this);
    this->setStyleSheet("background-color: rgb(200, 200, 200); ");
    move(100, 30);

    QPixmap pixmap(":/images/celepixel.png");
    setWindowIcon(QIcon(pixmap.scaled(QSize(48, 48))));
    this->setWindowTitle("CeleX Sensor Demo");

    m_pCelexSensor = new CelexSensorDLL;
    //open sensor
    m_pCelexSensor->openSensor("");
	//m_pCelexSensor->setSensorMode(FullPictureMode);
    //--- set fpn file API---
    QString strPath = QCoreApplication::applicationDirPath() + "/FPN.txt";
    if (!m_pCelexSensor->setFpnFile(strPath.toStdString()))
    {
        qDebug() << "Can't load FPN.txt or the file is invalid!";
    }
	
    m_pCelexSensor->setTimeBlock(60);

    //create read sensor data timer
    m_pPipeOutDataTimer = new QTimer(this);
    m_pPipeOutDataTimer->setSingleShot(true);
    connect(m_pPipeOutDataTimer, SIGNAL(timeout()), this, SLOT(onPipeoutDataTimer()));
    m_pPipeOutDataTimer->start(PIPEOUT_TIMER);

    m_pPlaybackTimer = new QTimer(this);
    m_pPlaybackTimer->setSingleShot(true);
    connect(m_pPlaybackTimer, SIGNAL(timeout()), this, SLOT(onPlaybackTimer()));

    m_pSensorDataObserver = new SensorDataObserver(m_pCelexSensor->getSensorDataObject(), this);
    m_pSensorDataObserver->show();
    //m_pSensorDataObserver->setGeometry(0, 220, 1588, 640);
    m_pSensorDataObserver->setWindow(this);

    createComboBoxStyle();

    QHBoxLayout* layoutBtn = new QHBoxLayout;
    createButtons(layoutBtn);

    QGridLayout* layoutSlider = new QGridLayout;
    createSliders(layoutSlider);

    QHBoxLayout* layoutComboBox = new QHBoxLayout;
    layoutComboBox->setContentsMargins(10, 0, 0, 0);
    layoutComboBox->addWidget(m_pComboBoxModeText1);
    layoutComboBox->addStretch();
    layoutComboBox->addWidget(m_pComboBoxModeText2);
    layoutComboBox->addStretch();

    QHBoxLayout* layoutPic = new QHBoxLayout;
    layoutPic->addWidget(m_pSensorDataObserver);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(layoutBtn);
    pMainLayout->addLayout(layoutSlider);
    pMainLayout->addLayout(layoutComboBox);
    pMainLayout->addLayout(layoutPic);
    pMainLayout->setStretchFactor(layoutBtn, 2);
    pMainLayout->setStretchFactor(layoutSlider, 2);
    pMainLayout->setStretchFactor(layoutComboBox, 1);
    pMainLayout->setStretchFactor(layoutPic, 12);
    this->setLayout(pMainLayout);

    m_pSensorDataObserver->raise();
    m_pComboBoxModeText1->raise();
    m_pComboBoxModeText2->raise();

    this->showMaximized();

    m_pPlayerTimer = new QTimer;
    connect(m_pPlayerTimer, SIGNAL(timeout()), this, SLOT(onSetCurrentTime()));

    HHSliderWidget* pSlider = new HHSliderWidget(this, 0, 300, 1, 100, 400, this);
    pSlider->setGeometry(80, 900, 500, 50);
    pSlider->setDisplayName("MultiSlice Time (ms): ");
    pSlider->setBiasType("SliceTime");
    pSlider->show();

    HHSliderWidget* pSlider1 = new HHSliderWidget(this, 1, 255, 1, 255, 400, this);
    pSlider1->setGeometry(1080, 900, 500, 50);
    pSlider1->setDisplayName("MultiSlice Count: ");
    pSlider1->setBiasType("SliceCount");
    pSlider1->show();

    m_pCelexSensor->setMultiSliceTime(100);
    m_pCelexSensor->setMultiSliceCount(255);
	m_pCelexSensor->setSensorMode(FullPictureMode);
}

void MainWindow::setRecording(bool bStart)
{
    if (!m_pCelexSensor->isSensorReady())
    {
        cout << "Sensor is not ready!" << endl;
        return;
    }
    QPushButton* pButton = (QPushButton*)m_pButtonGroup->button(4);
    if (bStart)
    {
        const QDateTime now = QDateTime::currentDateTime();
        const QString timestamp = now.toString(QLatin1String("yyyyMMdd_hhmmsszzz"));

        QString qstrBinName = QCoreApplication::applicationDirPath() + "/Recording_" + timestamp;
        if (FullPictureMode == m_pCelexSensor->getSensorMode())
            qstrBinName += "_F_";
        else if (EventMode == m_pCelexSensor->getSensorMode())
            qstrBinName += "_E_";
        else if (FullPic_Event_Mode == m_pCelexSensor->getSensorMode())
            qstrBinName += "_FE_";

        qstrBinName += QString::number(m_pCelexSensor->getClockRate());
        qstrBinName += "MHz.bin";
        std::string filePath = qstrBinName.toStdString();
        m_pCelexSensor->startRecording(filePath);
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: yellow; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F; }");
        pButton->setText("Stop Recording");
    }
    else
    {
        m_pCelexSensor->stopRecording();
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F; }");
        pButton->setText("Start Recording");
    }
    m_bRecording = bStart;
}

void MainWindow::enableADC(bool enable)
{
    QPushButton* pButton = (QPushButton*)m_pButtonGroup->button(2);
    if (enable)
    {
        m_pCelexSensor->enableADC(true);
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: yellow; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F; }");
        pButton->setText("Disable ADC");
    }
    else
    {
        m_pCelexSensor->enableADC(false);
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F; }");
        pButton->setText("Enable ADC");
    }
    m_bADCEnabled = enable;
}

void MainWindow::createButtons(QHBoxLayout* layout)
{
    QComboBox* pComboBox = new QComboBox(this);
    pComboBox->insertItem(0, "SensorMode - FullPic");
    pComboBox->insertItem(1, "SensorMode - Event");
    pComboBox->insertItem(2, "SensorMode - FullPic_Event"); //FullPic_Event
    connect(pComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onSensorModeChanged(QString)));
    pComboBox->setCurrentText("SensorMode - Event");
    QString style1 = "QComboBox {font: 20px Calibri; color: yellow; border: 2px solid darkgrey; "
                     "border-radius: 5px; background: #002F6F;}";
    QString style2 = "QComboBox:editable {background: #002F6F;}";
    pComboBox->setStyleSheet(style1 + style2);

    QStringList btnNameList;
    if (m_bUseEnglishName)
    {
        btnNameList.push_back("Reset ALL");
        btnNameList.push_back("Reset FPGA");
        btnNameList.push_back("Disable ADC");
        //btnNameList.push_back("Switch Sensor Mode");
        btnNameList.push_back("Trig FullPic");
        btnNameList.push_back("Start Recording");
        btnNameList.push_back("Playback");
        btnNameList.push_back("Generate FPN");
        //btnNameList.push_back("Advanced Settings");
        btnNameList.push_back("Rotate_LR");
        btnNameList.push_back("Rotate_UD");
    }
    else
    {
        btnNameList.push_back("复位ALL");
        btnNameList.push_back("复位FPGA");
        btnNameList.push_back("禁用ADC");
        //btnNameList.push_back("切换模式");
        btnNameList.push_back("生成FullPic");
        btnNameList.push_back("录制");
        btnNameList.push_back("回放");
        btnNameList.push_back("生成FPN文件");
        //btnNameList.push_back("高级设置");
        btnNameList.push_back("Rotate_LR");
        btnNameList.push_back("Rotate_UD");
    }

    m_pButtonGroup = new QButtonGroup;
    for (int i = 0; i < btnNameList.count(); ++i)
    {
        QPushButton* pButton = new QPushButton(btnNameList.at(i));
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F;}");
        m_pButtonGroup->addButton(pButton, i);
        if (i == 3)
        {
            layout->addWidget(pComboBox);
        }
        layout->addWidget(pButton);
    }
    connect(m_pButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonClicked(int)));
}

void MainWindow::createSliders(QGridLayout* layout)
{
    QStringList strDisplayNameList;
    if (m_bUseEnglishName)
    {
        strDisplayNameList.push_back("Lower ADC: ");
        strDisplayNameList.push_back("Upper ADC: ");
        strDisplayNameList.push_back("Brightness: ");
        strDisplayNameList.push_back(" Contrast: ");
        strDisplayNameList.push_back(" Threshold: ");
        strDisplayNameList.push_back("Time-Block (ms): ");
        strDisplayNameList.push_back("    Overlap (ms): ");
        strDisplayNameList.push_back("      Clock (MHz): ");
    }
    else
    {
        strDisplayNameList.push_back("Lower ADC: ");
        strDisplayNameList.push_back("Upper ADC: ");
        strDisplayNameList.push_back(QString("亮度: "));
        strDisplayNameList.push_back(QString("对比度: "));
        strDisplayNameList.push_back(QString("阈值: "));
        strDisplayNameList.push_back("建帧间隔（ms): ");
        strDisplayNameList.push_back("    Overlap (ms): ");
        strDisplayNameList.push_back("      Clock (MHz): ");
    }

    int row[8] = {0, 1, 0, 1, 2, 0, 1, 2};
    int col[8] = {0, 0, 1, 1, 1, 2, 2, 2};

    std::vector<CelexSensorDLL::ControlSliderInfo> controlList = m_pCelexSensor->getSensorControlList();
    int index = 0;
    for (std::vector<CelexSensorDLL::ControlSliderInfo>::iterator it = controlList.begin(); it != controlList.end(); it++)
    {
        CelexSensorDLL::ControlSliderInfo sliderInfo = *it;
        if (!sliderInfo.bAdvanced)
        {
            uint32_t min = sliderInfo.min;
            uint32_t max = sliderInfo.max;
            uint32_t value = sliderInfo.value;
            uint32_t step = sliderInfo.step;
            std::string name = sliderInfo.name;
            HHSliderWidget* pSlider = new HHSliderWidget(this, min, max, step, value, 400, this);
            pSlider->setBiasType(name);
            pSlider->setDisplayName(strDisplayNameList.at(index));
            pSlider->setObjectName(QString::fromStdString(name));      
            layout->addWidget(pSlider, row[index], col[index]);
            ++index;
        }
    }
}

void MainWindow::createComboBoxStyle()
{
    m_pComboBoxModeText1 = new QComboBox(this);
    m_pComboBoxModeText1->setGeometry(20, 190, 300, 30);
    m_pComboBoxModeText1->show();
    m_pComboBoxModeText1->insertItem(0, "EventMode - Accumulated Pic");
    m_pComboBoxModeText1->insertItem(1, "EventMode - Binary Pic");
    m_pComboBoxModeText1->insertItem(2, "EventMode - Gray Pic");
    m_pComboBoxModeText1->insertItem(3, "EventMode - Superimposed Pic");
    connect(m_pComboBoxModeText1, SIGNAL(currentIndexChanged(int)), this, SLOT(onDisplayMode1Changed(int)));
    QString style1 = "QComboBox {font: 18px Calibri; color: white; border: 2px solid darkgrey; "
                     "border-radius: 5px; background: green;}";
    QString style2 = "QComboBox:editable {background: green;}";
    m_pComboBoxModeText1->setStyleSheet(style1 + style2);
    m_pComboBoxModeText1->setCurrentText("EventMode - Accumulated Pic");

    m_pComboBoxModeText2 = new QComboBox(this);
    m_pComboBoxModeText2->setGeometry(810, 190, 300, 30);
    m_pComboBoxModeText2->show();
    m_pComboBoxModeText2->insertItem(0, "EventMode - Accumulated Pic");
    m_pComboBoxModeText2->insertItem(1, "EventMode - Binary Pic");
    m_pComboBoxModeText2->insertItem(2, "EventMode - Gray Pic");
    m_pComboBoxModeText2->insertItem(3, "EventMode - Superimposed Pic");
    connect(m_pComboBoxModeText2, SIGNAL(currentIndexChanged(int)), this, SLOT(onDisplayMode2Changed(int)));
    m_pComboBoxModeText2->setStyleSheet(style1 + style2);
    m_pComboBoxModeText2->setCurrentText("EventMode - Binary Pic");
}

void MainWindow::displayPlaybackBoard(bool display)
{
    if (!m_bFirstShowPBBoard)
    {
        if (display)
        {
            setEndTime();
            m_pWidgetBg->show();
        }
        else
            m_pWidgetBg->hide();
    }
    else
    {
        m_pWidgetBg = new QWidget(this);
        QString qsFontSize = "18px";
#ifdef _WIN32
        m_pWidgetBg->setGeometry(0, 50, this->width(), 120);
#else
        m_pWidgetBg->setGeometry(0, 50, this->width(), 140);
        qsFontSize = "16px";
#endif
        m_pWidgetBg->setStyleSheet("background-color: darkgray; ");

        m_pBtnPlayPause = new QPushButton(m_pWidgetBg);
        m_pBtnPlayPause->setGeometry(20, 35, 36, 36);
        m_pBtnPlayPause->setStyleSheet("QPushButton {background-color: #EEEEEE; background-image: url(:/images/player_pause.png); "
                                       "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #FFFFFF;} "
                                       "QPushButton:pressed {background: #992F6F; background-image: url(:/images/player_pause.png); }");
        connect(m_pBtnPlayPause, SIGNAL(released()), this, SLOT(onBtnPlayReleased()));

        m_pLabelCurrentTime = new QLabel(m_pWidgetBg);
        m_pLabelCurrentTime->setGeometry(290, 20, 70, 40);
        m_pLabelCurrentTime->setAlignment(Qt::AlignCenter);
        m_pLabelCurrentTime->setText("00:00:00");
        m_pLabelCurrentTime->setStyleSheet("font: "+qsFontSize+" Calibri; color: black; ");

        m_pSliderPlayer = new DoubleSlider(m_pWidgetBg);
        m_pSliderPlayer->setGeometry(143, 38, 400, 35);
        m_pSliderPlayer->setRange(0, 400);
        m_pSliderPlayer->setValue(0);
        connect(m_pSliderPlayer, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
        connect(m_pSliderPlayer, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
        connect(m_pSliderPlayer, SIGNAL(valueChanged(ulong)), this, SLOT(onSliderValueChanged(ulong)));
        connect(m_pSliderPlayer, SIGNAL(minValueChanged(unsigned long)), this, SLOT(onMinValueChanged(unsigned long)));
        connect(m_pSliderPlayer, SIGNAL(maxValueChanged(unsigned long)), this, SLOT(onMaxValueChanged(unsigned long)));

        m_pLabelStartTime = new QLabel(m_pWidgetBg);
        m_pLabelStartTime->setGeometry(70, 35, 70, 40);
        m_pLabelStartTime->setAlignment(Qt::AlignCenter);
        m_pLabelStartTime->setText("00:00:00");
        m_pLabelStartTime->setStyleSheet("font: "+qsFontSize+" Calibri; color: black; ");

        m_pLabelEndTime = new QLabel(m_pWidgetBg);
        m_pLabelEndTime->setGeometry(550, 35, 70, 40);
        m_pLabelEndTime->setAlignment(Qt::AlignCenter);
        m_pLabelEndTime->setStyleSheet("font: "+qsFontSize+" Calibri; color: black; ");
        setEndTime();

        m_pBtnRepeat = new QPushButton("Repeat Off", m_pWidgetBg);
        m_pBtnRepeat->setGeometry(650, 40, 100, 30);
        m_pBtnRepeat->setStyleSheet("QPushButton {background-color: #EEEEEE; font: "+qsFontSize+" Calibri; "
                                    "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #FFFFFF;} "
                                    "QPushButton:pressed {background: #992F6F; }");
        connect(m_pBtnRepeat, SIGNAL(released()), this, SLOT(onBtnRepeatReleased()));

        m_pBtnSaveFile = new QPushButton("Save Bin File", m_pWidgetBg);
        m_pBtnSaveFile->setGeometry(770, 40, 120, 30);
        m_pBtnSaveFile->setStyleSheet("QPushButton {background-color: #EEEEEE; font: "+qsFontSize+" Calibri; "
                                      "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #FFFFFF;} "
                                      "QPushButton:pressed {background: #992F6F; }");
        connect(m_pBtnSaveFile, SIGNAL(released()), this, SLOT(onBtnSaveReleased()));

        m_pBtnSaveBmp = new QPushButton("Start Saving Bmp", m_pWidgetBg);
        m_pBtnSaveBmp->setGeometry(910, 40, 150, 30);
        m_pBtnSaveBmp->setStyleSheet("QPushButton {background-color: #EEEEEE; font: "+qsFontSize+" Calibri; "
                                     "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #FFFFFF;} "
                                     "QPushButton:pressed {background: #992F6F; }");
        connect(m_pBtnSaveBmp, SIGNAL(released()), this, SLOT(onBtnSaveBmpReleased()));

        HHSliderWidget* pSlider1 = new HHSliderWidget(m_pWidgetBg, 1, 200, 200, 60, 400, this);
        pSlider1->setGeometry(1080, 10, 500, 50);
        pSlider1->setBiasType("Time Slice");
        pSlider1->setDisplayName("Time-Slice (ms): ");
        pSlider1->setObjectName("Time Slice");

        HHSliderWidget* pSlider2 = new HHSliderWidget(m_pWidgetBg, 1, 200, 200, 30, 400, this);
        pSlider2->setGeometry(1080, 50, 500, 50);
        pSlider2->setBiasType("Display");
        pSlider2->setDisplayName("     Display (ms): ");
        pSlider2->setObjectName("Display");

        m_bFirstShowPBBoard = false;
        m_pWidgetBg->show();
    }
}

void MainWindow::setEndTime()
{
    CelexSensorDLL::TimeInfo timeInfo = m_pCelexSensor->getRecordedTime();
    setSliderTimeText(m_pLabelEndTime, timeInfo.hour, timeInfo.minute, timeInfo.second);
    m_pPlayerTimer->start(1000);
    m_timeCurrent.setHMS(0, 0, 0);
    m_ulMaxRange = timeInfo.hour*3600 + timeInfo.minute*60 + timeInfo.second;
    m_pSliderPlayer->setRange(0, m_ulMaxRange);
}

void MainWindow::setSliderTimeText(QLabel *pLabel, int hour, int minute, int second)
{
    QString qsHour;
    if (hour < 10)
        qsHour = "0" + qsHour.number(hour);
    else
        qsHour = qsHour.number(hour);

    QString qsMinute;
    if (minute < 10)
        qsMinute = "0" + qsHour.number(minute);
    else
        qsMinute = qsHour.number(minute);

    QString qsSecond;
    if (second < 10)
        qsSecond = "0" + qsHour.number(second);
    else
        qsSecond = qsHour.number(second);

    pLabel->setText(qsHour + ":" + qsMinute + ":" + qsSecond);
}

void MainWindow::stopPlayback()
{
    m_pPlaybackTimer->stop();
    m_pPlayerTimer->stop();
    m_pSensorDataObserver->clearFrames();
    m_pCelexSensor->clearData();
}

void MainWindow::displayAdvancedSliders()
{
    if (NULL == m_pAdvancedSliderBg)
    {
        m_pAdvancedSliderBg = new QWidget();
        m_pAdvancedSliderBg->setGeometry(50, 50, 1100, 450);

        QStringList strDisplayNameList;
        strDisplayNameList.push_back("EVT_VL: ");
        strDisplayNameList.push_back("ZONE-: ");
        strDisplayNameList.push_back("REF-: ");
        strDisplayNameList.push_back("REF-H: ");
        strDisplayNameList.push_back("EVT_DC: ");
        strDisplayNameList.push_back("CDS_DC: ");
        strDisplayNameList.push_back("PixBias: ");
        strDisplayNameList.push_back("Clock: ");

        strDisplayNameList.push_back("EVT_VH: ");
        strDisplayNameList.push_back("ZONE+: ");
        strDisplayNameList.push_back("REF+: ");
        strDisplayNameList.push_back("REF+H: ");
        strDisplayNameList.push_back("LEAK: ");
        strDisplayNameList.push_back("CDS_V1: ");
        strDisplayNameList.push_back("Gain: ");
        strDisplayNameList.push_back("Resolution: ");

        std::vector<CelexSensorDLL::ControlSliderInfo> controlList = m_pCelexSensor->getSensorControlList();
        int index = 0;
        for (std::vector<CelexSensorDLL::ControlSliderInfo>::iterator it = controlList.begin(); it != controlList.end(); it++)
        {
            CelexSensorDLL::ControlSliderInfo sliderInfo = *it;
            if (sliderInfo.bAdvanced)
            {
                int col = index%2;
                int row = index/2;
                uint32_t min = sliderInfo.min;
                uint32_t max = sliderInfo.max;
                uint32_t value = sliderInfo.value;
                uint32_t step = sliderInfo.step;
                std::string name = sliderInfo.name;
                HHSliderWidget* pSlider = new HHSliderWidget(m_pAdvancedSliderBg, min, max, step, value, 350, this);
                pSlider->show();
                pSlider->setGeometry(100+col*550, 20+row*50, 420, 35);
                pSlider->setBiasType(name);
                pSlider->setDisplayName(strDisplayNameList.at(index));
                //pSlider->setObjectName(QString::fromStdString(name));
                ++index;
            }
        }
    }
    m_pAdvancedSliderBg->show();
}

void MainWindow::generateFPN()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save FPN file", QCoreApplication::applicationDirPath(), "FPN Files (*.txt)");
    if (!filePath.isEmpty())
    {
        m_pCelexSensor->generateFPN(filePath.toStdString());
        cout << filePath.toStdString() << endl;
    }
}

void MainWindow::playback()
{
    stopPlayback();
    QString filePath = QFileDialog::getOpenFileName(this, "Open a bin file", QCoreApplication::applicationDirPath(), "Bin Files (*.bin)");
    if (filePath.isEmpty())
        return;

    if (m_pCelexSensor->openPlaybackFile(filePath.toStdString()))
    {
        // stop pipe out
        m_bPipeoutAllowed = false;
        m_pPlaybackTimer->start(PLAYBACK_TIMER);
    }
    displayPlaybackBoard(true);

    QPushButton* pButton = (QPushButton*)m_pButtonGroup->button(5);
    pButton->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                           "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                           "font: 20px Calibri; }"
                           "QPushButton:pressed {background: #992F6F; }");

    m_pSensorDataObserver->setPlayback(true);
    m_pSensorDataObserver->setSensorMode(m_pCelexSensor->getSensorMode());
    setDisplayMode(m_pCelexSensor->getSensorMode());
}

void MainWindow::onButtonClicked(int index)
{
    cout << "MainWindow::onButtonClicked: " << index << endl;
    switch (index)
    {
    case 0:
        m_pCelexSensor->resetDevice(CelexSensorDLL::SensorAndFPGA);
        break;

    case 1:
         m_pCelexSensor->resetDevice(CelexSensorDLL::FPGA);
        break;

    case 2:
        enableADC(!m_bADCEnabled);
        break;

    case 3:
        m_pCelexSensor->trigFullPic();
        break;

    case 4:
        setRecording(!m_bRecording);
        break;

    case 5:
        playback();
        break;

    case 6:
        generateFPN();
        break;

    case 7:
        //displayAdvancedSliders();
        onBtnRotateLRReleased();
        break;

    case 8:
        //displayAdvancedSliders();
        onBtnRotateUDReleased();
        break;

    default:
        break;
    }
}

void MainWindow::onValueChanged(uint32_t value, HHSliderWidget *slider)
{
    cout << __func__ << "  " << slider->getBiasType() << "  " << value << endl;
    if ("Contrast" == slider->getBiasType())
    {
        m_pCelexSensor->setContrast(value);
    }
    else if ("Brightness" == slider->getBiasType())
    {
        m_pCelexSensor->setBrightness(value);
    }
    else if ("Threshold" == slider->getBiasType())
    {
        m_pCelexSensor->setThreshold(value);
    }
    else if ("Lower ADC" == slider->getBiasType())
    {
        m_pCelexSensor->setLowerADC(value);
    }
    else if ("Upper ADC" == slider->getBiasType())
    {
        m_pCelexSensor->setUpperADC(value);
    }
    else if ("Time Slice" == slider->getBiasType())
    {
        if (m_pCelexSensor->getSensorMode() == EventMode)
        {
            m_pCelexSensor->setTimeBlock(value);
            if (!m_bPipeoutAllowed)
            {
                long max = m_pSensorDataObserver->getDisplayTime() * (float)m_ulMaxRange/(float)value;
                m_pSliderPlayer->setRange(0, max);
                int hour   = max / 3600;
                int minute = (max%3600)/60;
                int second = (max%3600)%60;
                setSliderTimeText(m_pLabelEndTime, hour, minute, second);
            }
        }
        else if (m_pCelexSensor->getSensorMode() == FullPic_Event_Mode)
        {
            m_pCelexSensor->setTimeBlock(value);
        }
    }
    else if ("Display" == slider->getBiasType())
    {
        if (!m_bPipeoutAllowed)
        {
            m_pSensorDataObserver->setDisplayTime(value);

            long max = (float)(value * m_ulMaxRange)/float(m_pCelexSensor->getTimeBlock());
            m_pSliderPlayer->setRange(0, max);
            int hour   = max / 3600;
            int minute = (max%3600)/60;
            int second = (max%3600)%60;
            setSliderTimeText(m_pLabelEndTime, hour, minute, second);
        }
    }
    else if ("Overlap" == slider->getBiasType())
    {
        m_pCelexSensor->setOverlapTime(value);
    }
    else if ("SliceTime" == slider->getBiasType())
    {
        m_pCelexSensor->setMultiSliceTime(value);
    }
    else if ("SliceCount" == slider->getBiasType())
    {
        m_pCelexSensor->setMultiSliceCount(value);
    }
    else if ("Clock" == slider->getBiasType())
    {
        m_pCelexSensor->setClockRate(value);
        if (FullPictureMode == m_pCelexSensor->getSensorMode())
        {
            if (value < 11)
                m_pCelexSensor->setTimeBlock(220);
            else
                m_pCelexSensor->setTimeBlock(40);
        }
        else if (FullPic_Event_Mode == m_pCelexSensor->getSensorMode())
        {
            if (value < 5)
                m_pCelexSensor->setTimeBlock(300);
            else if (value < 10)
                m_pCelexSensor->setTimeBlock(100);
            else
                m_pCelexSensor->setTimeBlock(60);
        }
    }
}

void MainWindow::onBtnPlayReleased()
{
    if (m_bPaused)
    {
        m_pBtnPlayPause->setStyleSheet("QPushButton {background-color: #EEEEEE; background-image: url(:/images/player_pause.png); "
                                       "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #FFFFFF;} "
                                       "QPushButton:pressed {background: #992F6F; background-image: url(:/images/player_pause.png); }");
        m_pCelexSensor->play();
        m_pSensorDataObserver->setPlayState(true);
        m_pPlayerTimer->start(1000);
    }
    else
    {
        m_pBtnPlayPause->setStyleSheet("QPushButton {background-color: #EEEEEE; background-image: url(:/images/player_play.png); "
                                       "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #FFFFFF;} "
                                       "QPushButton:pressed {background: #992F6F; background-image: url(:/images/player_play.png); }");
        m_pCelexSensor->pause();
        m_pSensorDataObserver->setPlayState(false);
        m_pPlayerTimer->stop();
    }
    m_bPaused = !m_bPaused;
}

void MainWindow::onBtnRotateLRReleased()
{
    if (m_pSensorDataObserver->getRotateType()%2 == 1)
    {
        m_pSensorDataObserver->setRotateType(-1);
        QPushButton* pButton = (QPushButton*)m_pButtonGroup->button(7);
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F;}");
    }
    else
    {
        m_pSensorDataObserver->setRotateType(1);
        QPushButton* pButton = (QPushButton*)m_pButtonGroup->button(7);
        pButton->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F; }");
    }
}

void MainWindow::onBtnRotateUDReleased()
{
    if (m_pSensorDataObserver->getRotateType() >= 2)
    {
        m_pSensorDataObserver->setRotateType(-2);
        QPushButton* pButton = (QPushButton*)m_pButtonGroup->button(8);
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F;}");
    }
    else
    {
        m_pSensorDataObserver->setRotateType(2);
        QPushButton* pButton = (QPushButton*)m_pButtonGroup->button(8);
        pButton->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F; }");
    }
}

void MainWindow::onBtnRepeatReleased()
{
	if (m_bRepeatPlay)
	{
        m_pBtnRepeat->setText("Repeat Off");
	}
	else
	{
        m_pBtnRepeat->setText("Repeat On");
	}
    m_bRepeatPlay = !m_bRepeatPlay;
}

void MainWindow::onBtnSaveReleased()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save a bin file", QCoreApplication::applicationDirPath(), "Bin Files (*.bin)");
    if (filePath.isEmpty())
        return;

    int value = m_pSliderPlayer->maxValue() - m_pSliderPlayer->minValue();

    int hour   = value / 3600;
    int minute = (value%3600)/60;
    int second = (value%3600)%60;

    m_pCelexSensor->saveSelectedBinFile(filePath.toStdString(), m_ulPlaybackStartPos, m_ulPlaybackEndPos, hour, minute, second);
}

void MainWindow::onBtnSaveBmpReleased()
{

    if (m_pSensorDataObserver->isSavingBmp())
    {
        m_pSensorDataObserver->setSaveBmp(false);
        m_pBtnSaveBmp->setText("Start Saving Bmp");
        m_pBtnSaveBmp->setStyleSheet("QPushButton {background-color: #EEEEEE; font: 18px Calibri; "
                                     "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #FFFFFF;} "
                                     "QPushButton:pressed {background: #992F6F; }");
    }
    else
    {
        m_pSensorDataObserver->setSaveBmp(true);
        m_pBtnSaveBmp->setText("Stop Saving Bmp");
        m_pBtnSaveBmp->setStyleSheet("QPushButton {background-color: #992F6F; color: yellow; font: 18px Calibri; "
                                     "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #FFFFFF;} "
                                     "QPushButton:pressed {background: #992F6F; }");
    }
}

void MainWindow::onSetCurrentTime()
{
    if (m_bPipeoutAllowed)
        return;

    m_timeCurrent = m_timeCurrent.addSecs(1);
    setSliderTimeText(m_pLabelCurrentTime, m_timeCurrent.hour(), m_timeCurrent.minute(), m_timeCurrent.second());
    int value = m_timeCurrent.hour()*3600 + m_timeCurrent.minute()*60 + m_timeCurrent.second();
    if (m_bRepeatPlay)
    {
        if (value > m_pSliderPlayer->maxValue())
        {
            m_pSensorDataObserver->clearFrames();
            m_pPlayerTimer->start(1000);
            m_pCelexSensor->setPlayBackOffset(m_ulPlaybackStartPos);
            //set current time
            int hour   = m_pSliderPlayer->minValue() / 3600;
            int minute = (m_pSliderPlayer->minValue()%3600)/60;
            int second = (m_pSliderPlayer->minValue()%3600)%60;
            m_timeCurrent.setHMS(hour, minute, second);
            setSliderTimeText(m_pLabelCurrentTime, hour, minute, second);
            if (!m_pCelexSensor->readPlayBackData())
            {
                m_pPlaybackTimer->start(PLAYBACK_TIMER);
            }
            m_pSliderPlayer->setValue(m_pSliderPlayer->minValue());
            return;
        }
    }
    else
    {
        if (value >= m_pSliderPlayer->maxValue())
        {
            stopPlayback();
            m_pSliderPlayer->setValue(value);
            return;
        }
    }
    m_pSliderPlayer->setValue(value);
}

void MainWindow::onSliderValueChanged(unsigned long value)
{
    int hour   = value / 3600;
    int minute = (value%3600)/60;
    int second = (value%3600)%60;
    m_pPlayerTimer->stop();

    m_timeCurrent.setHMS(hour, minute, second);
    setSliderTimeText(m_pLabelCurrentTime, hour, minute, second);
}

void MainWindow::onSliderPressed()
{
}

void MainWindow::onSliderReleased()
{
    m_pSensorDataObserver->clearFrames();

    m_pPlayerTimer->start(1000);

    long offset = ((float)m_pSliderPlayer->value()/(float)m_pSliderPlayer->maxRange()) * (float)m_pCelexSensor->getPlaybackFileSize();
    m_pCelexSensor->setPlayBackOffset(offset);
    cout << "MainWindow::onSliderReleased: offset = " << offset << endl;
    if (!m_pCelexSensor->readPlayBackData())
    {
        m_pPlaybackTimer->start(PLAYBACK_TIMER);
    }

    int hour   = m_pSliderPlayer->value() / 3600;
    int minute = (m_pSliderPlayer->value()%3600)/60;
    int second = (m_pSliderPlayer->value()%3600)%60;
    m_timeCurrent.setHMS(hour, minute, second);
    setSliderTimeText(m_pLabelCurrentTime, hour, minute, second);
}

void MainWindow::onMinValueChanged(unsigned long value)
{
    int hour   = value / 3600;
    int minute = (value%3600)/60;
    int second = (value%3600)%60;
    setSliderTimeText(m_pLabelStartTime, hour, minute, second);

    m_ulPlaybackStartPos = ((float)value/(float)m_pSliderPlayer->maxRange()) * (float)m_pCelexSensor->getPlaybackFileSize();
}

void MainWindow::onMaxValueChanged(unsigned long value)
{
    int hour   = value / 3600;
    int minute = (value%3600)/60;
    int second = (value%3600)%60;
    setSliderTimeText(m_pLabelEndTime, hour, minute, second);

    m_ulPlaybackEndPos = ((float)value/(float)m_pSliderPlayer->maxRange()) * (float)m_pCelexSensor->getPlaybackFileSize();
}

void MainWindow::onSensorModeChanged(QString mode)
{
    cout << __func__ << "  " << mode.toStdString() << endl;
    if ("SensorMode - FullPic" == mode)
    {
        if (FullPictureMode != m_pCelexSensor->getSensorMode())
        {
            m_pCelexSensor->setSensorMode(FullPictureMode);
        }
        setDisplayMode(FullPictureMode);
        disableSlider("Overlap", true);
    }
    else if ("SensorMode - Event" == mode)
    {
        if (EventMode != m_pCelexSensor->getSensorMode())
        {
            m_pCelexSensor->setSensorMode(EventMode);
        }
        setDisplayMode(EventMode);
        disableSlider("Overlap", false);
    }
    else if ("SensorMode - FullPic_Event" == mode)
    {
        if (FullPic_Event_Mode != m_pCelexSensor->getSensorMode())
        {
            m_pCelexSensor->setSensorMode(FullPic_Event_Mode);
        }
        setDisplayMode(FullPic_Event_Mode);
        disableSlider("Overlap", true);
        if (!m_bGlobalLengthSet)
        {
            m_pCelexSensor->setResetLength(200000);
            m_bGlobalLengthSet = true;
        }
    }
}

void MainWindow::onDisplayMode1Changed(int mode)
{
    m_uiDisplayMode1 = mode;
    m_pSensorDataObserver->setDisplayMode1(mode);
    if (mode > 5)
        m_pCelexSensor->enableMultiSlice(true);
    else
    {
        if (m_uiDisplayMode2 < 6)
            m_pCelexSensor->enableMultiSlice(false);
    }
}

void MainWindow::onDisplayMode2Changed(int mode)
{
    m_uiDisplayMode2 = mode;
    m_pSensorDataObserver->setDisplayMode2(mode);
    if (mode > 5)
        m_pCelexSensor->enableMultiSlice(true);
    else
    {
        if (m_uiDisplayMode1 < 6)
            m_pCelexSensor->enableMultiSlice(false);
    }
}

void MainWindow::onPipeoutDataTimer()
{
    if (!m_bPipeoutAllowed)
    {
        return;
    }
    if (m_pCelexSensor->isSdramFull())
    {
        m_pCelexSensor->resetDevice(CelexSensorDLL::SensorAndFPGA);
    }
    else
    {
        m_pCelexSensor->pipeOutFPGAData();
    }
    m_pPipeOutDataTimer->start(PIPEOUT_TIMER);
}

void MainWindow::onPlaybackTimer()
{
    if (!m_pCelexSensor->readPlayBackData())
    {
        m_pPlaybackTimer->start(PLAYBACK_TIMER);
    }
}

MainWindow::~MainWindow()
{
    if (m_pCelexSensor)
        delete m_pCelexSensor;
    delete ui;
}

void MainWindow::pauseThread(bool pause)
{
    m_pCelexSensor->pauseThread(pause);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    qDebug() << m_pComboBoxModeText1->x() << m_pComboBoxModeText2->x();
    m_pSensorDataObserver->setImagePos(m_pComboBoxModeText1->x()-10, m_pComboBoxModeText2->x()-10);
}

void MainWindow::disableSlider(QString objName, bool disable)
{
    for (int i = 0; i < this->children().size(); ++i)
    {
        HHSliderWidget* pWidget = (HHSliderWidget*)this->children().at(i);
        if (pWidget->objectName() == objName)
        {
            pWidget->setDisabled(disable);
            return;
        }
    }
}

void MainWindow::setDisplayMode(emSensorMode mode)
{
    if (mode == FullPictureMode)
    {
        m_pComboBoxModeText1->clear();
        m_pComboBoxModeText1->insertItem(0, "FullPicMode - Full Pic");
        m_pComboBoxModeText2->clear();

        m_pSensorDataObserver->setSensorMode(FullPictureMode);
    }
    else if (mode == EventMode)
    {
        m_pComboBoxModeText1->clear();
        m_pComboBoxModeText1->insertItem(0, "EventMode - Accumulated Pic");
        m_pComboBoxModeText1->insertItem(1, "EventMode - Binary Pic");
        m_pComboBoxModeText1->insertItem(2, "EventMode - Gray Pic");
        m_pComboBoxModeText1->insertItem(3, "EventMode - Superimposed Pic");
        m_pComboBoxModeText1->insertItem(4, "EventMode - Denoised Binary Pic");
        m_pComboBoxModeText1->insertItem(5, "EventMode - Denoised Gray Pic");
        m_pComboBoxModeText1->insertItem(6, "EventMode - MultiSlice Pic");
        m_pComboBoxModeText1->insertItem(7, "EventMode - MultiSlice Direction Pic");
        m_pComboBoxModeText1->insertItem(8, "EventMode - MultiSlice Speed Pic");
        m_pComboBoxModeText1->setCurrentText("EventMode - Accumulated Pic");

        m_pComboBoxModeText2->clear();
        m_pComboBoxModeText2->insertItem(0, "EventMode - Accumulated Pic");
        m_pComboBoxModeText2->insertItem(1, "EventMode - Binary Pic");
        m_pComboBoxModeText2->insertItem(2, "EventMode - Gray Pic");
        m_pComboBoxModeText2->insertItem(3, "EventMode - Superimposed Pic");
        m_pComboBoxModeText2->insertItem(4, "EventMode - Denoised Binary Pic");
        m_pComboBoxModeText2->insertItem(5, "EventMode - Denoised Gray Pic");
        m_pComboBoxModeText2->insertItem(6, "EventMode - MultiSlice Pic");
        m_pComboBoxModeText2->insertItem(7, "EventMode - MultiSlice Direction Pic");
        m_pComboBoxModeText2->insertItem(8, "EventMode - MultiSlice Speed Pic");
        m_pComboBoxModeText2->setCurrentText("EventMode - Binary Pic");
        //
        m_pSensorDataObserver->setSensorMode(EventMode);
    }
    else if (mode == FullPic_Event_Mode)
    {
        m_pComboBoxModeText1->clear();
        m_pComboBoxModeText1->insertItem(0, "FullPicEventMode - Full Pic");
        m_pComboBoxModeText1->insertItem(1, "FullPicEventMode - Binary Pic");
        m_pComboBoxModeText1->insertItem(2, "FullPicEventMode - Gray Pic");
        m_pComboBoxModeText1->insertItem(3, "FullPicEventMode - Superimposed Pic");
        m_pComboBoxModeText1->insertItem(4, "EventMode - Denoised Binary Pic");
        m_pComboBoxModeText1->insertItem(5, "EventMode - Denoised Gray Pic");
        m_pComboBoxModeText1->insertItem(6, "EventMode - MultiSlice Pic");
        m_pComboBoxModeText1->insertItem(7, "EventMode - MultiSlice Direction Pic");
        m_pComboBoxModeText1->insertItem(8, "EventMode - MultiSlice Speed Pic");
        m_pComboBoxModeText1->setCurrentText("FullPicEventMode - Full Pic");

        m_pComboBoxModeText2->clear();
        m_pComboBoxModeText2->insertItem(0, "FullPicEventMode - Full Pic");
        m_pComboBoxModeText2->insertItem(1, "FullPicEventMode - Binary Pic");
        m_pComboBoxModeText2->insertItem(2, "FullPicEventMode - Gray Pic");
        m_pComboBoxModeText2->insertItem(3, "FullPicEventMode - Superimposed Pic");
        m_pComboBoxModeText2->insertItem(4, "EventMode - Denoised Binary Pic");
        m_pComboBoxModeText2->insertItem(5, "EventMode - Denoised Gray Pic");
        m_pComboBoxModeText2->insertItem(6, "EventMode - MultiSlice Pic");
        m_pComboBoxModeText2->insertItem(7, "EventMode - MultiSlice Direction Pic");
        m_pComboBoxModeText2->insertItem(8, "EventMode - MultiSlice Speed Pic");
        m_pComboBoxModeText2->setCurrentText("FullPicEventMode - Binary Pic");

        m_pSensorDataObserver->setSensorMode(FullPic_Event_Mode);
    }
}
