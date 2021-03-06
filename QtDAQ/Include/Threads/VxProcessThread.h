#pragma once

#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QVector>
#include <QTextStream>
#include <QFile>

#include "AcquisitionDefinitions.h"
#include "ProcessRoutines.h"
#include "AnalysisConfig.h"
#include "V8/V8Wrapper.h"

#define DT5730_ID 31
#define VX1761_ID 6
#define VX1742_ID 2

using namespace v8;

class VxProcessThread : public QThread
{
	 Q_OBJECT
public:
	VxProcessThread(QMutex* s_rawBuffer1Mutex, QMutex* s_rawBuffer2Mutex, EventVx* s_rawBuffer1, EventVx* s_rawBuffer2, int s_bufferLength = 1024, QObject *parent = 0);
	~VxProcessThread();
    bool initVxProcessThread(AnalysisConfig* s_analysisConfig, int updateTime=100);
	void restartProcessThread();
	void resetTriggerTimerAndV8();
	void loadTemperatureLog(QString filename);

 signals:
	 void newProcessedEvents(QVector<EventStatistics*>*);
	 void newEventSample(EventSampleData*);
	 void eventReadingFinished();

 private:

	 void processEvent(EventVx* rawEvent, bool outputSample);
	 bool processChannel(uint32_t boardId, EventVx* rawEvent, int ch, EventStatistics* stats, float GSPS, EventSampleData* sample, float  cfdOverrideTime=-1);


	//v8
	void compileV8();

	void run();
	bool runV8CodeInitial();
	bool runV8CodePreAnalysis();
	bool runV8CodePostChannelAnalysis(SampleStatistics* chStats);
	bool runV8CodePostEventAnalysis(EventStatistics* evStats);
	bool runV8CodeFinal();
	bool runV8CodeDefinitions();
	


	void printTruncatedSamples(float* sampleArray, float baseline, float startIndex, float endIndex);
	static void printSampleToStream(const FunctionCallbackInfo<Value>& args);
	static void printUncorrectedSampleToStream(const FunctionCallbackInfo<Value>& args);
	static void openFileStream(const FunctionCallbackInfo<Value>& args);
	static void closeStream(const FunctionCallbackInfo<Value>& args);
	static void openBinaryFileStream(const FunctionCallbackInfo<Value>& args);
	static void readFilterFile(const FunctionCallbackInfo<Value>& args);
	static void setDownsampleFactor(const FunctionCallbackInfo<Value>& args);

public slots:
	void onNewRawEvents(QVector<EventVx*>* events);
	void onUpdateTimerTimeout();
	void onAnalysisConfigChanged();
	void onResumeProcessing();
	
private:
	int eventIndex;
	int eventSize;
	int numSamples;
	float* tempValArray;
	float* tempFilteredValArray;
	float* tempMedianArray;
    QVector<EventStatistics*>* processedEvents;	
	QTimer updateTimer;
	bool sampleNextEvent;
	AnalysisConfig* analysisConfig;
	bool outputCurrentChannel;
	bool outputCurrentChannelUncorrected;
	QFile* file;
	QTextStream* textStream;
	QDataStream* dataStream;

	uint32_t previousRawTriggerTag;
	int wraparoundCounter;
	double startTimeMillis;
	int eventCounter;

	//v8
	Isolate* isolate;
	Persistent<ObjectTemplate> globalTemplate;
	Persistent<Object> sampleStatsObject;
	Persistent<Object> eventStatsObject;
	Persistent<Object> allChannelsStatsObject[NUM_DIGITIZER_CHANNELS_DAQ];
	Persistent<Context> persContext;
    Persistent<Script> scriptInitial, scriptFinished, scriptPre, scriptPostChannel, scriptPostEvent, scriptDef;

	//buffers
	//raw
	QMutex* rawMutexes[2];
	EventVx* rawBuffers[2];	
	//processed
	int currentBufferIndex;
	int currentBufferPosition;

	QMutex processedEventsMutex;
	QMutex v8Mutex;
	QMutex processingMutex;
	bool v8RecompileRequired = false;

	//temperature stored in array (per 5 seconds)
	QVector<float> temperatureArray;
	int bufferLength;
};
