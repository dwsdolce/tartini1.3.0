/********************************************************************************
** Form generated from reading UI file 'settingsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *vboxLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *vboxLayout1;
    QGroupBox *quickAnalysisOptionsGroupBox;
    QVBoxLayout *vboxLayout2;
    QComboBox *noteRangeChoice;
    QLabel *commentTextLabel;
    QGroupBox *tempFilesGroupBox;
    QVBoxLayout *vboxLayout3;
    QHBoxLayout *hboxLayout;
    QLabel *tempFilesFolderLabel;
    QLineEdit *tempFilesFolder;
    QPushButton *chooseTempFolderButton;
    QHBoxLayout *hboxLayout1;
    QLabel *fileGeneratingStringLabel;
    QLineEdit *filenameGeneratingString;
    QSpinBox *fileGeneratingNumber;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout2;
    QLabel *fileGeneratingNumberLabel;
    QSpinBox *fileNumberOfDigits;
    QLabel *filenameExample;
    QSpacerItem *spacerItem1;
    QSpacerItem *spacerItem2;
    QWidget *tab_2;
    QVBoxLayout *vboxLayout4;
    QGroupBox *generalGroupBox;
    QVBoxLayout *vboxLayout5;
    QHBoxLayout *hboxLayout3;
    QLabel *textLabel1;
    QComboBox *soundInput;
    QHBoxLayout *hboxLayout4;
    QLabel *textLabel2;
    QComboBox *soundOutput;
    QHBoxLayout *hboxLayout5;
    QLabel *numBuffersLabel;
    QSpinBox *numberOfBuffers;
    QSpacerItem *spacerItem3;
    QGroupBox *recordingGroupBox;
    QVBoxLayout *vboxLayout6;
    QHBoxLayout *hboxLayout6;
    QLabel *channelsLabel;
    QComboBox *numberOfChannels;
    QSpacerItem *spacerItem4;
    QLabel *sampleRateLabel;
    QComboBox *sampleRate;
    QSpacerItem *spacerItem5;
    QLabel *bitsPerSampleLabel;
    QComboBox *bitsPerSample;
    QCheckBox *muteOutput;
    QSpacerItem *spacerItem6;
    QSpacerItem *spacerItem7;
    QWidget *tab_3;
    QVBoxLayout *vboxLayout7;
    QGroupBox *analysisGroupBox;
    QVBoxLayout *vboxLayout8;
    QGridLayout *gridLayout;
    QCheckBox *doingHarmonicAnalysis;
    QCheckBox *doingFreqAnalysis;
    QCheckBox *doingAutoNoiseFloor;
    QCheckBox *doingDetailedPitch;
    QCheckBox *doingEqualLoudness;
    QHBoxLayout *hboxLayout7;
    QLabel *analysisTypeLabel;
    QComboBox *analysisType;
    QSpacerItem *spacerItem8;
    QLabel *thresholdTextLabel;
    QSpinBox *thresholdValue;
    QSpacerItem *spacerItem9;
    QGroupBox *bufferSizeGroupBox;
    QVBoxLayout *vboxLayout9;
    QHBoxLayout *hboxLayout8;
    QLabel *bufferSizeTextLabel;
    QSpinBox *bufferSizeValue;
    QComboBox *bufferSizeUnit;
    QSpacerItem *spacerItem10;
    QCheckBox *bufferSizeRound;
    QSpacerItem *spacerItem11;
    QGroupBox *stepSizeGroupBox;
    QVBoxLayout *vboxLayout10;
    QHBoxLayout *hboxLayout9;
    QLabel *stepSizeTextLabel;
    QSpinBox *stepSizeValue;
    QComboBox *stepSizeUnit;
    QSpacerItem *spacerItem12;
    QCheckBox *stepSizeRound;
    QSpacerItem *spacerItem13;
    QSpacerItem *spacerItem14;
    QWidget *tab_4;
    QVBoxLayout *vboxLayout11;
    QGroupBox *colorsGroupBox;
    QGridLayout *gridLayout1;
    QFrame *shading2Color;
    QFrame *shading1Color;
    QFrame *theBackgroundColor;
    QSpacerItem *spacerItem15;
    QSpacerItem *spacerItem16;
    QSpacerItem *spacerItem17;
    QPushButton *backgroundButton;
    QPushButton *shading1Button;
    QPushButton *shading2Button;
    QSpacerItem *spacerItem18;
    QGroupBox *screenRefreshGroup;
    QGridLayout *gridLayout2;
    QSpinBox *slowUpdateSpeed;
    QSpinBox *fastUpdateSpeed;
    QLabel *fastUpdateLabel;
    QLabel *milliSecondsLabel1;
    QSpacerItem *spacerItem19;
    QLabel *slowUpdateLabel;
    QLabel *milliSecondsLabel2;
    QSpacerItem *spacerItem20;
    QSpacerItem *spacerItem21;
    QGroupBox *windowOptionsGroupBox;
    QHBoxLayout *hboxLayout10;
    QCheckBox *useTopLevelWidgets;
    QSpacerItem *spacerItem22;
    QSpacerItem *spacerItem23;
    QWidget *tab_5;
    QVBoxLayout *vboxLayout12;
    QGroupBox *AdvancedGroupBox;
    QVBoxLayout *vboxLayout13;
    QCheckBox *showMeanVarianceBars;
    QComboBox *savingMode;
    QCheckBox *mouseWheelZooms;
    QGroupBox *VibratoGroupBox;
    QVBoxLayout *vboxLayout14;
    QCheckBox *vibratoSineStyle;
    QSpacerItem *spacerItem24;
    QHBoxLayout *hboxLayout11;
    QPushButton *resetDefaultsButton;
    QSpacerItem *spacerItem25;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName(QString::fromUtf8("SettingsDialog"));
        SettingsDialog->resize(570, 501);
        vboxLayout = new QVBoxLayout(SettingsDialog);
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        tabWidget = new QTabWidget(SettingsDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        vboxLayout1 = new QVBoxLayout(tab);
#ifndef Q_OS_MAC
        vboxLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        quickAnalysisOptionsGroupBox = new QGroupBox(tab);
        quickAnalysisOptionsGroupBox->setObjectName(QString::fromUtf8("quickAnalysisOptionsGroupBox"));
        vboxLayout2 = new QVBoxLayout(quickAnalysisOptionsGroupBox);
#ifndef Q_OS_MAC
        vboxLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        noteRangeChoice = new QComboBox(quickAnalysisOptionsGroupBox);
        noteRangeChoice->addItem(QString());
        noteRangeChoice->addItem(QString());
        noteRangeChoice->addItem(QString());
        noteRangeChoice->addItem(QString());
        noteRangeChoice->addItem(QString());
        noteRangeChoice->setObjectName(QString::fromUtf8("noteRangeChoice"));

        vboxLayout2->addWidget(noteRangeChoice);

        commentTextLabel = new QLabel(quickAnalysisOptionsGroupBox);
        commentTextLabel->setObjectName(QString::fromUtf8("commentTextLabel"));

        vboxLayout2->addWidget(commentTextLabel);


        vboxLayout1->addWidget(quickAnalysisOptionsGroupBox);

        tempFilesGroupBox = new QGroupBox(tab);
        tempFilesGroupBox->setObjectName(QString::fromUtf8("tempFilesGroupBox"));
        vboxLayout3 = new QVBoxLayout(tempFilesGroupBox);
#ifndef Q_OS_MAC
        vboxLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout3->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        tempFilesFolderLabel = new QLabel(tempFilesGroupBox);
        tempFilesFolderLabel->setObjectName(QString::fromUtf8("tempFilesFolderLabel"));

        hboxLayout->addWidget(tempFilesFolderLabel);

        tempFilesFolder = new QLineEdit(tempFilesGroupBox);
        tempFilesFolder->setObjectName(QString::fromUtf8("tempFilesFolder"));

        hboxLayout->addWidget(tempFilesFolder);

        chooseTempFolderButton = new QPushButton(tempFilesGroupBox);
        chooseTempFolderButton->setObjectName(QString::fromUtf8("chooseTempFolderButton"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(chooseTempFolderButton->sizePolicy().hasHeightForWidth());
        chooseTempFolderButton->setSizePolicy(sizePolicy);
        chooseTempFolderButton->setMaximumSize(QSize(40, 31));

        hboxLayout->addWidget(chooseTempFolderButton);


        vboxLayout3->addLayout(hboxLayout);

        hboxLayout1 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout1->setSpacing(6);
#endif
        hboxLayout1->setContentsMargins(0, 0, 0, 0);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        fileGeneratingStringLabel = new QLabel(tempFilesGroupBox);
        fileGeneratingStringLabel->setObjectName(QString::fromUtf8("fileGeneratingStringLabel"));

        hboxLayout1->addWidget(fileGeneratingStringLabel);

        filenameGeneratingString = new QLineEdit(tempFilesGroupBox);
        filenameGeneratingString->setObjectName(QString::fromUtf8("filenameGeneratingString"));

        hboxLayout1->addWidget(filenameGeneratingString);

        fileGeneratingNumber = new QSpinBox(tempFilesGroupBox);
        fileGeneratingNumber->setObjectName(QString::fromUtf8("fileGeneratingNumber"));
        fileGeneratingNumber->setMaximum(99999);

        hboxLayout1->addWidget(fileGeneratingNumber);

        spacerItem = new QSpacerItem(71, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacerItem);


        vboxLayout3->addLayout(hboxLayout1);

        hboxLayout2 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout2->setSpacing(6);
#endif
        hboxLayout2->setContentsMargins(0, 0, 0, 0);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        fileGeneratingNumberLabel = new QLabel(tempFilesGroupBox);
        fileGeneratingNumberLabel->setObjectName(QString::fromUtf8("fileGeneratingNumberLabel"));

        hboxLayout2->addWidget(fileGeneratingNumberLabel);

        fileNumberOfDigits = new QSpinBox(tempFilesGroupBox);
        fileNumberOfDigits->setObjectName(QString::fromUtf8("fileNumberOfDigits"));
        fileNumberOfDigits->setMaximum(5);

        hboxLayout2->addWidget(fileNumberOfDigits);

        filenameExample = new QLabel(tempFilesGroupBox);
        filenameExample->setObjectName(QString::fromUtf8("filenameExample"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(filenameExample->sizePolicy().hasHeightForWidth());
        filenameExample->setSizePolicy(sizePolicy1);

        hboxLayout2->addWidget(filenameExample);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout2->addItem(spacerItem1);


        vboxLayout3->addLayout(hboxLayout2);


        vboxLayout1->addWidget(tempFilesGroupBox);

        spacerItem2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout1->addItem(spacerItem2);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        vboxLayout4 = new QVBoxLayout(tab_2);
#ifndef Q_OS_MAC
        vboxLayout4->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout4->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
        generalGroupBox = new QGroupBox(tab_2);
        generalGroupBox->setObjectName(QString::fromUtf8("generalGroupBox"));
        vboxLayout5 = new QVBoxLayout(generalGroupBox);
#ifndef Q_OS_MAC
        vboxLayout5->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout5->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout5->setObjectName(QString::fromUtf8("vboxLayout5"));
        hboxLayout3 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout3->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        textLabel1 = new QLabel(generalGroupBox);
        textLabel1->setObjectName(QString::fromUtf8("textLabel1"));

        hboxLayout3->addWidget(textLabel1);

        soundInput = new QComboBox(generalGroupBox);
        soundInput->setObjectName(QString::fromUtf8("soundInput"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(soundInput->sizePolicy().hasHeightForWidth());
        soundInput->setSizePolicy(sizePolicy2);

        hboxLayout3->addWidget(soundInput);


        vboxLayout5->addLayout(hboxLayout3);

        hboxLayout4 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout4->setSpacing(6);
#endif
        hboxLayout4->setContentsMargins(0, 0, 0, 0);
        hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
        textLabel2 = new QLabel(generalGroupBox);
        textLabel2->setObjectName(QString::fromUtf8("textLabel2"));

        hboxLayout4->addWidget(textLabel2);

        soundOutput = new QComboBox(generalGroupBox);
        soundOutput->setObjectName(QString::fromUtf8("soundOutput"));
        sizePolicy2.setHeightForWidth(soundOutput->sizePolicy().hasHeightForWidth());
        soundOutput->setSizePolicy(sizePolicy2);

        hboxLayout4->addWidget(soundOutput);


        vboxLayout5->addLayout(hboxLayout4);

        hboxLayout5 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout5->setSpacing(6);
#endif
        hboxLayout5->setContentsMargins(0, 0, 0, 0);
        hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
        numBuffersLabel = new QLabel(generalGroupBox);
        numBuffersLabel->setObjectName(QString::fromUtf8("numBuffersLabel"));

        hboxLayout5->addWidget(numBuffersLabel);

        numberOfBuffers = new QSpinBox(generalGroupBox);
        numberOfBuffers->setObjectName(QString::fromUtf8("numberOfBuffers"));
        numberOfBuffers->setMinimumSize(QSize(0, 30));
        numberOfBuffers->setMaximum(63);
        numberOfBuffers->setMinimum(1);
        numberOfBuffers->setValue(4);

        hboxLayout5->addWidget(numberOfBuffers);

        spacerItem3 = new QSpacerItem(181, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout5->addItem(spacerItem3);


        vboxLayout5->addLayout(hboxLayout5);


        vboxLayout4->addWidget(generalGroupBox);

        recordingGroupBox = new QGroupBox(tab_2);
        recordingGroupBox->setObjectName(QString::fromUtf8("recordingGroupBox"));
        vboxLayout6 = new QVBoxLayout(recordingGroupBox);
#ifndef Q_OS_MAC
        vboxLayout6->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout6->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout6->setObjectName(QString::fromUtf8("vboxLayout6"));
        hboxLayout6 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout6->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout6->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
        channelsLabel = new QLabel(recordingGroupBox);
        channelsLabel->setObjectName(QString::fromUtf8("channelsLabel"));

        hboxLayout6->addWidget(channelsLabel);

        numberOfChannels = new QComboBox(recordingGroupBox);
        numberOfChannels->addItem(QString());
        numberOfChannels->addItem(QString());
        numberOfChannels->setObjectName(QString::fromUtf8("numberOfChannels"));

        hboxLayout6->addWidget(numberOfChannels);

        spacerItem4 = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout6->addItem(spacerItem4);

        sampleRateLabel = new QLabel(recordingGroupBox);
        sampleRateLabel->setObjectName(QString::fromUtf8("sampleRateLabel"));

        hboxLayout6->addWidget(sampleRateLabel);

        sampleRate = new QComboBox(recordingGroupBox);
        sampleRate->addItem(QString());
        sampleRate->addItem(QString());
        sampleRate->addItem(QString());
        sampleRate->addItem(QString());
        sampleRate->addItem(QString());
        sampleRate->addItem(QString());
        sampleRate->setObjectName(QString::fromUtf8("sampleRate"));

        hboxLayout6->addWidget(sampleRate);

        spacerItem5 = new QSpacerItem(16, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout6->addItem(spacerItem5);

        bitsPerSampleLabel = new QLabel(recordingGroupBox);
        bitsPerSampleLabel->setObjectName(QString::fromUtf8("bitsPerSampleLabel"));

        hboxLayout6->addWidget(bitsPerSampleLabel);

        bitsPerSample = new QComboBox(recordingGroupBox);
        bitsPerSample->addItem(QString());
        bitsPerSample->addItem(QString());
        bitsPerSample->setObjectName(QString::fromUtf8("bitsPerSample"));

        hboxLayout6->addWidget(bitsPerSample);


        vboxLayout6->addLayout(hboxLayout6);

        muteOutput = new QCheckBox(recordingGroupBox);
        muteOutput->setObjectName(QString::fromUtf8("muteOutput"));

        vboxLayout6->addWidget(muteOutput);

        spacerItem6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout6->addItem(spacerItem6);


        vboxLayout4->addWidget(recordingGroupBox);

        spacerItem7 = new QSpacerItem(20, 271, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout4->addItem(spacerItem7);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        vboxLayout7 = new QVBoxLayout(tab_3);
#ifndef Q_OS_MAC
        vboxLayout7->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout7->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout7->setObjectName(QString::fromUtf8("vboxLayout7"));
        analysisGroupBox = new QGroupBox(tab_3);
        analysisGroupBox->setObjectName(QString::fromUtf8("analysisGroupBox"));
        vboxLayout8 = new QVBoxLayout(analysisGroupBox);
#ifndef Q_OS_MAC
        vboxLayout8->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout8->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout8->setObjectName(QString::fromUtf8("vboxLayout8"));
        gridLayout = new QGridLayout();
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(0, 0, 0, 0);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        doingHarmonicAnalysis = new QCheckBox(analysisGroupBox);
        doingHarmonicAnalysis->setObjectName(QString::fromUtf8("doingHarmonicAnalysis"));

        gridLayout->addWidget(doingHarmonicAnalysis, 1, 0, 1, 1);

        doingFreqAnalysis = new QCheckBox(analysisGroupBox);
        doingFreqAnalysis->setObjectName(QString::fromUtf8("doingFreqAnalysis"));

        gridLayout->addWidget(doingFreqAnalysis, 0, 0, 1, 1);

        doingAutoNoiseFloor = new QCheckBox(analysisGroupBox);
        doingAutoNoiseFloor->setObjectName(QString::fromUtf8("doingAutoNoiseFloor"));

        gridLayout->addWidget(doingAutoNoiseFloor, 0, 1, 1, 1);

        doingDetailedPitch = new QCheckBox(analysisGroupBox);
        doingDetailedPitch->setObjectName(QString::fromUtf8("doingDetailedPitch"));
        doingDetailedPitch->setCursor(QCursor(static_cast<Qt::CursorShape>(0)));

        gridLayout->addWidget(doingDetailedPitch, 1, 1, 1, 1);

        doingEqualLoudness = new QCheckBox(analysisGroupBox);
        doingEqualLoudness->setObjectName(QString::fromUtf8("doingEqualLoudness"));

        gridLayout->addWidget(doingEqualLoudness, 2, 0, 1, 1);


        vboxLayout8->addLayout(gridLayout);

        hboxLayout7 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout7->setSpacing(6);
#endif
        hboxLayout7->setContentsMargins(0, 0, 0, 0);
        hboxLayout7->setObjectName(QString::fromUtf8("hboxLayout7"));
        analysisTypeLabel = new QLabel(analysisGroupBox);
        analysisTypeLabel->setObjectName(QString::fromUtf8("analysisTypeLabel"));

        hboxLayout7->addWidget(analysisTypeLabel);

        analysisType = new QComboBox(analysisGroupBox);
        analysisType->addItem(QString());
        analysisType->addItem(QString());
        analysisType->addItem(QString());
        analysisType->setObjectName(QString::fromUtf8("analysisType"));

        hboxLayout7->addWidget(analysisType);

        spacerItem8 = new QSpacerItem(271, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout7->addItem(spacerItem8);

        thresholdTextLabel = new QLabel(analysisGroupBox);
        thresholdTextLabel->setObjectName(QString::fromUtf8("thresholdTextLabel"));

        hboxLayout7->addWidget(thresholdTextLabel);

        thresholdValue = new QSpinBox(analysisGroupBox);
        thresholdValue->setObjectName(QString::fromUtf8("thresholdValue"));
        thresholdValue->setMaximum(100);
        thresholdValue->setMinimum(80);
        thresholdValue->setSingleStep(1);

        hboxLayout7->addWidget(thresholdValue);

        spacerItem9 = new QSpacerItem(221, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout7->addItem(spacerItem9);


        vboxLayout8->addLayout(hboxLayout7);


        vboxLayout7->addWidget(analysisGroupBox);

        bufferSizeGroupBox = new QGroupBox(tab_3);
        bufferSizeGroupBox->setObjectName(QString::fromUtf8("bufferSizeGroupBox"));
        vboxLayout9 = new QVBoxLayout(bufferSizeGroupBox);
#ifndef Q_OS_MAC
        vboxLayout9->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout9->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout9->setObjectName(QString::fromUtf8("vboxLayout9"));
        hboxLayout8 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout8->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout8->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout8->setObjectName(QString::fromUtf8("hboxLayout8"));
        bufferSizeTextLabel = new QLabel(bufferSizeGroupBox);
        bufferSizeTextLabel->setObjectName(QString::fromUtf8("bufferSizeTextLabel"));

        hboxLayout8->addWidget(bufferSizeTextLabel);

        bufferSizeValue = new QSpinBox(bufferSizeGroupBox);
        bufferSizeValue->setObjectName(QString::fromUtf8("bufferSizeValue"));
        bufferSizeValue->setMaximum(99999);
        bufferSizeValue->setMinimum(1);
        bufferSizeValue->setValue(50);

        hboxLayout8->addWidget(bufferSizeValue);

        bufferSizeUnit = new QComboBox(bufferSizeGroupBox);
        bufferSizeUnit->addItem(QString());
        bufferSizeUnit->addItem(QString());
        bufferSizeUnit->setObjectName(QString::fromUtf8("bufferSizeUnit"));

        hboxLayout8->addWidget(bufferSizeUnit);

        spacerItem10 = new QSpacerItem(180, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout8->addItem(spacerItem10);


        vboxLayout9->addLayout(hboxLayout8);

        bufferSizeRound = new QCheckBox(bufferSizeGroupBox);
        bufferSizeRound->setObjectName(QString::fromUtf8("bufferSizeRound"));
        bufferSizeRound->setChecked(true);

        vboxLayout9->addWidget(bufferSizeRound);

        spacerItem11 = new QSpacerItem(20, 16, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout9->addItem(spacerItem11);


        vboxLayout7->addWidget(bufferSizeGroupBox);

        stepSizeGroupBox = new QGroupBox(tab_3);
        stepSizeGroupBox->setObjectName(QString::fromUtf8("stepSizeGroupBox"));
        vboxLayout10 = new QVBoxLayout(stepSizeGroupBox);
#ifndef Q_OS_MAC
        vboxLayout10->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout10->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout10->setObjectName(QString::fromUtf8("vboxLayout10"));
        hboxLayout9 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout9->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout9->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout9->setObjectName(QString::fromUtf8("hboxLayout9"));
        stepSizeTextLabel = new QLabel(stepSizeGroupBox);
        stepSizeTextLabel->setObjectName(QString::fromUtf8("stepSizeTextLabel"));

        hboxLayout9->addWidget(stepSizeTextLabel);

        stepSizeValue = new QSpinBox(stepSizeGroupBox);
        stepSizeValue->setObjectName(QString::fromUtf8("stepSizeValue"));
        stepSizeValue->setMaximum(99999);
        stepSizeValue->setMinimum(1);
        stepSizeValue->setValue(12);

        hboxLayout9->addWidget(stepSizeValue);

        stepSizeUnit = new QComboBox(stepSizeGroupBox);
        stepSizeUnit->addItem(QString());
        stepSizeUnit->addItem(QString());
        stepSizeUnit->setObjectName(QString::fromUtf8("stepSizeUnit"));

        hboxLayout9->addWidget(stepSizeUnit);

        spacerItem12 = new QSpacerItem(210, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout9->addItem(spacerItem12);


        vboxLayout10->addLayout(hboxLayout9);

        stepSizeRound = new QCheckBox(stepSizeGroupBox);
        stepSizeRound->setObjectName(QString::fromUtf8("stepSizeRound"));
        stepSizeRound->setChecked(true);

        vboxLayout10->addWidget(stepSizeRound);

        spacerItem13 = new QSpacerItem(20, 21, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout10->addItem(spacerItem13);


        vboxLayout7->addWidget(stepSizeGroupBox);

        spacerItem14 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout7->addItem(spacerItem14);

        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        vboxLayout11 = new QVBoxLayout(tab_4);
#ifndef Q_OS_MAC
        vboxLayout11->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout11->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout11->setObjectName(QString::fromUtf8("vboxLayout11"));
        colorsGroupBox = new QGroupBox(tab_4);
        colorsGroupBox->setObjectName(QString::fromUtf8("colorsGroupBox"));
        gridLayout1 = new QGridLayout(colorsGroupBox);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        shading2Color = new QFrame(colorsGroupBox);
        shading2Color->setObjectName(QString::fromUtf8("shading2Color"));
        shading2Color->setMinimumSize(QSize(120, 16));
        shading2Color->setAutoFillBackground(true);
        shading2Color->setFrameShape(QFrame::StyledPanel);
        shading2Color->setFrameShadow(QFrame::Sunken);

        gridLayout1->addWidget(shading2Color, 2, 1, 1, 1);

        shading1Color = new QFrame(colorsGroupBox);
        shading1Color->setObjectName(QString::fromUtf8("shading1Color"));
        shading1Color->setMinimumSize(QSize(120, 16));
        shading1Color->setAutoFillBackground(true);
        shading1Color->setFrameShape(QFrame::StyledPanel);
        shading1Color->setFrameShadow(QFrame::Sunken);

        gridLayout1->addWidget(shading1Color, 1, 1, 1, 1);

        theBackgroundColor = new QFrame(colorsGroupBox);
        theBackgroundColor->setObjectName(QString::fromUtf8("theBackgroundColor"));
        theBackgroundColor->setMinimumSize(QSize(120, 16));
        theBackgroundColor->setAutoFillBackground(true);
        theBackgroundColor->setFrameShape(QFrame::StyledPanel);
        theBackgroundColor->setFrameShadow(QFrame::Sunken);

        gridLayout1->addWidget(theBackgroundColor, 0, 1, 1, 1);

        spacerItem15 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout1->addItem(spacerItem15, 2, 2, 1, 1);

        spacerItem16 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout1->addItem(spacerItem16, 1, 2, 1, 1);

        spacerItem17 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout1->addItem(spacerItem17, 0, 2, 1, 1);

        backgroundButton = new QPushButton(colorsGroupBox);
        backgroundButton->setObjectName(QString::fromUtf8("backgroundButton"));

        gridLayout1->addWidget(backgroundButton, 0, 0, 1, 1);

        shading1Button = new QPushButton(colorsGroupBox);
        shading1Button->setObjectName(QString::fromUtf8("shading1Button"));

        gridLayout1->addWidget(shading1Button, 1, 0, 1, 1);

        shading2Button = new QPushButton(colorsGroupBox);
        shading2Button->setObjectName(QString::fromUtf8("shading2Button"));

        gridLayout1->addWidget(shading2Button, 2, 0, 1, 1);

        spacerItem18 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout1->addItem(spacerItem18, 3, 2, 1, 1);


        vboxLayout11->addWidget(colorsGroupBox);

        screenRefreshGroup = new QGroupBox(tab_4);
        screenRefreshGroup->setObjectName(QString::fromUtf8("screenRefreshGroup"));
        gridLayout2 = new QGridLayout(screenRefreshGroup);
#ifndef Q_OS_MAC
        gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        slowUpdateSpeed = new QSpinBox(screenRefreshGroup);
        slowUpdateSpeed->setObjectName(QString::fromUtf8("slowUpdateSpeed"));
        slowUpdateSpeed->setMaximum(99999);

        gridLayout2->addWidget(slowUpdateSpeed, 1, 1, 1, 1);

        fastUpdateSpeed = new QSpinBox(screenRefreshGroup);
        fastUpdateSpeed->setObjectName(QString::fromUtf8("fastUpdateSpeed"));
        fastUpdateSpeed->setMaximum(99999);

        gridLayout2->addWidget(fastUpdateSpeed, 0, 1, 1, 1);

        fastUpdateLabel = new QLabel(screenRefreshGroup);
        fastUpdateLabel->setObjectName(QString::fromUtf8("fastUpdateLabel"));

        gridLayout2->addWidget(fastUpdateLabel, 0, 0, 1, 1);

        milliSecondsLabel1 = new QLabel(screenRefreshGroup);
        milliSecondsLabel1->setObjectName(QString::fromUtf8("milliSecondsLabel1"));

        gridLayout2->addWidget(milliSecondsLabel1, 0, 2, 1, 1);

        spacerItem19 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout2->addItem(spacerItem19, 0, 3, 1, 1);

        slowUpdateLabel = new QLabel(screenRefreshGroup);
        slowUpdateLabel->setObjectName(QString::fromUtf8("slowUpdateLabel"));

        gridLayout2->addWidget(slowUpdateLabel, 1, 0, 1, 1);

        milliSecondsLabel2 = new QLabel(screenRefreshGroup);
        milliSecondsLabel2->setObjectName(QString::fromUtf8("milliSecondsLabel2"));

        gridLayout2->addWidget(milliSecondsLabel2, 1, 2, 1, 1);

        spacerItem20 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout2->addItem(spacerItem20, 1, 3, 1, 1);

        spacerItem21 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout2->addItem(spacerItem21, 2, 1, 1, 1);


        vboxLayout11->addWidget(screenRefreshGroup);

        windowOptionsGroupBox = new QGroupBox(tab_4);
        windowOptionsGroupBox->setObjectName(QString::fromUtf8("windowOptionsGroupBox"));
        hboxLayout10 = new QHBoxLayout(windowOptionsGroupBox);
#ifndef Q_OS_MAC
        hboxLayout10->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout10->setContentsMargins(9, 9, 9, 9);
#endif
        hboxLayout10->setObjectName(QString::fromUtf8("hboxLayout10"));
        useTopLevelWidgets = new QCheckBox(windowOptionsGroupBox);
        useTopLevelWidgets->setObjectName(QString::fromUtf8("useTopLevelWidgets"));

        hboxLayout10->addWidget(useTopLevelWidgets);

        spacerItem22 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout10->addItem(spacerItem22);


        vboxLayout11->addWidget(windowOptionsGroupBox);

        spacerItem23 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout11->addItem(spacerItem23);

        tabWidget->addTab(tab_4, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        vboxLayout12 = new QVBoxLayout(tab_5);
#ifndef Q_OS_MAC
        vboxLayout12->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout12->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout12->setObjectName(QString::fromUtf8("vboxLayout12"));
        AdvancedGroupBox = new QGroupBox(tab_5);
        AdvancedGroupBox->setObjectName(QString::fromUtf8("AdvancedGroupBox"));
        vboxLayout13 = new QVBoxLayout(AdvancedGroupBox);
#ifndef Q_OS_MAC
        vboxLayout13->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout13->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout13->setObjectName(QString::fromUtf8("vboxLayout13"));
        showMeanVarianceBars = new QCheckBox(AdvancedGroupBox);
        showMeanVarianceBars->setObjectName(QString::fromUtf8("showMeanVarianceBars"));

        vboxLayout13->addWidget(showMeanVarianceBars);

        savingMode = new QComboBox(AdvancedGroupBox);
        savingMode->addItem(QString());
        savingMode->addItem(QString());
        savingMode->addItem(QString());
        savingMode->setObjectName(QString::fromUtf8("savingMode"));

        vboxLayout13->addWidget(savingMode);

        mouseWheelZooms = new QCheckBox(AdvancedGroupBox);
        mouseWheelZooms->setObjectName(QString::fromUtf8("mouseWheelZooms"));

        vboxLayout13->addWidget(mouseWheelZooms);


        vboxLayout12->addWidget(AdvancedGroupBox);

        VibratoGroupBox = new QGroupBox(tab_5);
        VibratoGroupBox->setObjectName(QString::fromUtf8("VibratoGroupBox"));
        vboxLayout14 = new QVBoxLayout(VibratoGroupBox);
#ifndef Q_OS_MAC
        vboxLayout14->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout14->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout14->setObjectName(QString::fromUtf8("vboxLayout14"));
        vibratoSineStyle = new QCheckBox(VibratoGroupBox);
        vibratoSineStyle->setObjectName(QString::fromUtf8("vibratoSineStyle"));

        vboxLayout14->addWidget(vibratoSineStyle);


        vboxLayout12->addWidget(VibratoGroupBox);

        spacerItem24 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout12->addItem(spacerItem24);

        tabWidget->addTab(tab_5, QString());

        vboxLayout->addWidget(tabWidget);

        hboxLayout11 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout11->setSpacing(6);
#endif
        hboxLayout11->setContentsMargins(0, 0, 0, 0);
        hboxLayout11->setObjectName(QString::fromUtf8("hboxLayout11"));
        resetDefaultsButton = new QPushButton(SettingsDialog);
        resetDefaultsButton->setObjectName(QString::fromUtf8("resetDefaultsButton"));

        hboxLayout11->addWidget(resetDefaultsButton);

        spacerItem25 = new QSpacerItem(371, 38, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout11->addItem(spacerItem25);

        okButton = new QPushButton(SettingsDialog);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout11->addWidget(okButton);

        cancelButton = new QPushButton(SettingsDialog);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout11->addWidget(cancelButton);


        vboxLayout->addLayout(hboxLayout11);

#if QT_CONFIG(shortcut)
        tempFilesFolderLabel->setBuddy(tempFilesFolder);
        fileGeneratingStringLabel->setBuddy(filenameGeneratingString);
        fileGeneratingNumberLabel->setBuddy(numberOfChannels);
        textLabel1->setBuddy(soundInput);
        textLabel2->setBuddy(soundOutput);
        channelsLabel->setBuddy(numberOfChannels);
        sampleRateLabel->setBuddy(sampleRate);
        bitsPerSampleLabel->setBuddy(bitsPerSample);
        bufferSizeTextLabel->setBuddy(bufferSizeValue);
        stepSizeTextLabel->setBuddy(stepSizeValue);
#endif // QT_CONFIG(shortcut)

        retranslateUi(SettingsDialog);

        tabWidget->setCurrentIndex(0);
        savingMode->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QCoreApplication::translate("SettingsDialog", "Tartini Preferences", nullptr));
        quickAnalysisOptionsGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Quick Analysis Options - Note Range", nullptr));
        noteRangeChoice->setItemText(0, QCoreApplication::translate("SettingsDialog", "Custom - Choose settings from the Analysis tab", nullptr));
        noteRangeChoice->setItemText(1, QCoreApplication::translate("SettingsDialog", "Notes F0 and higher - Double Bass, Bass Guitar ...", nullptr));
        noteRangeChoice->setItemText(2, QCoreApplication::translate("SettingsDialog", "Notes F1 and higher - Cello, Guitar, Bass Clarinet, General sounds ...", nullptr));
        noteRangeChoice->setItemText(3, QCoreApplication::translate("SettingsDialog", "Notes F2 and higher - Violin, Viola, Clarinet, Oboe, Saxophone ...", nullptr));
        noteRangeChoice->setItemText(4, QCoreApplication::translate("SettingsDialog", "Notes F3 and higher - Flute, Piccolo ...", nullptr));

        commentTextLabel->setText(QCoreApplication::translate("SettingsDialog", "Allowing lower notes results in less timing precision.", nullptr));
        tempFilesGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Temporary Files", nullptr));
#if QT_CONFIG(tooltip)
        tempFilesFolderLabel->setToolTip(QCoreApplication::translate("SettingsDialog", "Temporary files used by the program are stored here. This should preferably be on a fast disk (i.e. local)", nullptr));
#endif // QT_CONFIG(tooltip)
        tempFilesFolderLabel->setText(QCoreApplication::translate("SettingsDialog", "&Temp Folder:", nullptr));
#if QT_CONFIG(tooltip)
        tempFilesFolder->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Temporary files used by the program are stored here.<br>This should preferably be on a fast disk (i.e. local)</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        tempFilesFolder->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        tempFilesFolder->setText(QString());
#if QT_CONFIG(tooltip)
        chooseTempFolderButton->setToolTip(QCoreApplication::translate("SettingsDialog", "Browse for a temporary folder", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        chooseTempFolderButton->setWhatsThis(QCoreApplication::translate("SettingsDialog", "Browse for a temporary folder", nullptr));
#endif // QT_CONFIG(whatsthis)
        chooseTempFolderButton->setText(QCoreApplication::translate("SettingsDialog", "...", nullptr));
        fileGeneratingStringLabel->setText(QCoreApplication::translate("SettingsDialog", "Filename &generating string:", nullptr));
        filenameGeneratingString->setText(QCoreApplication::translate("SettingsDialog", "sound", nullptr));
        fileGeneratingNumberLabel->setText(QCoreApplication::translate("SettingsDialog", "Minimum &Digits:", nullptr));
        filenameExample->setText(QCoreApplication::translate("SettingsDialog", "sound003.wav", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("SettingsDialog", "General", nullptr));
        generalGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "General", nullptr));
        textLabel1->setText(QCoreApplication::translate("SettingsDialog", "Sound &Input Source:", nullptr));
#if QT_CONFIG(tooltip)
        soundInput->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Select from this list to choose which device should be used to record sound.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        soundInput->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        textLabel2->setText(QCoreApplication::translate("SettingsDialog", "Sound &Output Source:", nullptr));
#if QT_CONFIG(tooltip)
        soundOutput->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Select from this list to choose which device should be used to play sound.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        soundOutput->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        numBuffersLabel->setText(QCoreApplication::translate("SettingsDialog", "Number of Buffers:", nullptr));
        recordingGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Recording", nullptr));
        channelsLabel->setText(QCoreApplication::translate("SettingsDialog", "&Channels:", nullptr));
        numberOfChannels->setItemText(0, QCoreApplication::translate("SettingsDialog", "Mono", nullptr));
        numberOfChannels->setItemText(1, QCoreApplication::translate("SettingsDialog", "Stereo", nullptr));

#if QT_CONFIG(tooltip)
        numberOfChannels->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Analysis is done for each individual channel, so Mono is the default.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        sampleRateLabel->setText(QCoreApplication::translate("SettingsDialog", "Sample &Rate:", nullptr));
        sampleRate->setItemText(0, QCoreApplication::translate("SettingsDialog", "8000", nullptr));
        sampleRate->setItemText(1, QCoreApplication::translate("SettingsDialog", "11025", nullptr));
        sampleRate->setItemText(2, QCoreApplication::translate("SettingsDialog", "22050", nullptr));
        sampleRate->setItemText(3, QCoreApplication::translate("SettingsDialog", "44100", nullptr));
        sampleRate->setItemText(4, QCoreApplication::translate("SettingsDialog", "48000", nullptr));
        sampleRate->setItemText(5, QCoreApplication::translate("SettingsDialog", "96000", nullptr));

#if QT_CONFIG(tooltip)
        sampleRate->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>44100 Hz is a good default rate that most soundcards will support. This is the sample rate used on CD's. The higher the rate the more processing time needed.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        bitsPerSampleLabel->setText(QCoreApplication::translate("SettingsDialog", "&Bits Per Sample:", nullptr));
        bitsPerSample->setItemText(0, QCoreApplication::translate("SettingsDialog", "8", nullptr));
        bitsPerSample->setItemText(1, QCoreApplication::translate("SettingsDialog", "16", nullptr));

#if QT_CONFIG(tooltip)
        bitsPerSample->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>A higher number of bits is preferable</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        muteOutput->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Usually muting is used when you have speakers to reduce feedback</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        muteOutput->setText(QCoreApplication::translate("SettingsDialog", "Mute output when recording", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("SettingsDialog", "Sound", nullptr));
        analysisGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Analysis", nullptr));
#if QT_CONFIG(tooltip)
        doingHarmonicAnalysis->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Unchecking this will reduce some processor time, but the widgets that use harmonic infomation will not work.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        doingHarmonicAnalysis->setText(QCoreApplication::translate("SettingsDialog", "Do harmonic analysis", nullptr));
#if QT_CONFIG(tooltip)
        doingFreqAnalysis->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Unchecking this will turn off all analysis. This still allows recording, and is useful on really slow machines, where the file can be opened again afterwards with analysis in non real-time.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        doingFreqAnalysis->setText(QCoreApplication::translate("SettingsDialog", "Do frequency analysis", nullptr));
#if QT_CONFIG(tooltip)
        doingAutoNoiseFloor->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Uses the lowest RMS value to estimate the noise-floor</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        doingAutoNoiseFloor->setText(QCoreApplication::translate("SettingsDialog", "Automaticly calculate noise-floor", nullptr));
        doingDetailedPitch->setText(QCoreApplication::translate("SettingsDialog", "Do detailed pitch anaysis", nullptr));
#if QT_CONFIG(tooltip)
        doingEqualLoudness->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Apply a 2nd order highpass butterworth filter to reduce low frequencies filtered by the middle-outer ear</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        doingEqualLoudness->setText(QCoreApplication::translate("SettingsDialog", "Use human-ear-filter for equal-loudness", nullptr));
        analysisTypeLabel->setText(QCoreApplication::translate("SettingsDialog", "Analysis type:", nullptr));
        analysisType->setItemText(0, QCoreApplication::translate("SettingsDialog", "MPM", nullptr));
        analysisType->setItemText(1, QCoreApplication::translate("SettingsDialog", "Autocorrelation", nullptr));
        analysisType->setItemText(2, QCoreApplication::translate("SettingsDialog", "MPM + Modified Cepstrum", nullptr));

#if QT_CONFIG(tooltip)
        analysisType->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>Choose algorithm type. McLeod Pitch Method (MPM) recomended.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        thresholdTextLabel->setText(QCoreApplication::translate("SettingsDialog", "Default threshold (%):", nullptr));
#if QT_CONFIG(tooltip)
        thresholdValue->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>The defaul threshold for a new channel. This threshold is used in MPM to help determine the correct octave for a note's pitch. This threshold can be changed on screen per channel.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        bufferSizeGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Buffer Size", nullptr));
        bufferSizeTextLabel->setText(QCoreApplication::translate("SettingsDialog", "Use &buffer size of", nullptr));
#if QT_CONFIG(tooltip)
        bufferSizeValue->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>This is the buffer size used in analysis. This determines the range of pitch that can be found</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        bufferSizeUnit->setItemText(0, QCoreApplication::translate("SettingsDialog", "milli-seconds", nullptr));
        bufferSizeUnit->setItemText(1, QCoreApplication::translate("SettingsDialog", "samples", nullptr));

#if QT_CONFIG(tooltip)
        bufferSizeRound->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>This enables efficient use of the Fast Fourier Transform (FFT). Prefered.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        bufferSizeRound->setText(QCoreApplication::translate("SettingsDialog", "Round sample size to nearest power of 2 (for speed)", nullptr));
        stepSizeGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Step Size", nullptr));
        stepSizeTextLabel->setText(QCoreApplication::translate("SettingsDialog", "Use &step size of", nullptr));
#if QT_CONFIG(tooltip)
        stepSizeValue->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>This is how often the analysis is performed. About a half or a quarter of the buffer size is prefered.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        stepSizeValue->setPrefix(QString());
        stepSizeUnit->setItemText(0, QCoreApplication::translate("SettingsDialog", "milli-seconds", nullptr));
        stepSizeUnit->setItemText(1, QCoreApplication::translate("SettingsDialog", "samples", nullptr));

#if QT_CONFIG(tooltip)
        stepSizeRound->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>This enables efficient buffer manipulation. Prefered.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        stepSizeRound->setText(QCoreApplication::translate("SettingsDialog", "Round to the nearest whole fraction of the buffer window", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("SettingsDialog", "Analysis", nullptr));
        colorsGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Colors", nullptr));
        backgroundButton->setText(QCoreApplication::translate("SettingsDialog", "&Background", nullptr));
#if QT_CONFIG(shortcut)
        backgroundButton->setShortcut(QCoreApplication::translate("SettingsDialog", "Alt+B", nullptr));
#endif // QT_CONFIG(shortcut)
        shading1Button->setText(QCoreApplication::translate("SettingsDialog", "Shading&1", nullptr));
#if QT_CONFIG(shortcut)
        shading1Button->setShortcut(QCoreApplication::translate("SettingsDialog", "Alt+1", nullptr));
#endif // QT_CONFIG(shortcut)
        shading2Button->setText(QCoreApplication::translate("SettingsDialog", "Shading&2", nullptr));
#if QT_CONFIG(shortcut)
        shading2Button->setShortcut(QCoreApplication::translate("SettingsDialog", "Alt+2", nullptr));
#endif // QT_CONFIG(shortcut)
        screenRefreshGroup->setTitle(QCoreApplication::translate("SettingsDialog", "Screen refresh", nullptr));
#if QT_CONFIG(tooltip)
        slowUpdateSpeed->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>This is how often most widgets are updated. e.g. the pitch contour. A smaller value requires more CPU time. 100 is reasonable for a 1GHz machine.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        fastUpdateSpeed->setToolTip(QCoreApplication::translate("SettingsDialog", "<qt>This is how often the smoothest widgets are updated. e.g. the tuner dial. A smaller value requires more CPU time. 50 is reasonable for a 1GHz machine.</qt>", nullptr));
#endif // QT_CONFIG(tooltip)
        fastUpdateLabel->setText(QCoreApplication::translate("SettingsDialog", "Do fast updates every", nullptr));
        milliSecondsLabel1->setText(QCoreApplication::translate("SettingsDialog", "milli-seconds", nullptr));
        slowUpdateLabel->setText(QCoreApplication::translate("SettingsDialog", "Do slow updates every", nullptr));
        milliSecondsLabel2->setText(QCoreApplication::translate("SettingsDialog", "milli-seconds", nullptr));
        windowOptionsGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "GroupBox", nullptr));
        useTopLevelWidgets->setText(QCoreApplication::translate("SettingsDialog", "Use top level widgets", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QCoreApplication::translate("SettingsDialog", "Display", nullptr));
        AdvancedGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Advanced Settings", nullptr));
        showMeanVarianceBars->setText(QCoreApplication::translate("SettingsDialog", "Show mean/variance Bars", nullptr));
        savingMode->setItemText(0, QCoreApplication::translate("SettingsDialog", "Ask when closing unsaved files (normal)", nullptr));
        savingMode->setItemText(1, QCoreApplication::translate("SettingsDialog", "Don't ask when closing unsaved files (use with care)", nullptr));
        savingMode->setItemText(2, QCoreApplication::translate("SettingsDialog", "Save all unsaved files with their default generated name (uses lots of space)", nullptr));

        mouseWheelZooms->setText(QCoreApplication::translate("SettingsDialog", "Mouse-wheel zooms. (Note: Ctrl inverts zoom/scroll)", nullptr));
        VibratoGroupBox->setTitle(QCoreApplication::translate("SettingsDialog", "Vibrato Settings", nullptr));
        vibratoSineStyle->setText(QCoreApplication::translate("SettingsDialog", "Use sine style for period view", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_5), QCoreApplication::translate("SettingsDialog", "Advanced", nullptr));
#if QT_CONFIG(tooltip)
        resetDefaultsButton->setToolTip(QCoreApplication::translate("SettingsDialog", "Resets all of the settings back to the install state", nullptr));
#endif // QT_CONFIG(tooltip)
        resetDefaultsButton->setText(QCoreApplication::translate("SettingsDialog", "&Reset Defaults", nullptr));
#if QT_CONFIG(shortcut)
        resetDefaultsButton->setShortcut(QCoreApplication::translate("SettingsDialog", "Alt+R", nullptr));
#endif // QT_CONFIG(shortcut)
        okButton->setText(QCoreApplication::translate("SettingsDialog", "OK", nullptr));
        cancelButton->setText(QCoreApplication::translate("SettingsDialog", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSDIALOG_H
