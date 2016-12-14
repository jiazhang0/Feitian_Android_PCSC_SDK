#include <stdio.h>
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <assert.h>
#include "com_ftsafe_pcsclite_PCSC.h"

#include "pcsclite.h"
#include "winscard.h"
#include "reader.h"


#define LOG		"libWinScard"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , LOG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , LOG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  , LOG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG, __VA_ARGS__)

#define printLog(...)	if(printLogEnable) LOGD(__VA_ARGS__);
#define dprintf	printLog

#ifndef boolean
#define boolean int
#define true	1
#define false	0
#endif

char PCSCLITE_CSOCK_NAME[1024]="";
char PCSCLITE_RUN_PID[1024]="";

static boolean printLogEnable=false;


JNIEXPORT void JNICALL Java_com_ftsafe_pcsclite_PCSC_setPrintLogEnable
  (JNIEnv *env, jobject obj, jboolean b){
	printLogEnable = b;
}


JNIEXPORT void JNICALL Java_com_ftsafe_pcsclite_PCSC_setEnv
  (JNIEnv *env, jobject obj, jstring str){

	const char* pwd = (*env)->GetStringUTFChars(env, str, NULL);

	memset(PCSCLITE_CSOCK_NAME,0,sizeof(PCSCLITE_CSOCK_NAME));
	memset(PCSCLITE_RUN_PID,0,sizeof(PCSCLITE_RUN_PID));

	strcat(PCSCLITE_CSOCK_NAME,pwd);
	strcat(PCSCLITE_CSOCK_NAME,"/pcscd/pcscd.comm");

	strcat(PCSCLITE_RUN_PID,pwd);
	strcat(PCSCLITE_RUN_PID,"/pcscd/pcscd.pid");

	printLog("PCSCLITE_CSOCK_NAME===%s",PCSCLITE_CSOCK_NAME);
	printLog("PCSCLITE_RUN_PID======%s",PCSCLITE_RUN_PID);

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_STACK_BUFFER_SIZE 8192

// make the buffers larger than what should be necessary, just in case
#define ATR_BUFFER_SIZE 128
#define READERNAME_BUFFER_SIZE 128
#define RECEIVE_BUFFER_SIZE MAX_STACK_BUFFER_SIZE

#define J2PCSC_EXCEPTION_NAME "com/ftsafe/pcsclite/PCSCException"

void throwPCSCException(JNIEnv* env, LONG code) {
    jclass pcscClass;
    jmethodID constructor;
    jthrowable pcscException;

    pcscClass = (*env)->FindClass(env, J2PCSC_EXCEPTION_NAME);
    assert(pcscClass != NULL);
    constructor = (*env)->GetMethodID(env, pcscClass, "<init>", "(I)V");
    assert(constructor != NULL);
    pcscException = (jthrowable) (*env)->NewObject(env, pcscClass, constructor, (jint)code);
    (*env)->Throw(env, pcscException);
}
jboolean handleRV(JNIEnv* env, LONG code) {
    if (code == SCARD_S_SUCCESS) {
        return JNI_FALSE;
    } else {
        throwPCSCException(env, code);
        return JNI_TRUE;
    }
}
jobjectArray pcsc_multi2jstring(JNIEnv *env, char *spec) {
    jobjectArray result;
    jclass stringClass;
    char *cp, **tab;
    jstring js;
    int cnt = 0;

    cp = spec;
    while (*cp != 0) {
        cp += (strlen(cp) + 1);
        ++cnt;
    }

    tab = (char **)malloc(cnt * sizeof(char *));

    cnt = 0;
    cp = spec;
    while (*cp != 0) {
        tab[cnt++] = cp;
        cp += (strlen(cp) + 1);
    }

    stringClass = (*env)->FindClass(env, "java/lang/String");
    assert(stringClass != NULL);

    result = (*env)->NewObjectArray(env, cnt, stringClass, NULL);
    while (cnt-- > 0) {
        js = (*env)->NewStringUTF(env, tab[cnt]);
        (*env)->SetObjectArrayElement(env, result, cnt, js);
    }
    free(tab);
    return result;
}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT jlong JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardEstablishContext
  (JNIEnv *env, jclass thisClass, jint dwScope){
	SCARDCONTEXT context;
	    LONG rv;
	    dprintf("-establishContext\n");
	    rv = SCardEstablishContext(dwScope, NULL, NULL, &context);
	    if (handleRV(env, rv)) {
	        return 0;
	    }
	    // note: SCARDCONTEXT is typedef'd as long, so this works
	    return (jlong)context;
}

JNIEXPORT jobjectArray JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardListReaders
  (JNIEnv *env, jclass thisClass, jlong jContext){
    SCARDCONTEXT context = (SCARDCONTEXT)jContext;
    LONG rv;
    LPTSTR mszReaders;
    DWORD size;
    jobjectArray result;

    dprintf("-context: %lx\n", context);
    rv = SCardListReaders(context, NULL, NULL, &size);
    if (handleRV(env, rv)) {
        return NULL;
    }
    dprintf("-size: %ld\n", size);

    mszReaders = malloc(size);
    rv = SCardListReaders(context, NULL, mszReaders, &size);
    if (handleRV(env, rv)) {
        free(mszReaders);
        return NULL;
    }
    dprintf("-String: %s\n", mszReaders);

    result = pcsc_multi2jstring(env, mszReaders);
    free(mszReaders);
    return result;
}

JNIEXPORT jlong JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardConnect
(JNIEnv *env, jclass thisClass, jlong jContext, jstring jReaderName, jint jShareMode, jint jPreferredProtocols){
    SCARDCONTEXT context = (SCARDCONTEXT)jContext;
    LONG rv;
    LPCTSTR readerName;
    SCARDHANDLE card;
    DWORD proto;

    readerName = (*env)->GetStringUTFChars(env, jReaderName, NULL);
    rv = SCardConnect(context, readerName, jShareMode, jPreferredProtocols, &card, &proto);
    (*env)->ReleaseStringUTFChars(env, jReaderName, readerName);
    dprintf("-cardhandle: %lx\n", card);
    dprintf("-protocol: %ld\n", proto);
    if (handleRV(env, rv)) {
        return 0;
    }

    return (jlong)card;
}

JNIEXPORT jbyteArray JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardTransmit
(JNIEnv *env, jclass thisClass, jlong jCard, jint protocol, jbyteArray jBuf, jint jOfs, jint jLen){
    SCARDHANDLE card = (SCARDHANDLE)jCard;
    LONG rv;
    SCARD_IO_REQUEST sendPci;
    unsigned char *sbuf;
    unsigned char rbuf[RECEIVE_BUFFER_SIZE];
    DWORD rlen = RECEIVE_BUFFER_SIZE;
    int ofs = (int)jOfs;
    int len = (int)jLen;
    jbyteArray jOut;

    sendPci.dwProtocol = protocol;
    sendPci.cbPciLength = sizeof(SCARD_IO_REQUEST);

    sbuf = (unsigned char *) ((*env)->GetByteArrayElements(env, jBuf, NULL));
    rv = SCardTransmit(card, &sendPci, sbuf + ofs, len, NULL, rbuf, &rlen);
    (*env)->ReleaseByteArrayElements(env, jBuf, (jbyte *)sbuf, JNI_ABORT);

    if (handleRV(env, rv)) {
        return NULL;
    }

    jOut = (*env)->NewByteArray(env, rlen);
    (*env)->SetByteArrayRegion(env, jOut, 0, rlen, (jbyte *)rbuf);
    return jOut;
}

JNIEXPORT jbyteArray JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardControl
  (JNIEnv *env, jclass thisClass, jlong jCard, jlong jControlCode, jbyteArray jBuf, jint jLen){
	SCARDHANDLE card = (SCARDHANDLE)jCard;
	LONG rv;

	long controlCode = (int)jControlCode;

	unsigned char *sbuf;
	unsigned char rbuf[RECEIVE_BUFFER_SIZE];
	DWORD rlen = RECEIVE_BUFFER_SIZE;
	int len = (int)jLen;
	jbyteArray jOut;



	sbuf = (unsigned char *) ((*env)->GetByteArrayElements(env, jBuf, NULL));
	rv = SCardControl(card, controlCode, sbuf, len, rbuf, rlen, &rlen);
	(*env)->ReleaseByteArrayElements(env, jBuf, (jbyte *)sbuf, JNI_ABORT);

	if (handleRV(env, rv)) {
		return NULL;
	}

	jOut = (*env)->NewByteArray(env, rlen);
	(*env)->SetByteArrayRegion(env, jOut, 0, rlen, (jbyte *)rbuf);
	return jOut;

}

JNIEXPORT jbyteArray JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardStatus
(JNIEnv *env, jclass thisClass, jlong jCard, jbyteArray jStatus)
{
    SCARDHANDLE card = (SCARDHANDLE)jCard;
    LONG rv;
    char readerName[READERNAME_BUFFER_SIZE];
    DWORD readerLen = READERNAME_BUFFER_SIZE;
    unsigned char atr[ATR_BUFFER_SIZE];
    DWORD atrLen = ATR_BUFFER_SIZE;
    DWORD state;
    DWORD protocol;
    jbyteArray jArray;
    jbyte tmp;

    rv = SCardStatus(card, readerName, &readerLen, &state, &protocol, atr, &atrLen);
    if (handleRV(env, rv)) {
        return NULL;
    }
    dprintf("-reader: %s\n", readerName);
    dprintf("-status: %ld\n", state);
    dprintf("-protocol: %ld\n", protocol);

    jArray = (*env)->NewByteArray(env, atrLen);
    (*env)->SetByteArrayRegion(env, jArray, 0, atrLen, (jbyte *)atr);

    tmp = (jbyte)state;
    (*env)->SetByteArrayRegion(env, jStatus, 0, 1, &tmp);
    tmp = (jbyte)protocol;
    (*env)->SetByteArrayRegion(env, jStatus, 1, 1, &tmp);

    return jArray;
}

JNIEXPORT void JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardDisconnect
(JNIEnv *env, jclass thisClass, jlong jCard, jint jDisposition)
{
    SCARDHANDLE card = (SCARDHANDLE)jCard;
    LONG rv;

    rv = SCardDisconnect(card, jDisposition);
    dprintf("-disconnect: 0x%lX\n", rv);
    handleRV(env, rv);
    return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
jbyteArray makeByteArray(JNIEnv * env, unsigned char* byte, int size){
	jbyteArray jbarray = (*env)->NewByteArray(env,size);
	(*env)->SetByteArrayRegion(env,jbarray, 0, size, (jbyte*)byte);
	return jbarray;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
JNIEXPORT void JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardGetStatusChange
  (JNIEnv *env, jclass thisClass,
		  jlong jContext, jlong jTimeout,
		  jobjectArray jReaderNames, jintArray jCurrentStates, jintArray jEventStates, jobjectArray jRgbAtrs)
{
	SCARDCONTEXT context = (SCARDCONTEXT)jContext;
	LONG rv;

	int readers = (*env)->GetArrayLength(env, jReaderNames);
	dprintf("readers======%d\n",readers);

	SCARD_READERSTATE *readerState = malloc(readers * sizeof(SCARD_READERSTATE));

	int i;

	int *currentStates = (*env)->GetIntArrayElements(env, jCurrentStates, NULL);
	int *eventStates = (*env)->GetIntArrayElements(env, jEventStates, NULL);

	for(i=0;i<readers;i++){
		jobject jReaderName = (*env)->GetObjectArrayElement(env, jReaderNames, i);
		readerState[i].szReader = (*env)->GetStringUTFChars(env, jReaderName, NULL);
		dprintf("readerState[%d].szReader==========%s",i,readerState[i].szReader);


		readerState[i].pvUserData = NULL;


		readerState[i].dwCurrentState = currentStates[i];
		dprintf("readerState[%d].dwCurrentState======%ld",i,readerState[i].dwCurrentState);

		readerState[i].dwEventState = eventStates[i];
		dprintf("readerState[%d].dwEventState========%ld",i,readerState[i].dwEventState);

		readerState[i].cbAtr = 0;


		memset(readerState[i].rgbAtr,0,sizeof(readerState[i].rgbAtr));

	}

	(*env)->ReleaseIntArrayElements(env, jCurrentStates, currentStates, JNI_ABORT);
	(*env)->ReleaseIntArrayElements(env, jEventStates, eventStates, JNI_ABORT);


///////////////////////////////////////////////////////////////

	rv = SCardGetStatusChange(context, (DWORD)jTimeout, readerState, readers);

///////////////////////////////////////////////////////////////

	for(i=0;i<readers;i++){

		(*env)->SetIntArrayRegion(env, jCurrentStates, i, 1, (jint*)&(readerState[i].dwCurrentState));

		(*env)->SetIntArrayRegion(env, jEventStates, i, 1, (jint*)&(readerState[i].dwEventState));

		(*env)->SetObjectArrayElement(env, jRgbAtrs, i, makeByteArray(env,readerState[i].rgbAtr,readerState[i].cbAtr));
	 }


	 free(readerState);

	 handleRV(env, rv);
}
JNIEXPORT void JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardBeginTransaction
(JNIEnv *env, jclass thisClass, jlong jCard)
{
    SCARDHANDLE card = (SCARDHANDLE)jCard;
    LONG rv;

    rv = SCardBeginTransaction(card);
    dprintf("-beginTransaction: 0x%lX\n", rv);
    handleRV(env, rv);
    return;
}

JNIEXPORT void JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardEndTransaction
(JNIEnv *env, jclass thisClass, jlong jCard, jint jDisposition)
{
    SCARDHANDLE card = (SCARDHANDLE)jCard;
    LONG rv;

    rv = SCardEndTransaction(card, jDisposition);
    dprintf("-endTransaction: 0x%lX\n", rv);
    handleRV(env, rv);
    return;
}

JNIEXPORT void JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardIsValidContext
  (JNIEnv *env, jclass thisClass, jlong jContext){
	LONG rv;
	SCARDCONTEXT context = (SCARDCONTEXT)jContext;
	rv = SCardIsValidContext(context);
	handleRV(env, rv);
	return;
}

JNIEXPORT void JNICALL Java_com_ftsafe_pcsclite_PCSC_SCardReleaseContext
  (JNIEnv *env, jclass thisClass, jlong jContext){
	LONG rv;
	SCARDCONTEXT context = (SCARDCONTEXT)jContext;
	rv = SCardReleaseContext(context);
	handleRV(env, rv);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
