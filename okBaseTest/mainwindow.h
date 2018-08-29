#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <stdint.h>
#include "celexsensordll.h"
#include "dataqueue.h"
#include "HHDetectThread.h"

#pragma execution_character_set("utf-8")

class QLabel;
class MainWindow;
class SensorDataObserver : public QWidget, public Observer
{
    Q_OBJECT
public:
    SensorDataObserver(SensorData* sensorData, QWidget *parent);
    ~SensorDataObserver();
    virtual void updateData(emDataType type); //overrides Observer operation
    void setPlayState(bool state);
    void setDisplayTime(int time); //msec
    int  getDisplayTime();
    void setRotateType(int type); // 1: left/right; 2: up/down
    int  getRotateType();
    void setWindow(MainWindow* pWindow);
    void clearFrames();
    void setPlayback(bool bPlayback);
    void setSensorMode(emSensorMode mode);
    void setSaveBmp(bool save);
    bool isSavingBmp();
    void setDisplayMode1(unsigned int mode);
    void setDisplayMode2(unsigned int mode);
    void setImagePos(int x1, int x2);

private:
    void saveBmps();
    void updateImage(unsigned char* pBuffer1, unsigned char* pBuffer2);
signals:
	void sgupdateImg(const QImage& pBuffer1, const QImage& pBuffer2);
	void sgupdateBuffer(const unsigned char* pBuffer1, const unsigned char* pBuffer2, const unsigned char* pBuffer3 = 0);

protected:
    void paintEvent(QPaintEvent *event);

protected slots:
    void onUpdateDisplay();
public slots:
	void onRectReady(QRect,int);

public:
    QImage         m_imageFullPic;
    QImage         m_imageEventPic;

    SensorData*    m_pSensorData;
    CirDataQueue*  m_pFullPicQueue;
    CirDataQueue*  m_pEventPicQueue;
    QTimer*        m_pPlayTimer;
    MainWindow*    m_pWindow;
    QLabel*        m_pLabelBg;
	QRect          m_rect;
	int            m_dete_name_index;
    unsigned char* m_pFrameBuffer1;
    unsigned char* m_pFrameBuffer2;

    emSensorMode   m_emSensorMode;

    long           m_lFrameCount;

    unsigned int   m_uiDisplayMode1;
    unsigned int   m_uiDisplayMode2;
    int            m_iDisplayFrameTime;
    int            m_iRotateType;
    int            m_iImage1X;
    int            m_iImage2X;
    int            m_iImageWidth;
    int            m_iImageHeight;

    bool           m_bRealtimeDisplay;
    bool           m_bSaveBmps;
    bool           m_bPlayback;
    bool           m_bCreatingFramePaused;
	HHDetectThread mDetectThread;
};

namespace Ui {
class MainWindow;
}

class DoubleSlider;
class QPushButton;
class QLabel;
class QButtonGroup;
class CelexSensorDLL;
class SensorDataObserver;
class HHSliderWidget;
class QComboBox;
class QHBoxLayout;
class QGridLayout;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void pauseThread(bool pause);

protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void onPipeoutDataTimer();
    void onPlaybackTimer();
    void onButtonClicked(int index);
    void onValueChanged(uint32_t value, HHSliderWidget* slider);
    void onBtnPlayReleased();
    void onBtnRotateLRReleased();
    void onBtnRotateUDReleased();
    void onBtnRepeatReleased();

    void onBtnSaveReleased();
    void onBtnSaveBmpReleased();

    void onSetCurrentTime();
    void onSliderValueChanged(unsigned long value);
    void onSliderPressed();
    void onSliderReleased();
    void onMinValueChanged(unsigned long value);
    void onMaxValueChanged(unsigned long value);

    void onSensorModeChanged(QString mode);
    void onDisplayMode1Changed(int mode);
    void onDisplayMode2Changed(int mode);

private:
    void generateFPN();
    void playback();
    void setRecording(bool bStart);
    void enableADC(bool enable);
    void createButtons(QHBoxLayout* layout);
    void createSliders(QGridLayout* layout);
    void createComboBoxStyle();
    void displayPlaybackBoard(bool display);
    void setEndTime();
    void setSliderTimeText(QLabel* pLabel, int hour, int minute, int second);
    void stopPlayback();
    void displayAdvancedSliders();
    void disableSlider(QString objName, bool disable);
    void setDisplayMode(emSensorMode mode);

private:
    Ui::MainWindow *ui;
    QTimer*             m_pPipeOutDataTimer;
    QTimer*             m_pPlaybackTimer;
    QTimer*             m_pPlayerTimer;
    CelexSensorDLL*     m_pCelexSensor;
    SensorDataObserver* m_pSensorDataObserver;
    QButtonGroup*       m_pButtonGroup;

    QWidget*            m_pAdvancedSliderBg;
    QComboBox*          m_pComboBoxModeText1;
    QComboBox*          m_pComboBoxModeText2;

    //playback control widgets
    QWidget*            m_pWidgetBg;
    QPushButton*        m_pBtnPlayPause;
    QLabel*             m_pLabelCurrentTime;
    QLabel*             m_pLabelStartTime;
    QLabel*             m_pLabelEndTime;
    DoubleSlider*       m_pSliderPlayer;

	QPushButton*        m_pBtnRepeat;
    QPushButton*        m_pBtnSaveFile;
    QPushButton*        m_pBtnSaveBmp;
    bool                m_bFirstShowPBBoard;
    QTime               m_timeCurrent;
    unsigned long       m_ulPlaybackStartPos;
    unsigned long       m_ulPlaybackEndPos;
    unsigned long       m_ulMaxRange;
    int                 m_iCreateFrameTime;
    unsigned int        m_uiDisplayMode1;
    unsigned int        m_uiDisplayMode2;
    //
    bool                m_bPipeoutAllowed;
    bool                m_bUseEnglishName;
    bool                m_bRecording;
    bool                m_bADCEnabled;
    bool                m_bPaused;
    bool                m_bRepeatPlay;
    bool                m_bGlobalLengthSet;
};

#endif // MAINWINDOW_H
